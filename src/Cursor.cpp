#include <algorithm>
#include "Cursor.h"
#include "TextManager.h"
#include "TextureFont.h"
#include "VertexBuffer.h"
#include <Document.h>
#include <VestaOptions.h>
using namespace ftgl;

//This is the global buffer used to displsy the Cursor
// FIXME : there is room for optimizations here
static VertexBuffer gCursorBuffer("vertex:3f,tex_coord:2f,color:4f");

// FIXME: workaround to initialize gCursorBuffer. Looking forward to something
// nicer
namespace {
class ExecuteOnce
{
public:
	template <class Fn>
	ExecuteOnce(Fn F)
	{
		F();
	}
};

ExecuteOnce _([&]()
{
	Vertex Vertices[4] = {};
	constexpr static GLuint Indices[] = { 0, 1, 2, 0, 2, 3 };
	gCursorBuffer.push_back((const char*)Vertices, 4, Indices, 6);
	gCursorBuffer.push_back((const char*)Vertices, 4, Indices, 6);
});


void SetCursorVertices(const Glyph *glyph, Pen pen, Vertex *Vertices) {
	int x0 = int(pen.pos.x);
	int y0 = int(pen.pos.y + glyph->offset_y);
	int x1 = int(x0 + glyph->width);
	int y1 = int(y0 - glyph->height);
	float s0 = glyph->s0;
	float t0 = glyph->t0;
	float s1 = glyph->s1;
	float t1 = glyph->t1;

	float r = pen.col.r;
	float g = pen.col.g;
	float b = pen.col.b;
	float a = pen.col.a;


	Vertices[0] = { float(x0), float(y0), pen.pos.z,  s0, t0, r, g, b, a };
	Vertices[1] = { float(x0), float(y1), pen.pos.z,  s0, t1, r, g, b, a };
	Vertices[2] = { float(x1), float(y1), pen.pos.z,  s1, t1, r, g, b, a };
	Vertices[3] = { float(x1), float(y0), pen.pos.z,  s1, t0, r, g, b, a };
}

void SetHighlightVertices(const Glyph *Glyph, Pen Start, 
	Pen End, Vertex *Vertices)
{
	int x0 = int(Start.pos.x);
	int y0 = int(Start.pos.y + Glyph->offset_y);
	int x1 = int(End.pos.x + Glyph->width);
	int y1 = int(End.pos.y - Glyph->height / 3);
	float s0 = Glyph->s0;
	float t0 = Glyph->t0;
	float s1 = Glyph->s1;
	float t1 = Glyph->t1;

	float r = Start.col.r;
	float g = Start.col.g;
	float b = Start.col.b;
	float a = Start.col.a;


	Vertices[0] = { float(x0), float(y0), Start.pos.z,  s0, t0, r, g, b, a };
	Vertices[1] = { float(x0), float(y1), Start.pos.z,  s0, t1, r, g, b, a };
	Vertices[2] = { float(x1), float(y1), Start.pos.z,  s1, t1, r, g, b, a };
	Vertices[3] = { float(x1), float(y0), Start.pos.z,  s1, t0, r, g, b, a };
}

} // namespace

void Cursor::moveTo(DocPosition Pos)
{
	mPos = Pos;
	
	mIdealCol = Pos.column();
}

void Cursor::highlight(DocRange Selection)
{
	mSelection = Selection;
}

void Cursor::prev(unsigned N, bool AcrossLines)
{
	auto Ln = mPos.line().value();
	auto Cl = mPos.column().value();

	// Trying to go before the start of the line
	if (N >= Cl)
	{
		// Make sure we can/want to access the previous line
		if (AcrossLines && Ln > 1)
		{
			mPos = mDocument->lineAt(Line{ Ln - 1 }).endOfLine();
		}
		else
		{
			mPos = DocPosition(Ln, 1, 1);
		}

		return;
	}

	// We want to go to the previous location
	LineView LV = mDocument->lineAt(Line{ Ln });
	mPos = LV.position(ScreenPosition{ Ln, Cl - 1 });

	mIdealCol = mPos.column();
}

void Cursor::next(unsigned N, bool AcrossLines) {

	auto Ln = mPos.line().value();
	auto Cl = mPos.column().value();

	if (mPos == mDocument->lastLine().endOfLine()) {
		return;
	}

	LineView LV = mDocument->lineAt(mPos.line());

	if (AcrossLines && mPos == LV.endOfLine()) {
		mPos = DocPosition(Ln + 1, 1, 1);
		return;
	}
		
	mPos = LV.position(ScreenPosition{ Ln, Cl + 1 }, false);

	mIdealCol = mPos.column();
}

DocPosition Cursor::nextWord()
{
	LineView LV = mDocument->lineAt(mPos.line());

	DocRange Token = LV.tokenAt(mPos.character());

	moveTo(Token.end());

	return Token.end();
}

void Cursor::up(unsigned N)
{
	auto Ln = mPos.line().value();
	
	if (Ln == 1)
		return;

	LineView LV = mDocument->lineAt(Line{ --Ln });

	mPos = LV.position(ScreenPosition{ Ln, mIdealCol.value() });
}

void Cursor::down(unsigned N)
{
	auto Ln = mPos.line().value();

	if (Ln == mDocument->lastLine().line().value())
		return;

	LineView LV = mDocument->lineAt(Line{ ++Ln });

	mPos = LV.position(ScreenPosition{ Ln, mIdealCol.value() });
}

void Cursor::eol()
{
	LineView Line = mDocument->lineAt(mPos.line());
	mPos = Line.endOfLine();
}

void Cursor::updateBuffer()
{
	constexpr static GLuint Indices[] = { 0, 1, 2, 0, 2, 3 };
	gCursorBuffer.clear();
	// FIXME
	const VestaOptions &Opts = GetOptions();
	auto *Font = Opts.font().Font;
	auto *Glyph = Font->getGlyph("|");

	float FontWidth = Opts.font().Width;
	float FontHeight = Opts.font().Height;

	//Create a pen at the right position
	float X = FontWidth * (mPos.column().offset());
	float Y = FontHeight * (mPos.line().value());
	Pen P{ {X, -Y, 0.5f}, {1, 1, 1, 1} };

	Vertex Vertices[4];

	SetCursorVertices(Glyph, P, Vertices);

	//gCursorBuffer.replaceVertices(0, (const char*)Vertices, 4);
	gCursorBuffer.push_back((const char*)Vertices, 4, Indices, 6);


	// Deal with the cursor selection
	if (!mSelection.isValid()) {
		return;
	}



	Vertex SelVertices[4];
	
	//gCursorBuffer.replaceVertices(1, (const char *)SelVertices, 4);
	if (mSelection.containedLines() == 1) {
		DocPosition Start = mSelection.start();
		DocPosition End = mSelection.end();

		float SelX0 = FontWidth * (Start.column().offset());
		float SelY0 = FontHeight * (Start.line().value());
		float SelX1 = FontWidth * (End.column().offset());
		float SelY1 = FontHeight * (End.line().value());
		Pen SelStart{ { SelX0, -SelY0, 0.5f },{ 0, 0.3, 1, 0.6 } };
		Pen SelEnd{ { SelX1, -SelY1, 0.5f },{ 1, 1, 1, 0.2 } };
		SetHighlightVertices(Glyph, SelStart, SelEnd, SelVertices);
		gCursorBuffer.push_back((const char*)SelVertices, 4, Indices, 6);
	}
	else {
		mSelection.normalize();
		DocPosition Start = mSelection.start();
		// We deal with first and last line separately.
		Line L = Start.line();
		DocPosition End = mDocument->lineAt(L).endOfLine();

		float SelX0 = FontWidth * (Start.column().offset());
		float SelY0 = FontHeight * (Start.line().value());
		float SelX1 = FontWidth * (End.column().offset());
		float SelY1 = FontHeight * (End.line().value());
		Pen SelStart{ { SelX0, -SelY0, 0.5f },{ 0, 0.3, 1, 0.6 } };
		Pen SelEnd{ { SelX1, -SelY1, 0.5f },{ 1, 1, 1, 0.2 } };
		SetHighlightVertices(Glyph, SelStart, SelEnd, SelVertices);
		gCursorBuffer.push_back((const char*)SelVertices, 4, Indices, 6);

		// Deal with the middle lines
		unsigned LineCnt = mSelection.containedLines() - 2;
		while (++L != mSelection.end().line()) {
			End = mDocument->lineAt(L).endOfLine();
			SelX0 = 0;
			SelY0 = FontHeight * L.value();
			SelX1 = FontWidth * (End.column().offset());
			SelY1 = FontHeight * (End.line().value());
			SelStart = { { SelX0, -SelY0, 0.5f },{ 0, 0.3f, 1, 0.6f } };
			SelEnd = { { SelX1, -SelY1, 0.5f },{ 1, 1, 1, 0.2f } };
			SetHighlightVertices(Glyph, SelStart, SelEnd, SelVertices);
			gCursorBuffer.push_back((const char*)SelVertices, 4, Indices, 6);
		}

		// Deal with the last line
		End = mSelection.end();
		SelX0 = 0;
		SelY0 = FontHeight * L.value();
		SelX1 = FontWidth * (End.column().offset());
		SelY1 = FontHeight * (End.line().value());
		SelStart = { { SelX0, -SelY0, 0.5f },{ 0, 0.3f, 1, 0.6f } };
		SelEnd = { { SelX1, -SelY1, 0.5f },{ 1, 1, 1, 0.2f } };
		SetHighlightVertices(Glyph, SelStart, SelEnd, SelVertices);
		gCursorBuffer.push_back((const char*)SelVertices, 4, Indices, 6);




	}

}

void Cursor::render()
{
	TextManager::Instance()->renderText(&gCursorBuffer);
}
