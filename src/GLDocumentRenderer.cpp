#include "GLDocumentRenderer.h"
#include <VestaOptions.h>
#include "Document.h"


static void AddLineToVertexBuffer(LineView Line, VertexBuffer &Buffer, 
	std::vector<TinyColor> Cols) {
	const VestaOptions &Opts = GetOptions();

	Position Pos{ 0, 0, 0 };

	//FIXME: hate the negative sign
	Pos.y -= Line.line().value() * Opts.font().Height;

	Font *F = Opts.font().Font;

	const char *Codepoints = Line.start();
	size_t Len = Line.length();

	for (size_t J = 0;
		J < Len;
		J += utf8_surrogate_len(Codepoints + J)) {
		if (Codepoints[J] == '\t') {
			unsigned TabSize = Opts.textEditor().TabSize;
			Pos.x += Opts.font().Width * TabSize;
			continue;
		}
		const Glyph *G = F->getGlyph(Codepoints + J);

		if (!G) {
			continue;
		}

		Vertex Vertices[4];
		Color Col;
		TinyColor Tiny = Cols[J];
		Col.a = Tiny.a / 15.f;
		Col.r = Tiny.r / 15.f;
		Col.g = Tiny.g / 15.f;
		Col.b = Tiny.b / 15.f;
		SetGlyphVertices(G, Pos, Col, Vertices);

		constexpr static GLuint Indices[] = { 0, 1, 2, 0, 2, 3 };
		Buffer.push_back((const char*)Vertices, 4, Indices, 6);
		Pos.x += Opts.font().Width;
	}
}

static std::vector<TinyColor> AddLineToColorBuffer(LineView LV)
{
	std::vector<TinyColor> Colored;

	for (size_t J = 0; J < LV.length(); ++J)
	{
		//if (LV.start()[J] == 'a')
		//	Colored.push_back({ 1, 0, 0, 16 });
		//else if (LV.start()[J] == 'e')
		//	Colored.push_back({ 0, 1, 0, 16 });
		//else if (LV.start()[J] == 'i')
		//	Colored.push_back({ 0, 0, 1, 16 });
		//else if (LV.start()[J] == 'o')
		//	Colored.push_back({ 1, 1, 0, 16 });
		//else if (LV.start()[J] == 'u')
		//	Colored.push_back({ 0, 1, 1, 16 });
		//else
			Colored.push_back({ 1, 13, 1, 15 });
	}
	return Colored;
}

GLDocumentRenderer::GLDocumentRenderer(Document* D)
	: mDocument(D)
	, mGlyphBuffer("vertex:3f,tex_coord:2f,color:4f")
	, mGlyphBufferDirty(true)
{
	Vertex Vertices[4] = {};

	constexpr static GLuint Indices[] = {0, 1, 2, 0, 2, 3};
	mGlyphBuffer.push_back((const char*)Vertices, 4, Indices, 6);
}

void GLDocumentRenderer::render()
{
	if (true/*mGlyphBufferDirty*/)
	{
		generateGlyphBuffer();
	}
	TextManager::Instance()->renderText(&mGlyphBuffer);
}

void GLDocumentRenderer::generateGlyphBuffer()
{
	//FIXME : we dont need to flush the buffer every time we want to update.
	// We can render the active buffer separately afterwards.
	// NOTE: this effectively renders the buffer optimization useless
	//if (mPendingBufUpdate) {
	//	//flushBuffer();
	//}

	// Clear the current one before recreating it
	mGlyphBuffer.clear();
	//FIXME: I am not sure why we absolutely need a first element.
	Vertex Vertices[4] = {};
	constexpr static unsigned Indices[] = {0, 1, 2, 0, 2, 3};
	mGlyphBuffer.push_back((const char*)Vertices, 4, Indices, 6);


	for (LineView Line : *mDocument)
	{
		auto Cols = AddLineToColorBuffer(Line);
		AddLineToVertexBuffer(Line, mGlyphBuffer, Cols);
	}

	mGlyphBufferDirty = false;
}
