#include "VestaOptions.h"


VestaOptions::VestaOptions()
{
	mTextEditorOpts.TabSize = 4;
	mTextEditorOpts.OptBits.KeepTabs = true;
	mFontOpts.Name = "assets/fonts/Consolas.ttf";

	setFontSize(18);
	

}

VestaOptions &GetOptions()
{
	static VestaOptions Opts;
	return Opts;
}
