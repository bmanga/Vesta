#include "vesta.h"
#include "TextWindow.h"

#include <fstream>
#include <sstream>
#include <QKeyEvent>
#include "EditActions.h"
#include "Document.h"

Vesta::Vesta(QWidget *parent)
	: QMainWindow(parent)
	, Actions(Active)
{
	ui.setupUi(this);


	//QSurfaceFormat Format;

	//Format.setDepthBufferSize(16);
	//Format.setStencilBufferSize(8);
	//Format.setVersion(3, 2);
	//Format.setProfile(QSurfaceFormat::OpenGLContextProfile::CoreProfile);



	this->grabKeyboard();



}

Vesta::~Vesta()
{

}

void Vesta::keyPressEvent(QKeyEvent* Evt)
{
	Actions.analyze(Evt);
}

void Vesta::newEditWindow()
{
	auto File = std::make_shared<Document>("test.txt");
	TextWindow *Window = new TextWindow(nullptr, File);
	Active = Window;
	//TextWindow *Window2 = new TextWindow(nullptr, nullptr);

	Window->setWindowTitle("first window");
	Window->show();

	Window->setFont(22);

	//Window2->setWindowTitle("second window");
	//Window2->show();
	Pen pen = { { 50, 0, 0 },{ 0,0,0,1 } };


	std::ifstream f("test.txt");
	std::string line;
	while (std::getline(f, line)) {
		Window->addText(line.c_str(), pen);
		pen.pos.x = 5;
	}
}
