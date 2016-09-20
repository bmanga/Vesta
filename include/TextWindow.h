#pragma once
#include "Freetype-core/opengl.h"
#include <QObject>
#include <QOpenGLWidget>
#include <memory>
#include "Request.h"
#include <Freetype-core/VertexBuffer.h>
#include "Types.h"




using namespace ftgl;



struct WindowSize
{
	int x, y;
};

// Utility class that, given the size of the window, when passed a x,y pos
// returns the location within the document
class DocPositionSelector
{
public:
	DocPosition getDocLocation(int x, int y) const {
		unsigned NumLines = static_cast<unsigned>(mWinSize.y / mFontHeight);
		unsigned LineAt = unsigned(y) / mFontHeight;



		return { ++LineAt, unsigned(x / mFontWidth) + 1, 1};
	}

	void updateWindowSize(WindowSize WinSize) {
		mWinSize = WinSize;
	}

	void updateFont(Font *Font) {
		mFontHeight = Font->height();

		// We need to get the width of the glyphs, assuming a monospace font.
		// We use 'e' as it is usually the most common letter
		mFontWidth = Font->getGlyph("e")->advance_x;
	}
	//TODO: need to get the width

	float fontWidth() const {
		return mFontWidth;
	}
	float fontHeight() const{
		return mFontHeight;
	}

private:
	WindowSize mWinSize = { 0, 0 };

	float mFontWidth = 0;
	float mFontHeight = 0;
};



class TextLocation;
class TextRange;

class LineNumberArea;







class TextWindow : public QOpenGLWidget
{
	Q_OBJECT;
	using SharedDocument = std::shared_ptr<class Document>;
public:
	TextWindow(QWidget *Parent, SharedDocument File);

	void paintGL() override;

	void addText(std::string Text, class Pen &pen);

	void clear();

	void setFont(float Pt);

	bool handleRequest(Request Request);
private:
	void initialize() const;
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void mousePressEvent(QMouseEvent *Evt) override;
	void keyPressEvent(QKeyEvent *Evt) override;
	void repaint();


	bool                             mActive = false;
	std::unique_ptr<LineNumberArea>  mLineNumbers = nullptr;
	SharedDocument                   mDocument;

	// The first entry in the buffer is always the cursor
	VertexBuffer mTextBuffer;
	Font *mActiveFont;
	uptr<class Cursor> mCursor;
	DocPositionSelector mDocPosSelector;

	
};
