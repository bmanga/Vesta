#include <fstream>
#include <sstream>
#include <iostream> //Debug purposes
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

	TextChunk *Active = &mChunks[0];
	unsigned LineN = 0;
	while(std::getline(Doc, LineBuf))
	{
		LineView LV{ LineBuf, LineCnt++ };

		if (++LineN == 100)
		{
			LineN = 0;
			mChunks.emplace_back(LV);
			Active = &mChunks.back();
		}
		else {
			Active->append(LV);
		}
		
	}
}

void Document::save()
{
	std::ofstream File(mFilePath);

	for (auto& Chunk : mChunks)
	{
		File << Chunk;
	}

}

//std::ostringstream FileData;
//for (auto& Chunk : mChunks) {
//	FileData << Chunk;
//}
//
//std::string FileStr = FileData.str();
//FileStr.pop_back();
//
//std::ofstream File(mFilePath);
//File << FileStr;

char Document::deleteChar(DocPosition Pos)
{
	auto ChunkIt = containingTextChunk(Pos.line());

	return ChunkIt->deleteChar(Pos);
}

std::string Document::deleteRange(DocRange Rng) 
{
	// FIXME we assume the whole range is within the same chunk
	auto ChunkIt = containingTextChunk(Rng.start().line());
	return ChunkIt->deleteRange(Rng);
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

std::pair<std::string, DocPosition> Document::replaceRange(DocRange Rng,
	const std::string & Str)
{
	return containingTextChunk(Rng.start().line())->replaceRange(Rng, Str);
}


LineView Document::lineAt(Line Ln) const
{
	auto ChunkIt = containingTextChunk(Ln);

	if (ChunkIt == mChunks.end())
	{
		return LineView::Invalid();
	}

	return ChunkIt->lineAt(Ln);
}
