#include <fstream>
#include "Document.h"
#include "EditActions.h"


Document::Document(fs::path File)
	: mFilePath(File)
	, mActiveLine(1)
{
}

void Document::open()
{
	mChunks.clear();

	std::ifstream Doc(mFilePath);

	std::string LineBuf;

	Line LineCnt{ 1 };

	uint32_t NewLineOffset = 0;

	std::getline(Doc, LineBuf);
	mChunks.emplace_back(LineView{ LineBuf, LineCnt++ });

	while(std::getline(Doc, LineBuf))
	{
		LineView LV{ LineBuf, LineCnt++ };

		mChunks[0].append(LV);
		
	}
}


char Document::deleteChar(DocPosition Pos)
{
	auto ChunkIt = containingTextChunk(Pos.line());

	return ChunkIt->deleteChar(Pos);
}

DocPosition Document::position(ScreenPosition SPos)
{
	LineView LV = lineAt(SPos.line());

	// We assume we are past the the end of the document. We return
	// the last line 
	if (!LV.isValid())
	{
		return mChunks.back().lastLine().endOfLine();
	}
	return LV.position(SPos);
}

bool Document::handleRequest(FinalizedRequest Request)
{
	auto Action = Request.getAction();
	auto *Cursor = Request.getCursor();
	Action->commit(this, Cursor);
	return true;
}

DocPosition Document::insertChar(DocPosition Pos, char C)
{
	auto ChunkIt = containingTextChunk(Pos.line());

	return ChunkIt->insertChar(Pos, C);
}

LineView Document::lineAt(Line Ln)
{
	auto ChunkIt = containingTextChunk(Ln);

	if (ChunkIt == mChunks.end())
	{
		return LineView::Invalid();
	}

	return ChunkIt->lineAt(Ln);
}
