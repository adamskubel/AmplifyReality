//#include "userinterface/uimodel/Panel.hpp"
#include "userinterface/uimodel/GridLayout.hpp"
#include "model/Engine.hpp"
#include "model/Drawable.hpp"
#include "model/IDeletable.hpp"
#include "userinterface/uimodel/NumberSpinner.hpp"
#include "userinterface/uimodel/PageDisplay.hpp"
#include "userinterface/events/EventDelegates.hpp"


#ifndef POSITION_CONTROL_CONFIGURATOR_HPP_
#define POSITION_CONTROL_CONFIGURATOR_HPP_

namespace DrawModes
{
	enum DrawMode
	{
		ColorImage = 0, GrayImage = 1, BinaryImage = 2
	};
}


using namespace cv;
class ARConfigurator : public PageDisplay
{
public:
	ARConfigurator(Engine * engine);

	void SetDefaults();		

	void PositionFilterAlphaChanged(void * sender, NumberSpinnerEventArgs args);
	void RotationFilterAlphaChanged(void * sender, NumberSpinnerEventArgs args);
	void MinimumFinderPatternScoreChanged(void * sender, NumberSpinnerEventArgs args);
	void MinimumAlignmentPatternScoreChanged(void * sender, NumberSpinnerEventArgs args);

	void GlobalTouchEvent(void * sender, TouchEventArgs args);

	//Params
	float PositionFilterAlpha;
	float RotationFilterAlpha;
	float MinFinderPatternScore;
	float MinAlignmentScore;

	void ToggleVisibility(void * sender, EventArgs args);

	DrawModes::DrawMode currentDrawMode;


};

#endif