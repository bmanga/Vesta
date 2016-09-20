#include <fstream>
#include "Document.h"
#include "EditActions.h"

LineView::LineView(const char* Start, size_t Len, Line Line) 
	: mStart(Start)
	, mLength(Len)
	, mLine(Line){
}

unsigned LineView::count(const char C, Character Off) const {
	return std::count(mStart + Off.offset(), mStart + mLength, C);
}

DocPosition LineView::endOfLinePos() const
{
	unsigned NumTabs = count('\t');
	// FIXME : this needs to change as soon as we have a global settings 
	// instance
	VestaOptions Opts;

	unsigned TabSize = Opts.textEditor().TabSize;

	unsigned Col = (TabSize - 1) * NumTabs + mLength;

	return DocPosition(
		mLine, 
		Column(Col, Column::OFFSET),
		Character(mLength, Character::OFFSET));
}

void LineView::adjustPos(DocPosition& Pos) const
{
	if (Pos > endOfLinePos())
		Pos = endOfLinePos();

	// FIXME: this needs to access global options
	VestaOptions Opts;
	unsigned TabSize = Opts.textEditor().TabSize;
	unsigned NumChars = Pos.column().value();

	for (Character Char{1}; Char.offset() < Pos.column().offset(); ++Char)
	{
		if (isTab(Char))
			NumChars -= TabSize - 1;
	}

	Pos.set(Character(NumChars, Character::VALUE));
}

Document::Document(fs::path File): mFilePath(File)
{
	open();
}

void Document::open()
{
	mNewlines.clear();
	mNewlines.push_back(0);

	std::ifstream Doc(mFilePath);

	std::string Line;
	uint32_t NewLineOffset = 0;

	while(std::getline(Doc, Line))
	{
		mText += Line.append("\n");
		NewLineOffset += Line.size();
		mNewlines.push_back(NewLineOffset);
		
	}
}

char Document::deleteChar(DocPosition Pos)
{
	unsigned Offset = mNewlines[Pos.line().offset()];
	Offset += Pos.character().offset() - 1;

	char Deleted = mText[Offset];
	mText.erase(Offset, 1);


	// Update Newline Indices
	for (auto It = mNewlines.begin() + Pos.line().value();
		It != mNewlines.end();
		++It) {
		--(*It);
	}

	if (Deleted == '\n') {
		mNewlines.erase(mNewlines.begin() + Pos.line().offset());
	}

	return Deleted;
}

const std::string& Document::text() const
{
	return mText;
}

bool Document::handleRequest(FinalizedRequest Request)
{
	auto Action = Request.getAction();
	auto *Cursor = Request.getCursor();
	Action->commit(this, Cursor);
	return true;
}

void Document::insertChar(DocPosition Pos, char C)
{
	unsigned Offset = mNewlines[Pos.line().offset()];
	Offset += Pos.character().offset();

	mText.insert(Offset, 1, C);

		// Update Newline Indices
	for (auto It = mNewlines.begin() + Pos.line().value(); 
		It != mNewlines.end(); 
		++It)
	{
		++(*It);
	}

	if (C == '\n')
		mNewlines.insert(mNewlines.begin() + Pos.line().value(), Offset + 1);
}
