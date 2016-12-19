#include "VestaOptions.h"

// FIXME: Ugly workaround until we get guaranteed copy elision.
VestaOptions &Init() 
{
	static VestaOptions Opts;
	return Opts;
}


VestaOptions::VestaOptions()
{
	mTextEditorOpts.TabSize = 4;
	mTextEditorOpts.OptBits.KeepTabs = true;
	mFontOpts.Name = "assets/fonts/Consolas.ttf";

	setFontSize(18);
	

}

const VestaOptions &GetOptions()
{
	return Init();
}

OptionsChanger::OptionsChanger(): mOptions(Init())
{
}
