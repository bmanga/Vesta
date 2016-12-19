#pragma once
#include <QKeyEvent>
#include "EditActions.h"
#include "Request.h"
#include "TextWindow.h"
#include "Tokenizer.h"

class QKeyEvent;

class ActionManager
{
public:
	ActionManager(TextWindow *&Active);

	bool analyze(QKeyEvent* Event);
private:
	TextWindow *&mActiveWindow;
	TriggerLex mLexer;
};

