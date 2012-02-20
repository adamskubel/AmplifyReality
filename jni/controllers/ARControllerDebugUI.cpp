#include "ARControllerDebugUI.hpp"

ARControllerDebugUI::ARControllerDebugUI(Engine * engine, Point2i position) : PageDisplay()
{	
	int padding = 20;
	GridDimensions = Size2i(3,2);
	
	GridLayout * firstPage = new GridLayout(Size2i(4,3));
	
	AddChild(firstPage);
	
	
	fpsLabel = new Label("[FPS]",Point2i(0,0),Colors::MidnightBlue,Colors::White);
	firstPage->AddChild(fpsLabel,Point2i(0,0));

	GridLayout * myGrid = new GridLayout(Size2i(3,6));	

	//rotationLabel = new DataDisplay("%3.2lf",Colors::Red);	
	//translationLabel = new DataDisplay("%3.2lf",Colors::Blue);
	//myGrid->AddChild(rotationLabel,Point2i(0,2));
	//myGrid->AddChild(translationLabel,Point2i(0,3));


	stateLabel = new Label("[State]",Point2i(0,0),Colors::Blue,Colors::White);
	myGrid->AddChild(stateLabel,Point2i(0,0));
	AddChild(myGrid);
		

	//certaintyIndicator = new CertaintyIndicator(0);
	//myGrid->AddChild(certaintyIndicator,Point2i(3,2));
	
	

	/*AddNewParameter("MinFPScore",190,10.0f,100,300,"%3.1f",3);
	AddNewParameter("MinAlignScore",180,10.0f,100,300,"%3.1f",3);*/
		
	//Need to automate this..maybe
	SelectBox * drawModeSelect = new SelectBox(3,Colors::MidnightBlue);
	drawModeSelect->AddItem(new SelectBoxItem("Color"));
	drawModeSelect->AddItem(new SelectBoxItem("Gray"));	
	drawModeSelect->AddItem(new SelectBoxItem("Binary"));
	drawModeSelect->AddSelectionChangedDelegate(SelectionChangedEventDelegate::from_method<ARControllerDebugUI,&ARControllerDebugUI::DrawmodeSelectionChanged>(this));
	drawModeSelect->SetSelectedIndex(1);
	//AddInNextPosition(drawModeSelect,3);

	currentDrawMode = DrawModes::GrayImage;
		
	//AddNewParameter("T-Alpha",0.9f,0.05f,0.0f,1.0f,"%2.2f",3);
	//AddNewParameter("R-Alpha",0.9f,0.05f,0.0f,1.0f,"%2.2f",3);
	

	SetPage(0);
	SetVisible(true);
	LOGD(LOGTAG_INPUT,"(DebugUI) Starting layout");
	DoLayout(Rect(0,0,engine->imageWidth,engine->imageHeight));
}

void ARControllerDebugUI::SetLabelValue(std::string labelName, float labelValue)
{
	map<std::string,pair<Label*,std::string> >::iterator labelIterator = labelMap.find(labelName);

	if (labelIterator != labelMap.end())
	{		
		std::stringstream textStream;
		textStream << labelName;
		textStream << "=";
		textStream.precision(3);
		textStream.setf(ios_base::fixed);
		textStream << labelValue;
		textStream << (*labelIterator).second.second;
		(*labelIterator).second.first->SetText(textStream.str());
	}
	else
	{
		LOGW(LOGTAG_INPUT,"Attempted to set value of non-existent label: %s",labelName.c_str());
	}
}

void ARControllerDebugUI::AddNewLabel(std::string labelName, std::string format, int desiredPage)
{
	Label * newLabel = new Label(format,Point2i(0,0),Colors::Black,Colors::White);

	labelMap.insert(pair<std::string,pair<Label*,std::string> >(labelName,pair<Label*,std::string>(newLabel,format)));
	AddInNextPosition(newLabel,desiredPage);
}

void ARControllerDebugUI::AddNewParameter(std::string paramName,  float defaultValue, float step, float minValue, float maxValue, std::string format, int desiredPage)
{
	AddNewParameter(paramName,paramName,defaultValue,step,minValue,maxValue,format,desiredPage);
}

void ARControllerDebugUI::AddNewParameter(std::string paramName, std::string paramKey, float defaultValue, float step, float minValue, float maxValue, std::string format, int desiredPage)
{
	if (parameterMap.find(paramKey) != parameterMap.end())	
	{
		LOGI(LOGTAG_INPUT,"Parameter already exists, deleting");
		//Clean up last spinner
		for (int i=0;i<Children.size();i++)
		{
			UIElement * element = ((GridLayout*)Children.at(i))->GetElementByName(paramKey);
			if (element != NULL)
			{
				delete element;
				LOGI(LOGTAG_INPUT,"Parameter deleted successfully.");
				break;
			}
		}
	}

	NumberSpinner * newSpinner = new NumberSpinner(paramName,defaultValue,step,format);
	newSpinner->Name = paramKey;
	newSpinner->SetMinimum(minValue);
	newSpinner->SetMaximum(maxValue);
	newSpinner->AddValueChangedDelegate(NumberSpinnerEventDelegate::from_method<ARControllerDebugUI,&ARControllerDebugUI::NumberSpinnerValueChanged>(this));

	parameterMap.insert(pair<std::string,float>(paramKey,defaultValue));
	AddInNextPosition(newSpinner,desiredPage);
}

void ARControllerDebugUI::AddInNextPosition(GraphicalUIElement * newControl, int desiredPage)
{
	Point2i position;
	int page = -1;
	FindNextPosition(position,page,desiredPage);
	GridLayout * grid = (GridLayout*)Children.at(page);
	LOGI(LOGTAG_INPUT,"Adding new control to cell (%d,%d) on page %d",position.x,position.y,page);
	grid->AddChild(newControl,position);
}

void ARControllerDebugUI::FindNextPosition(Point2i & position, int & pageNum, int desiredPage, Size2i controlSize)
{
	if (desiredPage >= Children.size())
	{
		LOGW(LOGTAG_INPUT, "Requested page %d is out of range, only %d pages present",desiredPage,Children.size());
		desiredPage = Children.size();
	}

	for (int page = desiredPage; page < Children.size(); page++)
	{
		LOGI(LOGTAG_INPUT,"Looking for empty positions on page %d",page);
		GridLayout * grid = (GridLayout*) Children[page];

		for (int x = 0; x < grid->GetWidth(); x++)
		{
			for (int y= 0;y<grid->GetHeight();y++)
			{
				if (grid->GetElementAtCell(Point2i(x,y)) == NULL)
				{
					position = Point2i(x,y);
					pageNum = page;
					return;
				}
			}
		}
	}

	//If we haven't returned yet, then all possible positions are full. Add a new page.
	position = Point2i(0,0);

	GridLayout * newGrid = new GridLayout(GridDimensions);	
	AddChild(newGrid);
	pageNum = Children.size()-1;
	return;
}



void ARControllerDebugUI::NumberSpinnerValueChanged(void * sender, NumberSpinnerEventArgs args)
{
	if (sender == NULL)
		return;
	
	parameterMap[(((NumberSpinner*)sender)->Name)] = args.NewValue;
}

float ARControllerDebugUI::GetParameter(std::string paramKey)
{
	map<string,float>::iterator paramMapIterator = parameterMap.find(paramKey);
	if (paramMapIterator != parameterMap.end())
	{
		return (*paramMapIterator).second;
	}
	else
	{
		LOGE("Parameter '%s' does not exit in map!",paramKey.c_str());
		return MAXFLOAT;
	}
}

float ARControllerDebugUI::GetFloatParameter(std::string paramKey)
{
	return GetParameter(paramKey);
}

bool ARControllerDebugUI::GetBooleanParameter(std::string paramKey)
{
	map<string,float>::iterator paramMapIterator = parameterMap.find(paramKey);
	if (paramMapIterator != parameterMap.end())
	{
		return ((*paramMapIterator).second == 1.0f);
	}
	else
	{
		LOGW("Boolean parameter '%s' does not exit in map!",paramKey.c_str());
		return false;
	}
}

int ARControllerDebugUI::GetIntegerParameter(std::string paramKey)
{
	map<string,float>::iterator paramMapIterator = parameterMap.find(paramKey);
	if (paramMapIterator != parameterMap.end())
	{
		return (int)(*paramMapIterator).second;
	}
	else
	{
		LOGE("Parameter '%s' does not exit in map!",paramKey.c_str());
		return 0;
	}
}

void ARControllerDebugUI::ToggleVisibility(void * sender, EventArgs args)
{
	isVisible = !isVisible;
}

void ARControllerDebugUI::DrawmodeSelectionChanged(void * sender, SelectionChangedEventArgs args)
{
	if (args.NewSelection != NULL)
	{
		std::string label = ((SelectBoxItem*)args.NewSelection)->Name;
		LOGD(LOGTAG_INPUT,"Processing selection changed, new label = %s",label.c_str());
		if (label.compare("Color") == 0)
			currentDrawMode = DrawModes::ColorImage;
		else if(label.compare("Gray") == 0)
			currentDrawMode = DrawModes::GrayImage;
		else if (label.compare("Binary") == 0)
			currentDrawMode = DrawModes::BinaryImage;
	}
}

void ARControllerDebugUI::SetFPS(float fps)
{
	char fpsString[100];
	sprintf(fpsString,"FPS=%3.1f",fps);
	fpsLabel->SetText(fpsString);
}


void ARControllerDebugUI::SetStateDisplay(string stateDescription)
{
	stateLabel->SetText(stateDescription);
}

void ARControllerDebugUI::SetRotation(Mat * mat)
{
	//rotationLabel->SetData(mat);
}
void ARControllerDebugUI::SetTranslation(Mat * mat)
{
	//translationLabel->SetData(mat);
}

void ARControllerDebugUI::SetPositionCertainty(float certainty)
{
	//certaintyIndicator->SetCertainty(certainty);
}