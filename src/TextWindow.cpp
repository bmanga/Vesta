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
	// FIXME
	VestaOptions &Opts = GetOptions();
	float FontHeight = Opts.font().Height;
	float FontWidth = Opts.font().Width;
	unsigned NumLines = static_cast<unsigned>(mWinSize.y / FontHeight);
	unsigned LineAt = unsigned(y) / FontHeight;

	return {++LineAt, unsigned(x / FontWidth) + 1};
}

TextWindow::TextWindow(QWidget *Parent, SharedDocument File)
	: QOpenGLWidget(Parent), mDocument(File)
	, mTextBuffer("vertex:3f,tex_coord:2f,color:4f")
{
	//Leave space for the first glyph, which is always going to be the cursor
	GLuint Indices [6] = { 0, 1, 2, 0, 2, 3 };
	Vertex Vertices[4]{};
	mTextBuffer.push_back((const char*)Vertices, 4, Indices, 6);

	if (!mDocument)
	{
		mDocument = std::make_shared<Document>();
	}

	mCursor = 
		std::make_unique<Cursor>(mDocument.get(), FontInfo{});
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
	mTextBuffer.clear();

	Vertex Vertices[4]{};
	GLuint Indices[6] = { 0, 1, 2, 0, 2, 3 };
	mTextBuffer.push_back((const char*)Vertices, 4, Indices, 6);
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

	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//TextManager::Instance()->renderText(&mTextBuffer);
	//if(mDirtyBuffer)
	mDocument->render();
	mCursor->render();
}


void TextWindow::resizeGL(int width, int height) {
	mat4_set_orthographic(&projection, 0, width, 0, height, -1, 1);
	mat4_set_translation(&view, 0, height, 0);

	mDocPosSelector.updateWindowSize({ width, height });
}

void TextWindow::mousePressEvent(QMouseEvent* Evt)
{
	auto SPos = mDocPosSelector.getDocLocation(Evt->pos().x(), Evt->pos().y());
	
	Pen P{ {5, 0, 0}, {0, 0, 1, 1} };

	auto Pos = mDocument->position(SPos);
	mCursor->moveTo(Pos);
	this->repaint();
}


void TextWindow::repaint()
{
	//if (mDirtyBuffer) {
	//	clear();
	//	mDocument->render();
	//	mDirtyBuffer = false;
	//}

	mCursor->updateBuffer();
	QOpenGLWidget::repaint();
}

bool TextWindow::handleRequest(Request R)
{
	mDirtyBuffer = !mDirtyBuffer;

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
