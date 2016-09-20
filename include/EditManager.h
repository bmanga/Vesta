#pragma once
#include <QKeyEvent>
#include "EditActions.h"
#include "Request.h"
#include "TextWindow.h"

class QKeyEvent;

class ActionManager
{
public:
	ActionManager(TextWindow *&Active)
		: mActiveWindow(Active) { }

	bool analyze(QKeyEvent* Event);
private:
	TextWindow *&mActiveWindow;
};

