#pragma once
#include <string>
#include <cassert>

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

	ThisType &operator++() {
		++mValue;
		return *this;
	}

private:
	unsigned mValue;
};

using Line = PosCoord<0>;
using Column = PosCoord<1>;
using Character = PosCoord<2>;

// Describes a location within a Document
class DocPosition
{
public:
	DocPosition(unsigned Ln, unsigned Cl, unsigned Ch) :
		mLine(Ln, Line::VALUE),
		mColumn(Cl, Column::VALUE),
		mCharacter(Ch, Character::VALUE) {
	}

	DocPosition(Line Ln, Column Cl, Character Ch) :
		mLine(Ln),
		mColumn(Cl),
		mCharacter(Ch) { }

	DocPosition() : DocPosition(1, 1, 1){
	}

	void set(Character C)
	{
		mCharacter = C;
	}
	Line line() const { return mLine; }
	Column column() const { return mColumn; }
	Character character() const { return mCharacter; }

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
private:
	Line mLine;
	Column mColumn;
	Character mCharacter;
};


struct Vertex
{
	float x, y, z;    // position
	float s, t;       // texture
	float r, g, b, a; // color
};

struct FontInfo
{
	std::string Name;
	float Pt;
	float Width;
	float Height;
};