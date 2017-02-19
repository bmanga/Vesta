#pragma once
#include <VertexBuffer.h>
#include "TextManager.h"

class Document;

struct TinyColor
{
	unsigned r : 4;
	unsigned g : 4;
	unsigned b : 4;
	unsigned a : 4;
};

class GLDocumentRenderer
{
public:
	GLDocumentRenderer(Document* D);

	void render();

	void generateGlyphBuffer(Line First, Line Last);
private:
	Document *mDocument;
	ftgl::VertexBuffer mGlyphBuffer;
	std::vector<TinyColor> mColorBuffer;
	bool mGlyphBufferDirty : 1;
};