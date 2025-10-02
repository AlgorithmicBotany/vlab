#ifndef __PANELPARAMS_H__
#define __PANELPARAMS_H__

namespace PanelParameters
{

	enum
	{
		eMaxNameLength = 32,
			eMaxActionLength = 128,
			eMaxItems = 256,
			eFontHeight = 18,
			eMaxButtonsInGroup = 64,
			eMaxLineLength = 256
	};
	enum ButtonState
	{
		bsOff = 0, 
			bsOn = 1,
			bsMonostable = -1
	};
	enum PanelTarget
	{
		ptLsystem,
			ptViewFile,
			ptFile
	};
	enum TriggerCommand
	{
		tcNewLsystem,
		tcNewView,
		tcNewModel,
		tcRerun,
		tcNothing
	};
	enum Dimension
	{
		dmSliderWidth       = 128,
			dmSliderHeight  =  12,
			dmButtonWidth   = 100,
			dmButtonHeight  =  24,
			dmGroupMargin   =   4,
			dmSelectionMark =   2
	};
	enum PanelItemType
	{
		pitUnknown,
		pitSlider,
			pitButton,
			pitGroup,
			pitLabel
	};
	extern int _AveCharWidth;
	extern int _CharHeight;

	COLORREF DefaultColormap(int);

	extern const Font PanelFont;
}



#else
	#error File already included
#endif
