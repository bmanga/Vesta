#pragma once
#include <memory>


template <class T>
using uptr = std::unique_ptr<T>;

class EditAction;
class Cursor;

class FinalizedRequest
{
	friend class Request;
public:
	uptr<EditAction> getAction();
	Cursor *getCursor() const;

private:
	FinalizedRequest(uptr<EditAction> Action, Cursor* Cursor);

private:
	uptr<EditAction> mAction;
	Cursor *mCursor;
};

class Request
{
public:
	Request(uptr<EditAction> Action);
	FinalizedRequest operator()(Cursor* Cursor);
private:
	uptr<EditAction> mAction;
};