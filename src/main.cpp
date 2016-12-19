#include "vesta.h"
#include <QtWidgets/QApplication>
#include "EditManager.h"
#include <string>


int main(int argc, char *argv[])
{
	std::string File = "test.txt";
	if (argc > 1)
	{
		File = argv[1];
	}
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QApplication a(argc, argv);
	
	Vesta w;
	//w.show();
	w.newEditWindow(File.c_str());
	return a.exec();
}
