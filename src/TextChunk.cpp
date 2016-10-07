#include "TextChunk.h"
#include <VestaOptions.h>
#include "TextManager.h"

namespace __Impl {
static std::string __gActiveLineBuffer;
}


static Line gActiveLine{ 1 };
static StringRef gActiveLineBuffer(__Impl::__gActiveLineBuffer);
static TextChunk *gpActiveTextChunk = nullptr;



// This is a hand unrolled version to help the optimizer for sse2 instructions
// std::for_each with a lambda did not do a good enough job :(
using VShortIt = std::vector<uint16_t>::iterator;
static void unrolled_vector_add(VShortIt Begin, VShortIt End, int16_t N) {

	auto Sz = End - Begin;
	auto Iters = Sz >> 3;

	size_t Idx = 0;
	while (Iters--) {
		Begin[Idx] += N;
		Begin[Idx + 1] += N;
		Begin[Idx + 2] += N;
		Begin[Idx + 3] += N;
		Begin[Idx + 4] += N;
		Begin[Idx + 5] += N;
		Begin[Idx + 6] += N;
		Begin[Idx + 7] += N;
		Idx += 8;
	}

	switch (Sz - Idx) {
	case 7:
		Begin[Idx + 6] += N;
	case 6:
		Begin[Idx + 5] += N;
	case 5:
		Begin[Idx + 4] += N;
	case 4:
		Begin[Idx + 3] += N;
	case 3:
		Begin[Idx + 2] += N;
	case 2:
		Begin[Idx + 1] += N;
	case 1:
		Begin[Idx] += N;
	default:;
	}

}



static void AddLineToVertexBuffer(LineView Line, VertexBuffer &Buffer)
{
	const VestaOptions &Opts = GetOptions();

	Pen Pos{ { 0, 0, 0 },{ 0,0,0,1 } };

	//FIXME: hate the negative sign
	Pos.pos.y -= Line.line().value() * Opts.font().Height;

	Font *F = Opts.font().Font;

	const char *Codepoints = Line.start();
	size_t Len = Line.length();

	for (size_t J = 0;
		J < Len;
		J += utf8_surrogate_len(Codepoints + J))
	{
		if (Codepoints[J] == '\t')
		{
			unsigned TabSize = Opts.textEditor().TabSize;
			Pos.pos.x += Opts.font().Width * TabSize;
			continue;
		}
		const Glyph *G = F->getGlyph(Codepoints + J);

		if (!G)
		{
			continue;
		}

		Vertex Vertices[4];

		SetGlyphVertices(G, Pos, Vertices);

		constexpr static GLuint Indices[] = { 0, 1, 2, 0, 2, 3 };
		Buffer.push_back((const char*)Vertices, 4, Indices, 6);
		Pos.pos.x += Opts.font().Width;
	}

}

void TextChunk::replace(LineView Line) {
	mGlyphBufferDirty = true;

	auto Idx = newlineVecOffset(Line.line());

	auto BuffOffStart = mNewlines[Idx];
	auto BuffOffEnd = mNewlines[Idx + 1];

	// Calculate the size difference between the two strings 
	auto CurrSize = BuffOffEnd - BuffOffStart;
	int Diff = static_cast<int>(Line.length()) - CurrSize;

	// Update the buffer
	mContentBuffer.replace(BuffOffStart, CurrSize, Line.start(), Line.length());

	// Update the indices
	unrolled_vector_add(mNewlines.begin() + Idx + 1, mNewlines.end(), Diff);
}

void TextChunk::insert(LineView Line)
{
	mGlyphBufferDirty = true;
	auto Idx = newlineVecOffset(Line.line());

	mContentBuffer.insert(mNewlines[Idx], Line.start(), Line.length());

	// Insert the new newline
	mNewlines.insert(mNewlines.begin() + Idx + 1,
	                 mNewlines[Idx]);

	// Update the indices
	unrolled_vector_add(mNewlines.begin() + Idx + 1, mNewlines.end(), 
		Line.length());
}

void TextChunk::erase(Line Line)
{
	mGlyphBufferDirty = true;
	auto Idx = newlineVecOffset(Line);

	auto OffStart = mNewlines[Idx];
	auto OffEnd = mNewlines[Idx + 1];

	// Intentionally negative
	int16_t Size = OffStart - OffEnd;

	mContentBuffer.erase(mContentBuffer.begin() + OffStart, 
		mContentBuffer.begin() + OffEnd);

	mNewlines.erase(mNewlines.begin() + Idx);

	unrolled_vector_add(mNewlines.begin() + Idx, mNewlines.end(), Size);
}

StringRef TextChunk::activeLineBuffer(Line Ln, bool ForceUpdate)
{
	// We are the first ones to modify the buffer. Set the active to us.
	if (!gpActiveTextChunk)
	{
		gActiveLine = Ln;
		gpActiveTextChunk = this;
		gActiveLineBuffer->assign(lineAt(Ln).str());
		
		return gActiveLineBuffer;
	}

	// We are working on a separate line and we have made some changes.
	// Store them in the previously active TextChunk buffer.
	if ((Ln != gActiveLine && mPendingBufUpdate) || ForceUpdate)
	{
		LineView LV{ (std::string const&) gActiveLineBuffer, gActiveLine };

		gpActiveTextChunk->replace(LV);
		gpActiveTextChunk->mPendingBufUpdate = false;

		// Set this line and TextChunk as the active ones, and generate the new
		// buffer.
		gActiveLine = Ln;
		gpActiveTextChunk = this;

		LineView CurLine = lineAt(Ln);
		gActiveLineBuffer->assign(CurLine.start(), CurLine.length());

	}

	return gActiveLineBuffer;
}

void TextChunk::render()
{
	if (true/*mGlyphBufferDirty*/)
	{
		generateGlyphBuffer();
	}
	TextManager::Instance()->renderText(&mGlyphBuffer);
}

char TextChunk::deleteChar(DocPosition Pos)
{
	assert(contains(Pos.line()));
	mPendingBufUpdate = true;
	
	StringRef Buf = activeLineBuffer(Pos.line());

	// We are trying to delete the end of line
	if (Buf->length() == Pos.character().offset())
	{
		deleteNewlineAfter(Pos.line());
		return '\n';
	}
	unsigned CharOff = Pos.character().offset();
	char Deleted = Buf[CharOff];
	Buf->erase(CharOff, 1);
	flushBuffer();
	return Deleted;
}

DocPosition TextChunk::insertChar(DocPosition Pos, char C)
{
	assert(mDocSpan.contains(Pos.line()));

	uint8_t NumCharRepeats = 1;
	VestaOptions &Opts = GetOptions();

	if (C == '\n')
	{
		insertNewline(Pos);
		return DocPosition{Pos.line().value() + 1, 1, 1};
	}

	unsigned CurrCol = Pos.column().value();

	if (C == '\t')
	{
		CurrCol += Opts.textEditor().TabSize;
		if (!(Opts.textEditor().OptBits.KeepTabs))
		{
			NumCharRepeats = Opts.textEditor().TabSize;
			C = ' ';
		}
	}
	else
	{
		++CurrCol;
	}

	mPendingBufUpdate = true;
	StringRef Buf = activeLineBuffer(Pos.line());
	unsigned CharOff = Pos.character().offset();

	Buf->insert(CharOff, NumCharRepeats, C);

	flushBuffer();

	return DocPosition{ Pos.line().value(), 
		CurrCol,
		Pos.character().value() + NumCharRepeats };
}

void TextChunk::insertNewline(DocPosition Pos)
{
	activeLineBuffer(Pos.line(), true);

	mNewlines.insert(mNewlines.begin() + Pos.line().value(),
	                 mNewlines[Pos.line().offset()] + Pos.character().offset());

	unsigned LastLn = mDocSpan.end().line().value();
	unsigned LastCl = mDocSpan.end().column().value();
	unsigned LastCh = mDocSpan.end().character().value();
	mDocSpan.setEnd(DocPosition{++LastLn, LastCl, LastCh});

	// Force update the globals
	gActiveLineBuffer->assign(lineAt(Pos.line()).str());
	gActiveLine = Pos.line();
}

void TextChunk::flushBuffer()
{
 	replace(LineView{ (std::string const&)gActiveLineBuffer, gActiveLine });
	mPendingBufUpdate = false;
}

void TextChunk::deleteNewlineAfter(Line Ln)
{
	// Force the current line to be uploaded before joining with the next
	StringRef Buf = activeLineBuffer(Ln, true);

	mNewlines.erase(mNewlines.begin() + Ln.value());

	unsigned LastLine = mDocSpan.end().line().value();

	mDocSpan.setEnd(lineAt(Line{--LastLine}).endOfLine());

	// Force update the globals
	gActiveLineBuffer->assign(lineAt(Ln).str());
	gActiveLine = Ln;

	//TODO: notify document of the deleted line

	mPendingBufUpdate = false;
}

void TextChunk::generateGlyphBuffer()
{
	//FIXME : we dont need to flush the buffer every time we want to update.
	// We can render the active buffer separately afterwards.
	// NOTE: this effectively renders the buffer optimization useless
	if (mPendingBufUpdate) {
		//flushBuffer();
	}

	// Clear the current one before recreating it
	mGlyphBuffer.clear();

	for (Line Ln = mDocSpan.start().line();
		Ln <= mDocSpan.end().line(); 
		++Ln)
	{
		//// The active line is not up to date. Its real content 
		//// is in the active buffer.
		//if (Ln == gActiveLine)
		//{
		//	LineView ActiveLine{ (const std::string&)gActiveLineBuffer, Ln };
		//	AddLineToVertexBuffer(ActiveLine, mGlyphBuffer);

		//	continue;
		//}
		LineView Line = lineAt(Ln);

		AddLineToVertexBuffer(Line, mGlyphBuffer);
	}

	mGlyphBufferDirty = false;
}
