#pragma once

#include <string>
#include "Types.h"

class Cursor;
class Document;

class NavigateAction
{
public:
	enum Direction : uint8_t
	{
		Prev,
		Next,
		Up,
		Down,
		EndOfLine,
		StartOfLine,
		EndOfFile,
		StartOfFile
	};

	NavigateAction(Direction Dir)
		: mDirection(Dir) { }

	bool execute(Cursor *C) const;

private:
	Direction mDirection;
};

class EditAction
{
public:
	EditAction(bool canRevert) 
		: mCanRevert(canRevert) { }

	virtual ~EditAction() = default;

	virtual bool commit(Document*, Cursor *) = 0;
	virtual bool revert(Document*, Cursor *) = 0;

	bool canRevert() const { return mCanRevert; };

private:
	bool mCanRevert;
};

class TypeAction : public EditAction
{
public:
	TypeAction(std::string);
	~TypeAction() override = default;

	bool commit(Document*, Cursor*) override;
	bool revert(Document*, Cursor*) override;

private:
	std::string mContent;
	DocPosition mPos;
};

class DeleteAction : public EditAction
{
public:
	DeleteAction(bool BackSpace);
	~DeleteAction() override = default;
	bool commit(Document*, Cursor*) override;
	bool revert(Document*, Cursor*) override;

private:
	std::string mContent;
	DocPosition mPos;
	bool mBackSpace;
};


