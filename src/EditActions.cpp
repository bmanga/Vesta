#include "EditActions.h"

#include "Cursor.h"
#include "Document.h"



TypeAction::TypeAction(std::string S)
	: EditAction(true)
	, mContent(S), mPos{}
{
}

bool TypeAction::commit(Document *D, Cursor *C)
{
	//FIXME : this is wrong
	D->insertChar(C->getPosition(), mContent[0]);

	if (mContent[0] == '\n') {
		DocPosition P= C->getPosition();
		C->moveTo({ P.line().value() + 1, 1, 1});
	}
	else 
		C->next(mContent.size());
	return true;
}

bool TypeAction::revert(Document*, Cursor*)
{
	return true;
}

DeleteAction::DeleteAction(unsigned N)
	: EditAction(true)
{
	mContent.resize(N);
}

bool DeleteAction::commit(Document *D, Cursor *C)
{
	mContent[0] = D->deleteChar(C->getPosition());

	if (mContent[0] == '\n')
	{
		C->up();
		C->eol();
	}
	else
		C->prev();

	
	return true;
}

bool DeleteAction::revert(Document*, Cursor*)
{
	return true;
}
