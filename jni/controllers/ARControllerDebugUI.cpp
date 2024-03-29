#include "ARControllerDebugUI.hpp"

ARControllerDebugUI::ARControllerDebugUI(Engine * engine, Point2i position) : TabDisplay(true)
{	
	int padding = 20;
	GridDimensions = Size2i(3,2);
	
	GridLayout * myGrid = new GridLayout(Size2i(3,6));		
	PageDisplay * page0 = new PageDisplay();
	page0->AddChild(myGrid);
	AddTab("Data",page0); 

	rotationLabel = new DataDisplay("%4.3f",Colors::Aqua,Colors::Blue);		
	translationLabel = new DataDisplay("%4.1f",Colors::Aqua,Colors::Blue);		
	
	myGrid->AddChild(translationLabel,Point2i(1,0),Size2i(1,1));
	myGrid->AddChild(rotationLabel,Point2i(1,2),Size2i(1,1));
		
	//Need to automate this..maybe
	SelectBox * drawModeSelect = new SelectBox(3,Colors::MidnightBlue);
	drawModeSelect->AddItem(new SelectBoxItem("Color"));
	drawModeSelect->AddItem(new SelectBoxItem("Gray"));	
	drawModeSelect->AddItem(new SelectBoxItem("Binary"));
	drawModeSelect->AddSelectionChangedDelegate(SelectionChangedEventDelegate::from_method<ARControllerDebugUI,&ARControllerDebugUI::DrawmodeSelectionChanged>(this));
	drawModeSelect->SetSelectedIndex(1);
	AddInNextPosition(drawModeSelect,"Tracking");
	currentDrawMode = DrawModes::GrayImage;	

	SetTab(0);
	SetVisible(true);
	LOGD(LOGTAG_INPUT,"(DebugUI) Starting layout");
	DoLayout(Rect(0,0,engine->imageWidth,engine->imageHeight));
}

void ARControllerDebugUI::SetLabelValue(std::string labelName, std::string labelText)
{
	map<std::string,pair<Label*,std::string> >::iterator labelIterator = labelMap.find(labelName);

	if (labelIterator != labelMap.end())
	{	
		(*labelIterator).second.first->SetText(labelText);
	}
	else
	{
		LOGW(LOGTAG_INPUT,"Attempted to set value of non-existent label: %s",labelName.c_str());
	}
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

void ARControllerDebugUI::AddNewLabel(std::string labelName, std::string format, string category)
{
	Label * newLabel = new Label(format,Point2i(0,0),Colors::Black,Colors::White);

	labelMap.insert(pair<std::string,pair<Label*,std::string> >(labelName,pair<Label*,std::string>(newLabel,format)));
	AddInNextPosition(newLabel,category);
}

void ARControllerDebugUI::AddNewParameter(std::string paramName,  float defaultValue, float step, float minValue, float maxValue, std::string format, string category, bool mapOnly)
{
	AddNewParameter(paramName,paramName,defaultValue,step,minValue,maxValue,format,category,mapOnly);
}

void ARControllerDebugUI::AddNewParameter(std::string paramName, std::string paramKey, float defaultValue, float step, float minValue, float maxValue, std::string format, string category, bool mapOnly)
{
	if (mapOnly)
	{
		parameterMap.insert(pair<std::string,float>(paramKey,defaultValue));
		return;
	}

	if (parameterMap.find(paramKey) != parameterMap.end())	
	{
		LOGI(LOGTAG_INPUT,"Parameter already exists, deleting");
		//Clean up last spinner
		for (int i=0;i<TabChildren.size();i++)
		{
			PageDisplay * pageObject = (PageDisplay*)TabChildren.at(i)->TabContent;
			for (int j=0;j<pageObject->Children.size();j++)
			{
				UIElement * element = ((GridLayout*)pageObject->Children.at(i))->GetElementByName(paramKey);
				if (element != NULL)
				{
					delete element;
					LOGI(LOGTAG_INPUT,"Parameter deleted successfully.");
					break;
				}
			}
		}
	}

	NumberSpinner * newSpinner = new NumberSpinner(paramName,defaultValue,step,format);
	newSpinner->Name = paramKey;
	newSpinner->SetMinimum(minValue);
	newSpinner->SetMaximum(maxValue);
	newSpinner->AddValueChangedDelegate(NumberSpinnerEventDelegate::from_method<ARControllerDebugUI,&ARControllerDebugUI::NumberSpinnerValueChanged>(this));

	parameterMap.insert(pair<std::string,float>(paramKey,defaultValue));
	AddInNextPosition(newSpinner,category);
}

void ARControllerDebugUI::AddInNextPosition(GraphicalUIElement * newControl, string category, int desiredPage)
{
	PageDisplay * pageObject = (PageDisplay*)GetTabByName(category);
	if (pageObject == NULL)
	{
		pageObject = new PageDisplay();
		AddTab(category, pageObject);
	}
	Point2i position;
	int page = -1;
	FindNextPosition(pageObject, position,page,desiredPage);
	GridLayout * grid = (GridLayout*)pageObject->Children.at(page);
	LOGI(LOGTAG_INPUT,"Adding new control to cell (%d,%d) on page %d",position.x,position.y,page);
	grid->AddChild(newControl,position);
}

void ARControllerDebugUI::FindNextPosition(PageDisplay * pageObject, Point2i & position, int & pageNum, int desiredPage, Size2i controlSize)
{
	if (desiredPage >= pageObject->Children.size())
	{
		LOGW(LOGTAG_INPUT, "Requested page %d is out of range, only %d pages present",desiredPage,pageObject->Children.size());
		desiredPage = pageObject->Children.size();
	}

	for (int page = desiredPage; page < pageObject->Children.size(); page++)
	{
		LOGI(LOGTAG_INPUT,"Looking for empty positions on page %d",page);
		GridLayout * grid = (GridLayout*) pageObject->Children[page];

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
	pageObject->AddChild(newGrid);
	pageNum = pageObject->Children.size()-1;
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
		string selectionName = ((SelectBoxItem*)args.NewSelection)->Name;
		LOGD(LOGTAG_INPUT,"Processing selection changed, name = %s",selectionName.c_str());
		if (selectionName.compare("Color") == 0)
			currentDrawMode = DrawModes::ColorImage;
		else if(selectionName.compare("Gray") == 0)
			currentDrawMode = DrawModes::GrayImage;
		else if(selectionName.compare("Binary") == 0)
			currentDrawMode = DrawModes::BinaryImage;
	}
}

void ARControllerDebugUI::SetRotation(Mat * mat)
{
	rotationLabel->SetData(mat);
}
void ARControllerDebugUI::SetTranslation(Mat * mat)
{
	translationLabel->SetData(mat);
}
