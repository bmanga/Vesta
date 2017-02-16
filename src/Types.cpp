#include "Types.h"
#include "VestaOptions.h"
#include <string>


bool DocRange::contains(Line Line) const {
	return mStart.line() <= Line &&
		mEnd.line() >= Line;
}

LineView::LineView(const std::string& Start, Line Line)
	: mStart(Start.data())
	, mLength(Start.length())
	, mLine(Line)
{
}

LineView::LineView(const char* Start, size_t Len, Line Line)
	: mStart(Start)
	, mLength(Len)
	, mLine(Line) {
}

unsigned LineView::count(const char C, Character Off) const {
	return std::count(mStart + Off.offset(), mStart + mLength, C);
}

DocPosition LineView::endOfLine() const {
	unsigned NumTabs = count('\t');
	// FIXME : this needs to change as soon as we have a global settings 
	// instance
	const VestaOptions &Opts = GetOptions();

	unsigned TabSize = Opts.textEditor().TabSize;

	unsigned Col = (TabSize - 1) * NumTabs + mLength;

	return DocPosition(
		mLine,
		Column(Col, Column::OFFSET),
		Character(mLength, Character::OFFSET));
}

DocPosition LineView::startOfLine() const {
	return DocPosition(mLine, Column(1), Character(1));
}

DocRange LineView::tokenAt(Character DP) {
	unsigned OffsetB = DP.offset();
	unsigned OffsetF = OffsetB;

	const char *TokenSeam = " :!<.>()[]{}=";
	while (OffsetB != 0) {
		--OffsetB;
		if (std::any_of(TokenSeam, TokenSeam + 13, [this, &OffsetB](char C) {
			return mStart[OffsetB] == C; }
		)) {
			break;
		}

	}
	
	// FIXME avoid this ugliness
	// If the current one is a TokenSeam, just increase by 1 and return
	if (std::any_of(TokenSeam, TokenSeam + 11, [this, &OffsetF](char C) {
		return mStart[OffsetF] == C; }
	)) {
		++OffsetF;
		goto END;
	}

	while (OffsetF < endOfLine().character().offset()) {
		
		if (std::any_of(TokenSeam, TokenSeam + 11, [this, &OffsetF](char C) {
			return mStart[OffsetF] == C; }
		)) {
			break;
		}
		++OffsetF;
	}

END:	// FIXME this ignores tabs
	DocPosition TokenStart{ mLine, Column(OffsetB, Column::OFFSET), Character(OffsetB, Character::OFFSET) };
	DocPosition TokenEnd{ mLine, Column(OffsetF, Column::OFFSET), Character(OffsetF, Character::OFFSET) };
	return{ TokenStart, TokenEnd };
}

DocPosition LineView::position(ScreenPosition SP, bool BeforeTab) const
{
	if (SP.column() > endOfLine().column())
		return endOfLine();

	const VestaOptions &Opts = GetOptions();
	unsigned TabSize = Opts.textEditor().TabSize;

	unsigned ClPos = 1;
	unsigned ChPos = 1;
	unsigned PrevCl = 1;
	unsigned PrevCh = 1;
	while (ClPos < SP.column().value())
	{
		PrevCl = ClPos;
		PrevCh = ChPos;

		if (isTab(Character(ChPos)))
		{
			ClPos += TabSize - 1;
		}
		++ChPos;
		++ClPos;
	}

	if (ClPos > SP.column().value() && BeforeTab)
	{
		ClPos = PrevCl;
		ChPos = PrevCh;
	}

	return{ SP.line().value(), ClPos, ChPos };
}

