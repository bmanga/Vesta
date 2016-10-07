#pragma once
#include <string>

#include "Types.h"
#include "Freetype-core/VertexBuffer.h"
#include <vector>
#include "TextManager.h"


// Used to pass non-const references to string around, while avoiding implicit
// string copies. Uses pointer syntax to access the string :(
class StringRef
{
public:
	StringRef(std::string &Str)
		: mStr(Str) {
	}

	std::string *operator->() const {
		return &mStr;
	}

	char operator[] (size_t N) const
	{
		return mStr[N];
	}

	explicit operator const std::string &() const {
		return mStr;
	}

private:
	std::string &mStr;
};


/* TextChunk is the class responsible for a chunk of text.
 *
 */

class TextChunk
{
public:
	TextChunk (LineView First) 
		: mDocSpan(First.lineRange())
		, mContentBuffer(First.str())
		, mGlyphBuffer("vertex:3f,tex_coord:2f,color:4f")
		, mGlyphBufferDirty(true)
		, mPendingBufUpdate(false)
	{
		mNewlines.push_back(0);
		mNewlines.push_back(static_cast<uint16_t>(First.length()));

	}
	void append(LineView Line)
	{
		assert(mNewlines.size() < 255);
		assert(Line.startOfLine() > mDocSpan.end()
			&& "Line must come after what the current buffer spans over");

		mContentBuffer.append(Line.start(), Line.length());
		mNewlines.push_back(static_cast<uint16_t>
			(mNewlines.back() + Line.length()));

		mDocSpan.setEnd(Line.endOfLine());
	}

	void replace(LineView Line);

	void insert(LineView Line);
	void erase(Line Line);

	LineView lineAt(Line Line) const
	{
		auto Idx = newlineVecOffset(Line);

		auto OffStart = mNewlines[Idx];
		auto OffEnd = mNewlines[Idx + 1];

		const char *Start = &mContentBuffer[OffStart];
		unsigned Length = OffEnd - OffStart;

		return { Start, Length, Line };
	}

	LineView lastLine() const
	{
		return lineAt(mDocSpan.end().line());
	}

	const std::string &text() const {
		return mContentBuffer;
	}

	StringRef activeLineBuffer(Line Ln, bool ForceUpdate = false);

	void render();

	char deleteChar(DocPosition Pos);
	DocPosition insertChar(DocPosition Pos, char C);

	bool contains(Line Ln) const
	{
		return mDocSpan.contains(Ln);
	}


	void insertNewline(DocPosition Pos);

private:
	void flushBuffer();

	void deleteNewlineAfter(Line Ln);
	void generateGlyphBuffer();

	size_t newlineVecOffset(Line Line) const
	{
		assert(mDocSpan.contains(Line));

		auto StartLn = mDocSpan.start().line().value();
		auto LineLn = Line.value();

		return LineLn - StartLn;
	}

	DocPosRange mDocSpan;
	std::string mContentBuffer;
	//We store the index of the first line characters here.
	//We do not store the newline characters inside of the string
	std::vector<uint16_t> mNewlines;
	ftgl::VertexBuffer mGlyphBuffer;

	bool mGlyphBufferDirty : 1;
	bool mPendingBufUpdate : 1; // I dont think we need this
};


