#pragma once
#include "EditManager.h"

bool ActionManager::analyze(QKeyEvent* Event) {
	if (Event->matches(QKeySequence::Back)) {
		auto Action = std::make_unique<DeleteAction>(1);
		Request R(std::move(Action));
		return mActiveWindow->handleRequest(std::move(R));
	}

	else if (Event->text() != "") {
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

