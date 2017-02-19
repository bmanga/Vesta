#pragma once

#include "TextChunk.h"
#include <vector>
#include <experimental/filesystem>
#include "Request.h"
#include "Types.h"


namespace fs = std::experimental::filesystem;

class Document;

class LineViewRange
{
public:
	using iterator = line_iterator<Document>;
	LineViewRange(const Document &Doc, Line First, Line Last) 
		: mDoc(Doc)
		, mFirst(First)
		, mLast(Last) {}


	iterator begin() {
		return iterator{ mDoc, mFirst };
	}

	iterator end() {
		return iterator{ mDoc, Line{mLast.value() + 1} };
	}
private:
	const Document &mDoc;
	Line mFirst;
	Line mLast;

};


class Document
{
	
public:
	using iterator = line_iterator<Document>;

	Document(fs::path File);

	Document() : mActiveLine(1) {};

	// We deal with Requests here because we want to have a common pool 
	// of previous Requests, so we can deal with undo/redo independently of
	// the editor window we are in.
	bool handleRequest(FinalizedRequest Request);

	DocPosition insertChar(DocPosition Pos, char C);
	std::pair<std::string, DocPosition> replaceRange(DocRange Rng, const std::string &Str);
	LineView lineAt(Line Ln) const;

	void open();
	//void close();
	void save();

	char deleteChar(DocPosition Pos);
	std::string deleteRange(DocRange Rng);


	LineViewRange range(Line First, Line Last) {
		if (mChunks.size() == 0) {
			Last = Line{ 1 };
		}
		else {
			Last = std::min(Last, lastLine().line());
		}
		return{ *this, First, Last };
	}

	DocPosition position(ScreenPosition SPos);

	LineView lastLine() const
	{
		return mChunks.back().lastLine();
	}

	iterator begin()
	{
		return iterator(*this, Line{ 1 });
	}

	iterator end()
	{
		if (empty())
			return iterator{ *this, Line{1} };

		return iterator(*this, Line{ lastLine().line().value() + 1 });
	}

	bool empty() const
	{
		return mChunks.empty();
	}

private:
	const auto containingTextChunk(Line Ln) const
	{
		auto It = std::find_if(mChunks.begin(), mChunks.end(), 
			[Ln](const TextChunk &TC)
		{
			return TC.contains(Ln);
		});

		// This should never fail
		//assert(It != mChunks.end());

		return It;
	}
	auto containingTextChunk(Line Ln) {
		auto It = std::find_if(mChunks.begin(), mChunks.end(),
			[Ln](const TextChunk &TC)
		{
			return TC.contains(Ln);
		});

		// This should never fail
		//assert(It != mChunks.end());

		return It;
	}
	enum class NewlineType: unsigned char
	{
		LF,  // \n -> Unix
		CR,  // \r -> Mac
		CRLF // \r\n -> Windows
	};
	NewlineType mNewlineType;
	fs::path mFilePath;
	std::vector<TextChunk> mChunks;
	std::string mActiveTextBuffer;
	Line mActiveLine;
};
