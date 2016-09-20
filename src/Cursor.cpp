#include <algorithm>
#include "Cursor.h"
#include "TextManager.h"
#include "TextureFont.h"
#include "VertexBuffer.h"
#include <Document.h>
using namespace ftgl;

void Cursor::moveTo(DocPosition Pos)
{
	mPos = Pos;

	LineView Line = mDocument->lineAt(Pos);
	// Adjust the character count
	Line.adjustPos(mPos);
	updateBuffer();
}

void Cursor::prev(unsigned N)
{
	//FIXME: this is utter ugliness
	auto Line = mPos.line().value();
	auto Column = mPos.column().value();
	auto Character = mPos.character().value();
	Column -= N;
	Character -= N;
	mPos = DocPosition(Line, Column, Character);

	//updateBuffer();
}

void Cursor::next(unsigned N) {
	//FIXME: this is utter ugliness
	auto Line = mPos.line().value();
	auto Column = mPos.column().value();
	auto Character = mPos.character().value();
	Column += N;
	Character += N;
	mPos = DocPosition(Line, Column, Character);

	//updateBuffer();
}

void Cursor::up(unsigned N)
{
	//FIXME: this is utter ugliness
	auto Line = mPos.line().value();
	auto Column = mPos.column().value();
	auto Character = mPos.character().value();
	Line -= N;
	mPos = DocPosition(Line, Column, Character);
}

void Cursor::down(unsigned N)
{
	//FIXME: this is utter ugliness
	auto Line = mPos.line().value();
	auto Column = mPos.column().value();
	auto Character = mPos.character().value();
	Line += N;
	mPos = DocPosition(Line, Column, Character);
}

void Cursor::eol()
{
	LineView Line = mDocument->lineAt(mPos);
	mPos = Line.endOfLinePos();
}

void Cursor::updateBuffer() const{
	Vertex Vertices[4];

	auto *Font = TextManager::Instance()->getFont(mFontInfo);
	auto *Glyph = Font->getGlyph("|");


	//Create a pen at the right position
	float X = mFontInfo.Width * (mPos.column().offset());
	float Y = mFontInfo.Height * (mPos.line().offset() + 1);
	Pen P{ {X - 5, -Y, 0.1f}, {0, 0, 1, 1} };
	SetGlyphVertices(Glyph, P, Vertices);

	unsigned Indices[6] = { 0, 1, 2, 0, 2, 3 };
	mTextBuffer->replaceVertices(0, (const char*)Vertices, 4);
	//mTextBuffer->erase(4);
	//mTextBuffer->insert(4, (const char*)Vertices, 4, Indices, 6);

	//use | to test



}
