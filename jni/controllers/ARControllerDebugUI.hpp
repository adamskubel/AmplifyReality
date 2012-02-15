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

	void NumberSpinnerValueChanged(void * sender, NumberSpinnerEventArgs args);
	void SetDefaults();		
	//void GlobalTouchEvent(void * sender, TouchEventArgs args);
	
	float GetParameter(std::string paramName);
	void AddNewParameter(std::string paramName, float startValue, float step = 1.0f, float minValue = 0.0f, float maxValue = MAXFLOAT, std::string format = "%f", int desiredPage = 0);

	void ToggleVisibility(void * sender, EventArgs args);

	void DrawmodeSelectionChanged(void * sender, SelectionChangedEventArgs args);

	DrawModes::DrawMode currentDrawMode;


private:
	void addPage2(Engine * engine);

	DataDisplay * translationLabel;
	DataDisplay * rotationLabel;
	CertaintyIndicator * certaintyIndicator;
	Label * stateLabel;
	map<std::string,float> parameterMap;
	Size2i GridDimensions;

	void FindNextPosition(Point2i & page, int & pageNum, int desiredPage = 0, Size2i controlSize = Size2i(1,1));
	void AddInNextPosition(GraphicalUIElement * newControl, int desiredPage = 0);



};

#endif