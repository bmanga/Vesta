#pragma once

class TextEditorOpts
{
public:
	unsigned TabSize;

	struct
	{
		unsigned KeepTabs : 1;
	}OptBits;
};

class VestaOptions
{
public:
	const TextEditorOpts& textEditor() const
	{
		return mTextEditorOpts;
	}
	VestaOptions()
	{
		mTextEditorOpts.TabSize = 4;
		mTextEditorOpts.OptBits.KeepTabs = true;
	}
private:
	TextEditorOpts mTextEditorOpts;
};