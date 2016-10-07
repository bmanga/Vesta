#include "Types.h"
#include "VestaOptions.h"

bool DocPosRange::contains(Line Line) const {
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
	VestaOptions &Opts = GetOptions();

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

DocPosition LineView::position(ScreenPosition SP, bool BeforeTab) const
{
	if (SP.column() > endOfLine().column())
		return endOfLine();

	VestaOptions &Opts = GetOptions();
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

