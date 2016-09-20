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
	Cursor(Document *Document, VertexBuffer *TextBuffer, FontInfo Info) 
		: mDocument(Document)
		, mTextBuffer(TextBuffer)
		, mFontInfo(Info)
		, mPos(1, 1, 1) {

	}

	void moveTo(DocPosition Pos);

	void setFontInfo(FontInfo Info)
	{
		mFontInfo = Info;
	}

	void prev(unsigned N = 1);
	void next(unsigned N = 1);
	void up(unsigned N = 1);
	void down(unsigned N = 1);
	void eol();
	DocPosition getPosition() const { return mPos; }

	Document *getUnderlyingDocument() const { return nullptr; }

	void updateBuffer() const;

private:
	Document *mDocument;
	VertexBuffer *mTextBuffer;
	FontInfo mFontInfo;
	DocPosition mPos;
};

