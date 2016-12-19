#pragma once
#include <QObject>
#include "Freetype-core/opengl.h"
#include <QOpenGLWidget>
#include <memory>
#include "Request.h"
#include "Freetype-core/VertexBuffer.h"
#include "Types.h"
#include "EditActions.h"
#include "GLDocumentRenderer.h"

using namespace ftgl;



struct WindowSize
{
	int x, y;
};

// Utility class that, given the size of the window, when passed a x,y pos
// returns the location within the document
class ScreenPositionSelector
{
public:
	ScreenPosition getDocLocation(int x, int y) const;


	void updateWindowSize(WindowSize WinSize) {
		mWinSize = WinSize;
	}

	//TODO: need to get the width



private:
	WindowSize mWinSize = { 0, 0 };
};



class TextLocation;
class TextRange;

class LineNumberArea;


/*
* Self documenting type which prevents some blatant attempts at being wrong.
* Note that this does not cover the case of the base copy, move constructors /
* assignment operators. You could sneak a nullptr through there. Not my problem.
*/
template <class T>
struct NotNull : public T
{
	using T::T;
	NotNull(nullptr_t) = delete;
	NotNull& operator=(nullptr_t) = delete;
};



class TextWindow : public QOpenGLWidget
{
	Q_OBJECT;
	using SharedDocument = NotNull<std::shared_ptr<class Document>>;
public:
	TextWindow(QWidget *Parent, SharedDocument File);

	void paintGL() override;

	void clear();

	bool handleRequest(Request Request);
	bool handleAction(NavigateAction Action);
	bool handleAction(DocumentAction Action);

	Cursor *getCursor() {
		return mCursor.get();
	}

	Document *getDocument() {
		return mDocument.get();
	}
private:
	void initialize() const;
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void mousePressEvent(QMouseEvent *Evt) override;
	//void keyPressEvent(QKeyEvent *Evt) override;
	void repaint();


	bool                             mActive = false;
	std::unique_ptr<LineNumberArea>  mLineNumbers = nullptr;
	SharedDocument                   mDocument;
	GLDocumentRenderer               mDocRenderer;
	// The first entry in the buffer is always the cursor
	VertexBuffer mTextBuffer;
	uptr<class Cursor> mCursor;
	ScreenPositionSelector mDocPosSelector;

	bool mDirtyBuffer = true;
};
