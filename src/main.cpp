#include "vesta.h"
#include <QtWidgets/QApplication>
#include "EditManager.h"
#include <windows.h>


int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QApplication a(argc, argv);
	
	Vesta w;
	w.show();
	w.newEditWindow();
	return a.exec();
}
