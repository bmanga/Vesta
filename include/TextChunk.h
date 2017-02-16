#pragma once

#include "Types.h"
#include "Freetype-core/VertexBuffer.h"
#include <vector>
#include "Iterators.h"

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

// TODO: move to its own file.

/* TextChunk is the class responsible for a chunk of text.
 *
 * Edits are applied either by
 * -> replacing a line through the LineView class
 * -> modifying the activeLineBuffer string
 * -> using the deleteChar/insertChar functions.
 */

class TextChunk
{
public:
	using iterator = line_iterator<TextChunk>;
	using const_iterator = line_iterator<TextChunk>;

	explicit TextChunk(LineView First);

	void append(LineView Line);
	void replace(LineView Line);
	void insert(LineView Line);
	void erase(Line Line);

	LineView lineAt(Line Line) const;
	LineView lastLine() const;


	StringRef activeLineBuffer(Line Ln, bool ForceUpdate = false);
	
	char deleteChar(DocPosition Pos);
	DocPosition insertChar(DocPosition Pos, char C);

	bool contains(Line Ln) const;

	void insertNewline(DocPosition Pos);

	friend std::ostream& operator<<(std::ostream& Out, const TextChunk& Chunk);


	iterator begin() const;
	iterator end() const;


private:
	void flushBuffer();
	void deleteNewlineAfter(Line Ln);

	size_t newlineVecOffset(Line Line) const;

	DocRange mDocSpan;
	std::string mContentBuffer;
	//We store the index of the first line characters here.
	//We do not store the newline characters inside of the string
	std::vector<uint16_t> mNewlines;
	

	
	bool mPendingBufUpdate : 1; // I dont think we need this
};



