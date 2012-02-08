#ifndef AR_DEBUG_UI_
#define AR_DEBUG_UI_

#include "userinterface/uimodel/Label.hpp"
#include "userinterface/uimodel/CertaintyIndicator.hpp"
#include "userinterface/uimodel/GridLayout.hpp"
#include "userinterface/uimodel/InputScaler.hpp"
#include "userinterface/events/EventDelegates.hpp"
#include "userinterface/uimodel/SelectBox.hpp"
#include "userinterface/uimodel/NumberSpinner.hpp"
#include "userinterface/uimodel/DataDisplay.hpp"
#include "userinterface/uimodel/PageDisplay.hpp"

namespace DrawModes
{
	enum DrawMode
	{
		ColorImage = 0, GrayImage = 1, BinaryImage = 2
	};
}


class ARControllerDebugUI : public PageDisplay
{
public:
	ARControllerDebugUI(Engine * engine, Point2i position);
	void SetTranslation(Mat * data);
	void SetRotation(Mat * data);
	void SetPositionCertainty(float certainty);
	void SetStateDisplay(string stateDescription);

	
	void SetDefaults();		

	void PositionFilterAlphaChanged(void * sender, NumberSpinnerEventArgs args);
	void RotationFilterAlphaChanged(void * sender, NumberSpinnerEventArgs args);
	void MinimumFinderPatternScoreChanged(void * sender, NumberSpinnerEventArgs args);
	void MinimumAlignmentPatternScoreChanged(void * sender, NumberSpinnerEventArgs args);
	void FastThresholdChanged(void * sender, NumberSpinnerEventArgs args);

	void GlobalTouchEvent(void * sender, TouchEventArgs args);

	//Params
	float PositionFilterAlpha;
	float RotationFilterAlpha;
	float MinFinderPatternScore;
	float MinAlignmentScore;
	float FastThreshold;

	void ToggleVisibility(void * sender, EventArgs args);

	void DrawmodeSelectionChanged(void * sender, SelectionChangedEventArgs args);

	DrawModes::DrawMode currentDrawMode;


private:
	void addPage2(Engine * engine);

	DataDisplay * translationLabel;
	DataDisplay * rotationLabel;
	CertaintyIndicator * certaintyIndicator;
	Label * stateLabel;


};

#endif