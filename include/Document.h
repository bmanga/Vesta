#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <experimental/filesystem>
#include "Request.h"
#include "Types.h"
#include "VestaOptions.h"
#include <cassert>

namespace fs = std::experimental::filesystem;



class LineView
{
public:

	LineView(const char* Start, size_t Len, Line Line);

	unsigned count(const char C, Character Off = Character(1)) const;

	bool isTab(Character Char) const
	{
		return mStart[Char.offset()] == '\t';
	}

	//size_t countIfNot(const char C, Offset Off = 0) const;

	DocPosition endOfLinePos() const;
	void adjustPos(DocPosition& Pos) const;


private:
	const char *mStart;
	size_t      mLength;
	Line        mLine;
};

class Document
{
	
public:
	Document(fs::path File);

	Document() = default;

	bool handleRequest(FinalizedRequest Request);

	void insertChar(DocPosition Pos, char C);

	LineView lineAt(DocPosition Pos)
	{
		assert(Pos.line().value() < mNewlines.size());
		const char *pText = &mText[0];
		size_t OffStart = mNewlines[Pos.line().offset()];
		size_t OffEnd = mNewlines[Pos.line().offset() + 1];
		return{ pText + OffStart, OffEnd - OffStart - 1, Pos.line() };
	}

	void open();
	//void close();
	//void save();

	char deleteChar(DocPosition Pos);

	const std::string& text() const;
private:
	enum class NewlineType: unsigned char
	{
		LF,  // \n -> Unix
		CR,  // \r -> Mac
		CRLF // \r\n -> Windows
	};
	NewlineType mNewlineType;
	fs::path mFilePath;
	std::vector<uint32_t> mNewlines;
	std::string mText;
};
