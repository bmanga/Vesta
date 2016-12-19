#pragma once
#include "EditManager.h"
#include "Types.h"
#include "Cursor.h"


int gOpenBraces = 0;
bool gOpenBraceEvent = false;

void callback(TextWindow *TW, char C) {
	TW->handleRequest(Request(std::make_unique<TypeAction>(" Puttana")));
}

void callback2(TextWindow *TW, char C) {
	TW->handleRequest(Request(std::make_unique<TypeAction>(" Cane")));
}

class Completion {
public:
	Completion() = default;
	virtual bool handleEvent(QKeyEvent* Evt) = 0;

	Completion *getNested() {
		return mNested.get();
	}

	virtual ~Completion() = default;

protected:
	std::unique_ptr<Completion> mNested = nullptr;
};

class ScopeBlockCompletion : public Completion
{
public:
	ScopeBlockCompletion(Document *Doc, Cursor * Cur) 
		: mCol{ Cur->getPosition().column() } {
	}

	bool handleEvent(QKeyEvent* Evt) override {
		return false;
	}

private:
	Column mCol;
};


ActionManager::ActionManager(TextWindow *& Active)
	: mActiveWindow(Active) {
	Trigger Test{ "Dio", callback2, Trigger::ActivationKind::Tab };
	mLexer.add(std::move(Test));
	Trigger Test2{ "Madonna", callback };
	mLexer.add(std::move(Test2));

	Trigger Test3{ "Silly->",
		[](TextWindow *TW, char C)
	{
		TW->handleRequest(Request(std::make_unique<TypeAction>("Cunt")));
		//TW->handleAction(NavigateAction::Prev);
	},
	Trigger::ActivationKind::Tab};

	mLexer.add(std::move(Test3));
}

bool ActionManager::analyze(QKeyEvent* Event) {
	char NewChar = Event->text()[0].toLatin1();
	if (NewChar != 0)
		mLexer.processInput(mActiveWindow, NewChar);
	uptr<MultiEditAction> actions = std::make_unique<MultiEditAction>();

	auto Key = Event->key();

	if (Event->matches(QKeySequence::Save))
	{
		return mActiveWindow->handleAction(DocumentAction::SAVE);
	}
	if (Key == Qt::Key_Backspace) {
		auto Action = std::make_unique<DeleteAction>(true);
		Request R(std::move(Action));

		return mActiveWindow->handleRequest(std::move(R));
	}
	if (Key == Qt::Key_Delete) {
		auto Action = std::make_unique<DeleteAction>(false);
		Request R(std::move(Action));

		return mActiveWindow->handleRequest(std::move(R));
	}


	switch (Event->key()) {
	case Qt::Key_Up:
		return mActiveWindow->handleAction(NavigateAction::Up);
	case Qt::Key_Down:
		return mActiveWindow->handleAction(NavigateAction::Down);
	case Qt::Key_Left:
		return mActiveWindow->handleAction(NavigateAction::Prev);
	case Qt::Key_Right:
		return mActiveWindow->handleAction(NavigateAction::Next);
		
	default:
		;
	}

	if (Event->text() != "") {
		
		std::string Text = Event->text().toStdString();
		// We don't support unicode yet
		if (Text.size() > 1)
			return false;

		if (Text == "\r") {
			Text = "\n";

			if (gOpenBraceEvent) {
				actions->pushBack(std::make_unique<TypeAction>("\n"));
				for (int j = 0; j < gOpenBraces; ++j)
					actions->pushBack(std::make_unique<TypeAction>("\t"));
				actions->pushBack(std::make_unique<TypeAction>("\n"));

				for (int j = 0; j < gOpenBraces - 1; ++j)
					actions->pushBack(std::make_unique<TypeAction>("\t"));

				mActiveWindow->handleRequest(Request(std::move(actions)));
				mActiveWindow->handleAction(NavigateAction::Up);
				mActiveWindow->handleAction(NavigateAction::EndOfLine);
				return true;

			}
			
		}
		actions->pushFront(std::make_unique<TypeAction>(Text));
		Request R(std::move(actions));
		return mActiveWindow->handleRequest(std::move(R));
	}

	return false;
}
