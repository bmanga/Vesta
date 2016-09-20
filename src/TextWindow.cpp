#include "TextWindow.h"
#include "opengl.h"

#include <QOpenGLContext>
#include <QMouseEvent>
#include "VertexBuffer.h"
#include "Document.h"
#include "Cursor.h"
#include "TextManager.h"
#include "Request.h"
#include "EditActions.h"


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






TextWindow::TextWindow(QWidget *Parent, SharedDocument File)
	: QOpenGLWidget(Parent), mDocument(File)
	, mTextBuffer("vertex:3f,tex_coord:2f,color:4f")
{
	//Leave space for the first glyph, which is always going to be the cursor
	GLuint Indices [6] = { 0, 1, 2, 0, 2, 3 };
	Vertex Vertices[4]{};
	mTextBuffer.push_back((const char*)Vertices, 4, Indices, 6);
	mActiveFont = nullptr;

	if (!mDocument)
	{
		mDocument = std::make_shared<Document>();
	}

	mCursor = 
		std::make_unique<Cursor>(mDocument.get(), &mTextBuffer, FontInfo{});
}

void TextWindow::setFont(float Pt)
{
	mActiveFont = TextManager::Instance()->getFont(Pt);
	mCursor->setFontInfo(GetFontInfo(mActiveFont));
	mDocPosSelector.updateFont(mActiveFont);
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





void TextWindow::addText(std::string Text, Pen& pen) {
	
	assert(mActiveFont && "The font must be set before adding any text");
	// Make sure the correct size is available. If not, add it
	pen.pos.y -= mActiveFont->height();
	const ftgl::Glyph* previous = nullptr;
	const char *codepoints = Text.c_str();
	mActiveFont->loadGlyphs(codepoints);
	for (size_t j = 0; 
		j < utf8_strlen(codepoints); 
		j += utf8_surrogate_len(codepoints + j))
	{
		if (codepoints[j] == '\r')
			continue;


		if (codepoints[j] == '\n')
		{
			pen.pos.y -= mActiveFont->height();
			pen.pos.x = 0;
			continue;
		}
		const Glyph *glyph = mActiveFont->getLoadedGlyph(utf8_to_utf32(codepoints + j));
	
		if (codepoints[j] == '\t') {
			// FIXME :
			VestaOptions Opts;
			unsigned TabSize = Opts.textEditor().TabSize;
			pen.pos.x += glyph->advance_x * TabSize;
			continue;
		}


	//for (auto* glyph : ftgl::glyph_range(mActiveFont, Text.c_str())) {
		if (glyph == nullptr) continue;

		// Kerning should be disallowed

		//float kerning = previous ?
		//	glyph->getKerning(previous->codepoint) :
		//	0.0f;

		//pen.pos.x += kerning;


		Vertex vertices[4];
		SetGlyphVertices(glyph, pen, vertices);

		GLuint indices[6] = { 0, 1, 2, 0, 2, 3 };


		mTextBuffer.push_back((const char*)vertices, 4, indices, 6);
		pen.pos.x += glyph->advance_x;
		previous = glyph;
	}
}

void TextWindow::paintGL() {

	// There is nothing to display
	if (mTextBuffer.size() == 0)
		return;

	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	TextManager::Instance()->renderText(&mTextBuffer);
}


void TextWindow::resizeGL(int width, int height) {
	mat4_set_orthographic(&projection, 0, width, 0, height, -1, 1);
	mat4_set_translation(&view, 0, height, 0);

	mDocPosSelector.updateWindowSize({ width, height });
}

void TextWindow::mousePressEvent(QMouseEvent* Evt)
{
	auto Pos = mDocPosSelector.getDocLocation(Evt->pos().x(), Evt->pos().y());
	
	Pen P{ {5, 0, 0}, {0, 0, 1, 1} };
	//mTextBuffer.clear();
	//addText("Hey now why do u not wanna work", P);

	mCursor->moveTo(Pos);
	this->repaint();
}

void TextWindow::keyPressEvent(QKeyEvent* Evt)
{
	if (Evt->matches(QKeySequence::MoveToPreviousChar))
	{
		mCursor->prev();
	}

	if (Evt->matches(QKeySequence::MoveToNextChar)) {
		mCursor->next();
	}

	if (Evt->text() == "c")
	{
		this->repaint();
	}

	this->repaint();
}

void TextWindow::repaint()
{
	clear();
	Pen pen = {{0, 0, 0},{0,0,0,1}};
	addText(mDocument->text(), pen);

	mCursor->updateBuffer();
	QOpenGLWidget::repaint();
}

bool TextWindow::handleRequest(Request R)
{
	if (!mDocument->handleRequest(R(mCursor.get())))
		return false;

	this->repaint();
	
	return true;
}
