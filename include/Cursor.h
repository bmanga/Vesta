#pragma once
#include "Types.h"


class Document;

namespace ftgl {
class VertexBuffer;
}

using namespace ftgl;



class Cursor
{
public:
	Cursor(Document *Document) 
		: mDocument(Document)
		, mPos(1, 1, 1) {

	}

	void moveTo(DocPosition Pos);


	void prev(unsigned N = 1, bool AcrossLines = true);
	void next(unsigned N = 1, bool AcrossLines = true);
	void up(unsigned N = 1);
	void down(unsigned N = 1);
	void eol();
	DocPosition getPosition() const { return mPos; }

	Document *getUnderlyingDocument() const { return nullptr; }

	void updateBuffer() const;

	static void render();

private:
	Document *mDocument;
	DocPosition mPos;
	Column mIdealCol{1}; // Used to remember column while moving across lines
};

