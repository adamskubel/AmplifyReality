#include "ARControllerDebugUI.hpp"

ARControllerDebugUI::ARControllerDebugUI(Engine * engine, Point2i position) : PageDisplay()
{	
	int padding = 20;
	GridLayout * myGrid = new GridLayout(Size2i(4,3));
	
	rotationLabel = new DataDisplay("%3.2lf",Colors::Red);	
	translationLabel = new DataDisplay("%3.2lf",Colors::Blue);

	stateLabel = new Label("[State]",Point2i(0,0),Colors::Blue);

	myGrid->AddChild(rotationLabel,Point2i(0,1));
	myGrid->AddChild(translationLabel,Point2i(0,2));
	myGrid->AddChild(stateLabel,Point2i(0,0));

	Mat oneMatrix = Mat::ones(1,3,CV_64F);	
	Mat oneMatrix2 = Mat::zeros(1,3,CV_64F);
		
	rotationLabel->SetData(&oneMatrix);
	translationLabel->SetData(&oneMatrix2);

	certaintyIndicator = new CertaintyIndicator(0);
	myGrid->AddChild(certaintyIndicator,Point2i(3,2));

	AddChild(myGrid);

	SetPage(0);
	addPage2(engine);

	SetVisible(true);
	LOGD(LOGTAG_INPUT,"Laying out DebugUI");

	DoLayout(Rect(0,0,engine->imageWidth,engine->imageHeight));

	
	LOGD(LOGTAG_INPUT,"DebugUI Layout complete");
}


void ARControllerDebugUI::addPage2(Engine * engine) 
{
	SetDefaults();

	int padding = 20;
	DoLayout(Rect(padding,padding,engine->imageWidth-padding*2, engine->imageHeight - padding*2));

	GridLayout * myGrid = new GridLayout(Size2i(100,100),Size2i(3,2));


	NumberSpinner * numberSpinner2 = new NumberSpinner("MinFPScore",MinFinderPatternScore,10.0f,"%3.1f");
	numberSpinner2->AddValueChangedDelegate(NumberSpinnerEventDelegate::from_method<ARControllerDebugUI,&ARControllerDebugUI::MinimumFinderPatternScoreChanged>(this));
	myGrid->AddChild(numberSpinner2,Point2i(1,0));
	numberSpinner2->SetMinimum(100.0f);
	numberSpinner2->SetMaximum(300.0f);

	NumberSpinner * minAlignmentSpinner = new NumberSpinner("MinAlignScore",MinAlignmentScore,10.0f,"%3.1f");
	minAlignmentSpinner->AddValueChangedDelegate(NumberSpinnerEventDelegate::from_method<ARControllerDebugUI,&ARControllerDebugUI::MinimumAlignmentPatternScoreChanged>(this));
	myGrid->AddChild(minAlignmentSpinner,Point2i(1,1));
	minAlignmentSpinner->SetMinimum(100.0f);
	minAlignmentSpinner->SetMaximum(300.0f);
	
	SelectBox * drawModeSelect = new SelectBox(3,Colors::MidnightBlue);
	drawModeSelect->AddItem(new SelectBoxItem("Color"));
	drawModeSelect->AddItem(new SelectBoxItem("Gray"));	
	drawModeSelect->AddItem(new SelectBoxItem("Binary"));
	drawModeSelect->AddSelectionChangedDelegate(SelectionChangedEventDelegate::from_method<ARControllerDebugUI,&ARControllerDebugUI::DrawmodeSelectionChanged>(this));
	drawModeSelect->SetSelectedIndex(1);
	myGrid->AddChild(drawModeSelect,Point2i(2,0));

	NumberSpinner * fastThresh = new NumberSpinner("FastThresh",9,1,"%3.1f");
	fastThresh->Name = "FastThresh";
	fastThresh->AddValueChangedDelegate(NumberSpinnerEventDelegate::from_method<ARControllerDebugUI,&ARControllerDebugUI::NumberSpinnerValueChanged>(this));
	fastThresh->SetMaximum(15);
	fastThresh->SetMinimum(9);
	myGrid->AddChild(fastThresh,Point2i(0,0));
	
	//engine->inputHandler->AddGlobalTouchDelegate(TouchEventDelegate::from_method<ARControllerDebugUI,&ARControllerDebugUI::GlobalTouchEvent>(this));	


	LOGD(LOGTAG_INPUT,"Adding grid to page display");
	AddChild(myGrid);

	GridLayout * nextPageGrid = new GridLayout(Size2i(100,100),Size2i(3,2));

		
	NumberSpinner * positionSpinner = new NumberSpinner("T-Alpha",PositionFilterAlpha,0.05f,"%2.2f");
	positionSpinner->AddValueChangedDelegate(NumberSpinnerEventDelegate::from_method<ARControllerDebugUI,&ARControllerDebugUI::PositionFilterAlphaChanged>(this));
	nextPageGrid->AddChild(positionSpinner,Point2i(0,0));
	positionSpinner->SetMaximum(1.0f);
	positionSpinner->SetMinimum(0.0f);

	NumberSpinner * rotationSpinner = new NumberSpinner("R-Alpha",RotationFilterAlpha,0.05f,"%2.2f");
	rotationSpinner->AddValueChangedDelegate(NumberSpinnerEventDelegate::from_method<ARControllerDebugUI,&ARControllerDebugUI::RotationFilterAlphaChanged>(this));
	nextPageGrid->AddChild(rotationSpinner,Point2i(0,1));
	rotationSpinner->SetMaximum(1.0f);
	rotationSpinner->SetMinimum(0.0f);
	
	NumberSpinner * testSpinner = new NumberSpinner("Cats",0.1f,0.05f,"%2.2f");
	nextPageGrid->AddChild(testSpinner,Point2i(0,0));

	AddChild(nextPageGrid);
}


void ARControllerDebugUI::AddNewParameter(std::string paramName, float defaultValue, float step, float minValue, float maxValue, std::string format)
{
	NumberSpinner * newSpinner = new NumberSpinner(paramName,defaultValue,step,format);
	newSpinner->Name = paramName;
	newSpinner->AddValueChangedDelegate(NumberSpinnerEventDelegate::from_method<ARControllerDebugUI,&ARControllerDebugUI::NumberSpinnerValueChanged>(this));

	//Need to add it to somewhere
	parameterMap.insert(pair<std::string,float>(paramName,defaultValue));
}


void ARControllerDebugUI::SetDefaults()
{
	PositionFilterAlpha = 0.9f;	
	RotationFilterAlpha = 0.9f;	
	MinFinderPatternScore = 190;
	MinAlignmentScore = 180;
	currentDrawMode = DrawModes::GrayImage;
	
	parameterMap.insert(pair<std::string,float>("FastThresh",9.0f));
}

void ARControllerDebugUI::NumberSpinnerValueChanged(void * sender, NumberSpinnerEventArgs args)
{
	if (sender == NULL)
		return;
	
	parameterMap[(((NumberSpinner*)sender)->Name)] = args.NewValue;
}

float ARControllerDebugUI::GetParameter(std::string paramName)
{
	if (parameterMap.find(paramName) != parameterMap.end())
	{
		return parameterMap[paramName];
	}
	else
	{
		LOGE("Parameter %s does not exit in map!",paramName.c_str());
		return MAXFLOAT;
	}
}

void ARControllerDebugUI::PositionFilterAlphaChanged(void * sender, NumberSpinnerEventArgs args)
{
	PositionFilterAlpha = args.NewValue;
}

void ARControllerDebugUI::RotationFilterAlphaChanged(void * sender, NumberSpinnerEventArgs args)
{
	RotationFilterAlpha = args.NewValue;
}


void ARControllerDebugUI::MinimumFinderPatternScoreChanged(void * sender, NumberSpinnerEventArgs args)
{
	MinFinderPatternScore = args.NewValue;
}

void ARControllerDebugUI::MinimumAlignmentPatternScoreChanged(void * sender, NumberSpinnerEventArgs args)
{
	MinAlignmentScore = args.NewValue;
}


void ARControllerDebugUI::ToggleVisibility(void * sender, EventArgs args)
{
	isVisible = !isVisible;
}

void ARControllerDebugUI::DrawmodeSelectionChanged(void * sender, SelectionChangedEventArgs args)
{
	if (args.NewSelection != NULL)
	{
		std::string label = ((SelectBoxItem*)args.NewSelection)->label;
		LOGD(LOGTAG_INPUT,"Processing selection changed, new label = %s",label.c_str());
		if (label.compare("Color") == 0)
			currentDrawMode = DrawModes::ColorImage;
		else if(label.compare("Gray") == 0)
			currentDrawMode = DrawModes::GrayImage;
		else if (label.compare("Binary") == 0)
			currentDrawMode = DrawModes::BinaryImage;
	}
}

void ARControllerDebugUI::SetStateDisplay(string stateDescription)
{
	stateLabel->SetText(stateDescription);
}

void ARControllerDebugUI::SetRotation(Mat * mat)
{
	rotationLabel->SetData(mat);
}
void ARControllerDebugUI::SetTranslation(Mat * mat)
{
	translationLabel->SetData(mat);
}

void ARControllerDebugUI::SetPositionCertainty(float certainty)
{
	certaintyIndicator->SetCertainty(certainty);
}