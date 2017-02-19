#include "TextWindow.h"
#include "opengl.h"

#include <QOpenGLContext>
#include <QMouseEvent>
#include "VertexBuffer.h"
#include "Document.h"
#include "Cursor.h"
#include "TextManager.h"
#include "Request.h"
#include "VestaOptions.h"


using namespace ftgl;

// ------------------------------------------------------- typedef & struct ---
typedef struct {
	float x, y, z;    // position
	float s, t;       // texture
	float r, g, b, a; // color
} vertex_t;


// ------------------------------------------------------- global variables ---



// --------------------------------------------------------------- add_text ---




class LineNumberArea
{
	
};

ScreenPosition ScreenPositionSelector::getDocLocation(int x, int y) const
{
	x = std::max(x, 0);
	y = std::max(y, 0);
	const VestaOptions &Opts = GetOptions();
	float FontHeight = Opts.font().Height;
	float FontWidth = Opts.font().Width;
	unsigned LineAt = unsigned(y) / FontHeight;

	return {LineAt + 1, unsigned(x / FontWidth) + 1};
}

Line ScreenPositionSelector::getLastVisibleLine() const
{
	const VestaOptions &Opts = GetOptions();
	float FontHeight = Opts.font().Height;
	unsigned NumLines = static_cast<unsigned>(mWinSize.y / FontHeight);
	return Line{ NumLines };
}

TextWindow::TextWindow(QWidget *Parent, SharedDocument File)
	: QOpenGLWidget(Parent), mDocument(File)
	, mDocRenderer(mDocument.get())
{
	mCursor = 
		std::make_unique<Cursor>(mDocument.get());
}


void TextWindow::initializeGL() {
	glewExperimental = GL_TRUE;
	GLenum Error = glewInit();
	if (Error != GLEW_OK){
		printf("Error initializing GLEW: %p\n", glewGetErrorString(Error));
	}

	initialize();
}


void TextWindow::clear()
{
	//TODO
}
void TextWindow::initialize() const
{
	mat4_set_identity(&projection);
	mat4_set_identity(&model);
	mat4_set_identity(&view);
	mat4_set_orthographic(&projection, 0, width(), 0, height(), -1, 1);
	glClearColor(1, 1, 1, 1);


	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


}


void TextWindow::paintGL() {

	// There is nothing to display
	//if (mTextBuffer.empty())
	//	return;

	glClearColor(0.05, 0.05, 0.05, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//TextManager::Instance()->renderText(&mTextBuffer);
	if (mDirtyBuffer) {
		mDocRenderer.generateGlyphBuffer(mVisibleLines.first, mVisibleLines.second);
	}
	mDocRenderer.render();
	mDirtyBuffer = false;
	
	mCursor->render();
}


void TextWindow::resizeGL(int width, int height) {
	mat4_set_orthographic(&projection, 0, width, 0, height, -1, 1);
	mat4_set_translation(&view, 0, height, 0);

	mDocPosSelector.updateWindowSize({ width, height });
	mVisibleLines.second = mVisibleLines.first + 
		mDocPosSelector.getLastVisibleLine().offset();
	//mDirtyBuffer = true;
}

void TextWindow::mousePressEvent(QMouseEvent* Evt)
{
	auto SPos = mDocPosSelector.getDocLocation(Evt->pos().x(), Evt->pos().y());
	SPos.adjustByLineOffset(mVisibleLines.first);
	
	Pen P{ {5, 0, 0}, {0, 0, 1, 1} };

	auto Pos = mDocument->position(SPos);
	mCursor->moveTo(Pos);
	mCursor->highlight({ Pos, Pos });
	this->repaint();
}

void TextWindow::mouseMoveEvent(QMouseEvent * Evt) 
{
	if (!(Evt->buttons() & Qt::LeftButton)) {
		return;
	}

	auto SPos = mDocPosSelector.getDocLocation(Evt->pos().x(), Evt->pos().y());
	SPos.adjustByLineOffset(mVisibleLines.first);
	auto EndPos = mDocument->position(SPos);
	auto StartPos = mCursor->getSelection().start();

	mCursor->highlight({ StartPos, EndPos });
	mCursor->moveTo(EndPos);
	this->repaint();

}

//FIXME replace once std::clamp is available
namespace std {
	template <class T>
	const T& clamp(const T& Val, const T&Lo, const T&Hi) 		{
		return std::max(std::min(Val, Hi), Lo);
	}
}

void TextWindow::wheelEvent(QWheelEvent * Evt) {
	int NumSteps = -Evt->delta() / 80;

	unsigned First = mVisibleLines.first.value() + NumSteps;
	First = std::clamp(First, 1u, mDocument->lastLine().line().value());

	mVisibleLines.first = Line{ First };
	mVisibleLines.second += First;

	mDirtyBuffer = true;

	repaint();
}


void TextWindow::repaint()
{
	//if (mDirtyBuffer) {
	//	clear();
	//	mDocument->render();
	//	mDirtyBuffer = false;
	//}

	mCursor->updateBuffer(mVisibleLines.first);
	QOpenGLWidget::repaint();
}

bool TextWindow::handleRequest(Request R)
{
	mDirtyBuffer = true;

	if (!mDocument->handleRequest(R(mCursor.get())))
		return false;

	this->repaint();
	
	return true;
}

bool TextWindow::handleAction(NavigateAction Action)
{
	bool Res = Action.execute(mCursor.get());

	repaint();
	return Res;
}

bool TextWindow::handleAction(DocumentAction Action)
{
	mDirtyBuffer = true;
	bool Success = Action.execute(mDocument.get());

	repaint();
	return Success;
}
