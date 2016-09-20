#pragma once

#include <string>
#include "Types.h"

class Cursor;
class Document;

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
	DeleteAction(unsigned N);
	~DeleteAction() override = default;
	bool commit(Document*, Cursor*) override;
	bool revert(Document*, Cursor*) override;

private:
	std::string mContent;
	DocPosition mPos;
};


