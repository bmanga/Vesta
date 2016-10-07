#pragma once
#include "EditManager.h"
#include "VestaOptions.h"

bool ActionManager::analyze(QKeyEvent* Event) {
	auto Key = Event->key();
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

		if (Text == "\r")
			Text = "\n";
		auto Action = std::make_unique<TypeAction>(Text);
		Request R(std::move(Action));
		return mActiveWindow->handleRequest(std::move(R));
	}

	return false;
}
