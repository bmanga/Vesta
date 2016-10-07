#include "EditActions.h"

#include "Cursor.h"
#include "Document.h"


bool NavigateAction::execute(Cursor* C) const
{
	switch (mDirection)
	{
	case Prev:
		C->prev();
		break;
	case Next:
		C->next();
		break;
	case Up:
		C->up();
		break;
	case Down:
		C->down();
		break;
	default:
		;
	}

	return true;
}

TypeAction::TypeAction(std::string S)
	: EditAction(true)
	, mContent(S), mPos{}
{
}

bool TypeAction::commit(Document *D, Cursor *C)
{
	DocPosition NewPos = D->insertChar(C->getPosition(), mContent[0]);

	C->moveTo(NewPos);

	return true;
}

bool TypeAction::revert(Document*, Cursor*)
{
	return true;
}

DeleteAction::DeleteAction(bool BackSpace)
	: EditAction(true),
	mBackSpace(BackSpace)
{
}

bool DeleteAction::commit(Document *D, Cursor *C)
{
	auto CPos = C->getPosition();

	if (mBackSpace) {
		// Cannot backspace-delete at the start of the document
		if (CPos.line().value() == 1 && CPos.column().value() == 1) {
			return false;
		}

		C->prev();
	}
	else
	{
		// Cannot delete past the end of the document
		if (CPos == D->lastLine().endOfLine())
		{
			return false;
		}
	}


	mContent[0] = D->deleteChar(C->getPosition());

	return true;
}

bool DeleteAction::revert(Document*, Cursor*)
{
	return true;
}
