#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <algorithm>


struct Position
{
	float x, y;
	float z;
};

struct Color
{
	float r, g, b, a;
};

class Pen
{
public:
	Position pos;
	Color    col;
};

// Make it template so the various instances are different types
template <unsigned N>
class PosCoord
{
	using ThisType = PosCoord<N>;
public:
	enum Type: unsigned
	{
		VALUE = 0,
		OFFSET = 1
	};
	explicit PosCoord(unsigned Value, Type T = VALUE)
	{
		assert(Value + T > 0);
		mValue = Value + T;
	}

	unsigned value() const { return mValue;  }
	unsigned offset() const { return mValue - 1; }

	bool operator < (const ThisType &Rhs) const {
		return mValue < Rhs.mValue;
	}
	bool operator > (const ThisType &Rhs) const {
		return mValue > Rhs.mValue;
	}

	bool operator == (const ThisType &Rhs) const{
		return mValue == Rhs.mValue;
	}

	bool operator != (const ThisType &Rhs) const {
		return mValue != Rhs.mValue;
	}

	bool operator <= (const ThisType &Rhs) const {
		return !(*this > Rhs);
	}

	bool operator >= (const ThisType &Rhs) const {
		return !(*this < Rhs);
	}

	ThisType &operator++() {
		++mValue;
		return *this;
	}

	ThisType operator++(int) {
		return ThisType { mValue++ };
	}

private:
	unsigned mValue;
};

using Line = PosCoord<0>;
using Column = PosCoord<1>;
using Character = PosCoord<2>;

class ScreenPosition
{
public:
	ScreenPosition(unsigned Ln, unsigned Cl)
		: mLine(Ln)
		, mColumn(Cl) { }

	ScreenPosition(Line Ln, Column Cl)
		: mLine(Ln)
		, mColumn(Cl) { }

	auto line() const { return mLine; }
	auto column() const { return mColumn; }
protected:
	Line mLine;
	Column mColumn;
};
// Describes a location within a Document. Same as ScreenPosition but also has
// information about the character nummber
class DocPosition : public ScreenPosition
{
public:
	DocPosition(unsigned Ln, unsigned Cl, unsigned Ch) :
		ScreenPosition(Ln, Cl),
		mCharacter(Ch, Character::VALUE) {
	}

	DocPosition(Line Ln, Column Cl, Character Ch) :
		ScreenPosition(Ln, Cl),
		mCharacter(Ch) { }

	DocPosition(ScreenPosition SP, unsigned Ch)
		: ScreenPosition(SP)
		, mCharacter(Ch) { }

	DocPosition() : DocPosition(1, 1, 1){
	}

	auto character() const { return mCharacter; }

	bool operator > (const DocPosition &Rhs) const{
		if (mLine != Rhs.mLine) {
			return mLine > Rhs.mLine;
		}

		return mColumn > Rhs.mColumn;
	}

	bool operator < (const DocPosition &Rhs) const {
		if (mLine != Rhs.mLine) {
			return mLine < Rhs.mLine;
		}

		return mColumn < Rhs.mColumn;
	}

	bool operator <= (const DocPosition &Rhs) const {
		return !(*this > Rhs);
	}

	bool operator >= (const DocPosition &Rhs) const {
		return !(*this < Rhs);
	}

	bool operator == (const DocPosition &Rhs) const {
		return mLine == Rhs.mLine
			&& mColumn == Rhs.mColumn
			&& mCharacter == Rhs.mCharacter;
	}

	bool operator != (const DocPosition &Rhs) const {
		return !(*this == Rhs);
	}
private:
	Character mCharacter;
};


class DocRange
{
public:
	DocRange() = default;
	DocRange(DocPosition Start, DocPosition End)
		: mStart(Start)
		, mEnd(End)
	{ }
	DocPosition start() const {
		return mStart;
	}

	DocPosition end() const {
		return mEnd;
	}

	void setStart(DocPosition Pos)
	{
		mStart = Pos;
	}

	void setEnd(DocPosition Pos)
	{
		mEnd = Pos;
	}

	unsigned containedLines() const
	{
		return mEnd.line().value() - mStart.line().value();
	}

	bool contains(Line Line) const;
	//bool contains(Column Cl) const;
private:
	DocPosition mStart;
	DocPosition mEnd;
};



class LineView
{
public:
	static LineView Invalid()
	{
		return LineView{ nullptr, 0, Line{1} };
	}
	LineView(const std::string &Start, Line Line);
	LineView(const char *Start, size_t Len, Line Line);

	unsigned count(const char C, Character Off = Character(1)) const;

	bool isTab(Character Char) const {
		return mStart[Char.offset()] == '\t';
	}

	//size_t countIfNot(const char C, Offset Off = 0) const;

	DocPosition endOfLine() const;
	DocPosition startOfLine() const;
	DocRange lineRange() const {
		return{ startOfLine(), endOfLine() };
	}

	DocRange tokenAt(Character DP);

	DocPosition position(ScreenPosition SP, bool BeforeTab = true) const;

	size_t length() const {
		assert(isValid());
		return mLength;
	}

	const char *start() const {
		assert(isValid());
		return mStart;
	}

	auto line() const {
		assert(isValid());
		return mLine;
	}

	std::string str() const
	{
		assert(isValid());
		return std::string{ mStart, mLength };
	}

	bool isValid() const {
		return mStart != nullptr;
	}
private:
	const char *mStart;
	unsigned    mLength;
	Line        mLine;
};
