#ifndef VESTA_H
#define VESTA_H
#include "EditManager.h"

#include "ui_vesta.h"

class TextWindow;

class Vesta : public QMainWindow
{
	Q_OBJECT

public:
	Vesta(QWidget *parent = 0);
	~Vesta();

	void keyPressEvent(QKeyEvent *Evt) override;
	void newEditWindow(const char *Filename);
private:
	Ui::VestaClass ui;
	ActionManager Actions;
	TextWindow *Active;
};

#endif // VESTA_H
