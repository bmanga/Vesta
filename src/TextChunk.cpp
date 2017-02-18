#include "TextChunk.h"
#include <VestaOptions.h>
#include "TextManager.h"
#include <sstream>

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


TextChunk::TextChunk(LineView First): mDocSpan(First.lineRange())
                                      , mContentBuffer(First.str())

                                      , mPendingBufUpdate(false)
{
	mNewlines.push_back(0);
	mNewlines.push_back(static_cast<uint16_t>(First.length()));


}

void TextChunk::append(LineView Line)
{
	assert(mNewlines.size() < 255);
	assert(Line.startOfLine() > mDocSpan.end()
		&& "Line must come after what the current buffer spans over");

	mContentBuffer.append(Line.start(), Line.length());
	mNewlines.push_back(static_cast<uint16_t>
		(mNewlines.back() + Line.length()));

	mDocSpan.setEnd(Line.endOfLine());
}

bool TextChunk::contains(Line Ln) const
{
	return mDocSpan.contains(Ln);
}

std::ostream& operator<<(std::ostream& Out, const TextChunk& Chunk)
{
	for (LineView Line : Chunk)
	{
		Out.write(Line.start(), Line.length());
		Out.put('\n');
	}
	return Out;
}

TextChunk::iterator TextChunk::begin() const
{
	return iterator(*this, mDocSpan.start().line());
}

TextChunk::iterator TextChunk::end() const
{
	return iterator(*this, ++(mDocSpan.end().line()));
}

size_t TextChunk::newlineVecOffset(Line Line) const
{
	assert(mDocSpan.contains(Line));

	auto StartLn = mDocSpan.start().line().value();
	auto LineLn = Line.value();

	return LineLn - StartLn;
}

void TextChunk::replace(LineView Line) {
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
	auto Idx = newlineVecOffset(Line);

	auto OffStart = mNewlines[Idx];
	auto OffEnd = mNewlines[Idx + 1];

	// Intentionally negative
	int16_t Size = OffStart - OffEnd;

	mContentBuffer.erase(mContentBuffer.begin() + OffStart, 
		mContentBuffer.begin() + OffEnd);

	mNewlines.erase(mNewlines.begin() + Idx);

	unrolled_vector_add(mNewlines.begin() + Idx, mNewlines.end(), Size);

	DocPosition OldEnd = mDocSpan.end();
	DocPosition NewEnd;
	if (Line != mDocSpan.end().line()) {
		NewEnd = { --OldEnd.line(), OldEnd.column(), OldEnd.character() };
	}
	else {
		NewEnd = { lineAt(--OldEnd.line()).endOfLine() };
	}

	mDocSpan.setEnd(NewEnd);
}

LineView TextChunk::lineAt(Line Line) const
{
	/* FIXME this is a source of bugs at the moment
	if (Line == gActiveLine) {
		return { (const std::string &)gActiveLineBuffer, gActiveLine };
	}
	*/
	auto Idx = newlineVecOffset(Line);

	auto OffStart = mNewlines[Idx];
	auto OffEnd = mNewlines[Idx + 1];

	const char* Start = &mContentBuffer[OffStart];
	unsigned Length = OffEnd - OffStart;

	return {Start, Length, Line};
}

LineView TextChunk::lastLine() const
{
	return lineAt(mDocSpan.end().line());
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

std::string TextChunk::deleteRange(DocRange Rng) 
{
	if (!Rng.isValid()) {
		return "";
	}
	mPendingBufUpdate = true;
	size_t CharCnt = Rng.containedCharacters(this);

	if (Rng.containedLines() == 1) {
		LineView LV = lineAt(Rng.start().line());
		const char *Start = LV.start() + Rng.start().character().offset();
		const char *End = LV.start() + Rng.end().character().offset();
		std::string Deleted{ Start, End };

		StringRef Buf = activeLineBuffer(Rng.start().line());
		Buf->erase(Rng.start().character().offset(), End - Start);
		flushBuffer();

		return Deleted;
	}
	
	// In multiple line deletion, we consider first, last and middle 
	// lines separately.
	// FIXME: We could combine the string retrieval and the deletion operations
	std::stringstream Deleted;
	Line L = Rng.start().line();
	LineView LV = lineAt(L);

	const char *Start = LV.start() + Rng.start().character().offset();
	const char *End = LV.start() + LV.endOfLine().character().offset();

	Deleted << deleteRange({ Rng.start(), LV.endOfLine() });
	// We need a new line here
	Deleted << '\n';

	// Deal with middle lines.
	++L;
	unsigned LDiff = Rng.end().line().offset() - L.offset();
	for (unsigned j = 0; j < LDiff; ++j) {
		Deleted << lineAt(L);
		erase(L);
	}

	// Deal with last line.
	LV = lineAt(L);
	Start = LV.start();
	End = LV.start() + Rng.end().character().offset();

	Deleted.write(Start, End - Start);
	Deleted << '\n';

	// Deal with deletion.
	DocPosition OldEnd = Rng.end();
	DocPosition NewEnd{ Line{OldEnd.line().value() - LDiff }, OldEnd.column(), OldEnd.character() };
	deleteRange({ LV.startOfLine(), NewEnd });

	deleteNewlineAfter(Rng.start().line());

	return Deleted.str();


}

std::pair<std::string, DocPosition> TextChunk::replaceRange(DocRange Rng, const std::string &Str)
{
	std::string Deleted = deleteRange(Rng);

	DocPosition NewPos = insertChar(Rng.start(), Str[0]);
	for (size_t j = 1, Len = Str.length(); j < Len; ++j) {
		NewPos = insertChar(NewPos, Str[j]);
	}
	return{ Deleted, NewPos };
}

DocPosition TextChunk::insertChar(DocPosition Pos, char C)
{
	assert(mDocSpan.contains(Pos.line()));

	uint8_t NumCharRepeats = 1;
	const VestaOptions &Opts = GetOptions();

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
	//gActiveLine = ++Pos.line();

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