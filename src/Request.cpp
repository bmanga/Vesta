#include "Request.h"
#include "EditActions.h"

uptr<EditAction> FinalizedRequest::getAction() {
	return std::move(mAction);
}

Cursor* FinalizedRequest::getCursor() const {
	return mCursor;
}

FinalizedRequest::FinalizedRequest(uptr<EditAction> Action, Cursor* Cursor)
	: mAction(std::move(Action))
    , mCursor(Cursor)
{
}

Request::Request(uptr<EditAction> Action) : mAction(std::move(Action)) {
}

FinalizedRequest Request::operator()(Cursor* Cursor) {
	return{ std::move(mAction), Cursor };
}