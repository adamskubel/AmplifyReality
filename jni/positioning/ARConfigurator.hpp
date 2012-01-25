//#include "userinterface/uimodel/Panel.hpp"
#include "userinterface/uimodel/GridLayout.hpp"
#include "model/Engine.hpp"
#include "model/Drawable.hpp"
#include "model/IDeletable.hpp"
#include "userinterface/uimodel/NumberSpinner.hpp"



#ifndef POSITION_CONTROL_CONFIGURATOR_HPP_
#define POSITION_CONTROL_CONFIGURATOR_HPP_

using namespace cv;
class ARConfigurator : public IDeletable, public Drawable
{
public:
	ARConfigurator(Engine * engine, UIElementCollection * parent, Point2i position);
	~ARConfigurator();

	void SetDefaults();
		
	void Draw(Mat * rgbaImage);
	bool IsVisible();

	void PositionFilterAlphaChanged(void * sender, NumberSpinnerEventArgs args);
	void RotationFilterAlphaChanged(void * sender, NumberSpinnerEventArgs args);
	void MinimumFinderPatternScoreChanged(void * sender, NumberSpinnerEventArgs args);
	void MinimumAlignmentPatternScoreChanged(void * sender, NumberSpinnerEventArgs args);

	//Params
	float PositionFilterAlpha;
	float RotationFilterAlpha;
	float MinFinderPatternScore;
	float MinAlignmentScore;

	void ToggleVisibility(void * sender, EventArgs args);

private:
	
	bool isVisible;
	vector<GridLayout*> pages;
};

#endif