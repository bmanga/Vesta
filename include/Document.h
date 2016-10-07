#pragma once

#include "TextChunk.h"
#include <vector>
#include <experimental/filesystem>
#include "Request.h"
#include "Types.h"


namespace fs = std::experimental::filesystem;





class Document
{
	
public:
	Document(fs::path File);

	Document() : mActiveLine(1) {};

	bool handleRequest(FinalizedRequest Request);

	DocPosition insertChar(DocPosition Pos, char C);

	LineView lineAt(Line Ln);

	void open();
	//void close();
	//void save();

	char deleteChar(DocPosition Pos);

	void render()
	{
		for (auto &&Chunk : mChunks)
		{
			Chunk.render();
		}
	}

	DocPosition position(ScreenPosition SPos);

	LineView lastLine() const
	{
		return mChunks.back().lastLine();
	}
private:
	auto containingTextChunk(Line Ln)
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
