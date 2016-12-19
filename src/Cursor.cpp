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
}

void Cursor::moveTo(DocPosition Pos)
{
	mPos = Pos;
	
	mIdealCol = Pos.column();
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

void Cursor::updateBuffer() const{
	

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

	// TODO: Scale the cursor vertices so that it looks e bit bigger 
	// (and thinner) than the normal '|'
	SetCursorVertices(Glyph, P, Vertices);

	gCursorBuffer.replaceVertices(0, (const char*)Vertices, 4);
}

void Cursor::render()
{
	TextManager::Instance()->renderText(&gCursorBuffer);
}
