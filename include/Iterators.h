#pragma once

/*
* This iterator template can be used with any type that provides a lineAt
* member function
*/
template <class Ty>
class line_iterator
{
public:
	explicit line_iterator(const Ty &TC, Line Ln)
		: mTC(TC)
		, mLine(Ln) {
	}

	LineView operator*() const {
		return mTC.lineAt(mLine);
	}

	line_iterator &operator++() {
		mLine = Line{ mLine.value() + 1 };
		return *this;
	}

	bool operator==(const line_iterator &Rhs) const {
		return mLine == Rhs.mLine;
	}

	bool operator!=(const line_iterator &Rhs) const {
		return !(*this == Rhs);
	}

private:
	const Ty &mTC;
	Line mLine;
};