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
#include "userinterface/uimodel/TabDisplay.hpp"

namespace DrawModes
{
	enum DrawMode
	{
		ColorImage = 0, GrayImage = 1, BinaryImage = 2
	};
}


class ARControllerDebugUI : public TabDisplay
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
	
	float GetParameter(std::string paramKey);
	float GetFloatParameter(std::string paramKey);
	bool GetBooleanParameter(std::string paramKey);
	int GetIntegerParameter(std::string paramKey);
	
	//float GetFloatParameter(std::string paramKey, float defaultValue = 0);
	//bool GetBooleanParameter(std::string paramKey, bool defaultValue = false);
	//int GetIntegerParameter(std::string paramKey, int defaultValue = 0);


	void AddNewParameter(std::string paramName, std::string paramKey, float startValue, float step, float minValue, float maxValue, std::string format,string category, bool mapOnly = false);
	void AddNewParameter(std::string paramName, float startValue, float step, float minValue, float maxValue, std::string format, string category, bool mapOnly = false);

	void SetLabelValue(std::string labelName, float value);
	void AverageLabelValue(std::string labelName, float value);
	void AddNewLabel(std::string labelName, std::string suffex, string category = "Data");

	void ToggleVisibility(void * sender, EventArgs args);

	void DrawmodeSelectionChanged(void * sender, SelectionChangedEventArgs args);

	DrawModes::DrawMode currentDrawMode;
	void SetFPS(float fps);


private:
	void addPage2(Engine * engine);

	DataDisplay * translationLabel;
	DataDisplay * rotationLabel;
	CertaintyIndicator * certaintyIndicator;
	Label * stateLabel;
	map<std::string,float> parameterMap;
	map<std::string,pair<Label*,std::string> > labelMap;
	Size2i GridDimensions;

	void FindNextPosition(PageDisplay * pageObject, Point2i & page, int & pageNum, int desiredPage = 0, Size2i controlSize = Size2i(1,1));
	void AddInNextPosition(GraphicalUIElement * newControl, string tabName, int desiredPage = 0);



};

#endif