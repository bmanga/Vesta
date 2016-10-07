#pragma once

#include "TextManager.h"
#include "Freetype-core/TextureFont.h"


namespace {
class TextEditorOpts
{
public:
	uint8_t TabSize;

	struct
	{
		unsigned KeepTabs : 1;
	}OptBits;
};

class FontOpts
{
public:
	ftgl::Font *Font;
	std::string Name;
	float Width;
	float Height;
	uint8_t Pt;
};
}
class VestaOptions
{
	friend VestaOptions &GetOptions();
	friend class OptionsChanger;
public:
	VestaOptions(const VestaOptions &) = delete;
	VestaOptions(VestaOptions &&) = delete;
	VestaOptions& operator= (const VestaOptions &) = delete;
	VestaOptions& operator= (VestaOptions &&) = delete;

	const TextEditorOpts& textEditor() const {
		return mTextEditorOpts;
	}

	const FontOpts &font() const {
		return mFontOpts;
	}
private:
	VestaOptions();

	void setFontSize(uint8_t Pt)
	{
		mFontOpts.Font = TextManager::Instance()->getFont(Pt);
		mFontOpts.Height = mFontOpts.Font->height();
		mFontOpts.Width = mFontOpts.Font->getGlyph("e")->advance_x;
	}


private:
	TextEditorOpts mTextEditorOpts;
	FontOpts mFontOpts;
};

VestaOptions &GetOptions();

// This is the class that allows to modify the VestaOptions
class OptionsChanger
{
public:
	OptionsChanger()
		: mOptions(GetOptions()){ }

	void setFontSize(uint8_t Pt)
	{
		mOptions.setFontSize(Pt);
	}
private:
	VestaOptions &mOptions;
};



