#include "TabDisplay.hpp"


TabDisplay::TabDisplay(bool _collapseMode)
{
	collapseEnabled = _collapseMode;
	currentTab = -1;
	isCollapsed = false;
	collapseButton = NULL;
}


TabDisplay::~TabDisplay()
{
	if (collapseButton != NULL)
	{
		delete collapseButton;
	}

	while (!TabChildren.empty())
	{
		delete TabChildren.back();
		TabChildren.pop_back();
	}
}

void TabDisplay::AddTab(std::string tabName, GraphicalUIElement * tabContent)
{	
	TabPage * newTab = new TabPage();

	newTab->TabName = tabName;
	newTab->TabContent = tabContent;
	Button * tabButton = new Button(newTab->TabName,UI_BUTTON_COLOR,Colors::Black);
	tabButton->Alpha = 0.7f;
	tabButton->Name = tabName;
	tabButton->AddClickDelegate(ClickEventDelegate::from_method<TabDisplay,&TabDisplay::TabButtonPressed>(this));
	newTab->TabButton = tabButton;
	TabChildren.push_back(newTab);

	if (lastBoundaryRectangle.width != 0)
	{
		LayoutTabButtons(lastBoundaryRectangle,UI_BUTTON_SIZE);
		tabContent->DoLayout(contentRect);
	}
}

void TabDisplay::SetCollapseMode(bool _collapseMode)
{
	collapseEnabled = _collapseMode;
}

void TabDisplay::LayoutTabButtons(Rect boundaryRectangle, Size2i buttonSize)
{
	int numTabs = (collapseEnabled) ? TabChildren.size() + 1 : TabChildren.size();
	int buttonWidth = (int)round((float)boundaryRectangle.width / (float)numTabs);

	buttonWidth = MIN(buttonSize.width,buttonWidth);
	LOGD(LOGTAG_INPUT,"Setting tab button size to [%d,%d]",buttonWidth,buttonSize.height);
	
	
	if (collapseEnabled)
	{		
		Rect tabButtonRect = Rect(boundaryRectangle.x,boundaryRectangle.y,buttonWidth,buttonSize.height);
		if (collapseButton == NULL)
		{
			isCollapsed = false;
			collapseButton = new Button("<<<",UI_SPECIAL_BUTTON_COLOR);
			collapseButton->AddClickDelegate(ClickEventDelegate::from_method<TabDisplay,&TabDisplay::CollapsePressed>(this));
			LOGI(LOGTAG_INPUT,"Created collapse button");
		}
		collapseButton->DoLayout(tabButtonRect);
	}
	for (int i=0;i< TabChildren.size();i++)
	{
		int j = (collapseEnabled) ? i+1 : i;
		Rect tabButtonRect = Rect(boundaryRectangle.x + (j*buttonWidth),boundaryRectangle.y,buttonWidth,buttonSize.height);
		TabChildren[i]->TabButton->DoLayout(tabButtonRect);
	}
}

void TabDisplay::DoLayout(Rect boundaryRectangle)
{
	const Size2i DefaultButtonSize = UI_BUTTON_SIZE;
	Size2i buttonSize = DefaultButtonSize;
	lastBoundaryRectangle = boundaryRectangle;
	contentRect = Rect(boundaryRectangle.x,boundaryRectangle.y+buttonSize.height,boundaryRectangle.width,boundaryRectangle.height-buttonSize.height);	

	LayoutTabButtons(boundaryRectangle,buttonSize);

	for (int i=0;i<TabChildren.size();i++)
	{
		TabChildren.at(i)->TabContent->DoLayout(contentRect);
	}
	layoutDefined = true;
}

UIElement * TabDisplay::GetElementAt(Point2i point)
{
	if (!isVisible || currentTab < 0)
		return NULL;
	
	UIElement * element;
	if (collapseEnabled && collapseButton != NULL)
	{
		element = collapseButton->GetElementAt(point);
		if (element != NULL)
			return element;
	}

	if (!collapseEnabled || !isCollapsed)
	{
		//Check if point is in one of tab header buttons
		for (int i=0;i<TabChildren.size();i++)
		{
			element = TabChildren[i]->TabButton->GetElementAt(point);
			if (element != NULL)
			{
				LOGV(LOGTAG_INPUT,"Point(%d,%d) is within tab header #%d",point.x,point.y,i);
				return element;
			}
		}
		//If button is not pressed, then check current tab 
		return TabChildren.at(currentTab)->TabContent->GetElementAt(point);
	}
	return NULL;
}


void TabDisplay::TabButtonPressed(void * sender, EventArgs args)
{
	Button * button = (Button*)sender;
	std::string name = button->Name;
	if (currentTab >= 0)
	{
		TabChildren.at(currentTab)->TabButton->SetFillColor(UI_BUTTON_COLOR);
	}
	SetTab(name);	
	button->SetFillColor(UI_BUTTON_ALT_STATE_COLOR);
}

void TabDisplay::CollapsePressed(void * sender, EventArgs args)
{
	Button * button = (Button*)sender;
	
	//Toggle collapsed state
	isCollapsed = !isCollapsed;

	//Set button appearance
	if (isCollapsed)
	{
		button->SetFillColor(UI_BUTTON_COLOR);
		button->SetText(">>>");
	}
	else
	{
		button->SetFillColor(UI_SPECIAL_BUTTON_COLOR);
		button->SetText("<<<");
	}
}

void TabDisplay::SetTab(std::string tabName)
{
	for (int i=0;i<TabChildren.size();i++)
	{
		if (TabChildren.at(i)->TabName.compare(tabName) == 0)
		{
			SetTab(i);
			return;
		}
	}
}

GraphicalUIElement * TabDisplay::GetTabByName(std::string name)
{
	for (int i=0;i<TabChildren.size();i++)
	{
		if (TabChildren.at(i)->TabName.compare(name) == 0)
		{
			return TabChildren.at(i)->TabContent;
		}
	}
	return NULL;
}

void TabDisplay::SetTab(int tabIndex)
{
	if (tabIndex < TabChildren.size())
	{
		if (TabChildren.at(tabIndex)->TabButton != NULL)
			TabChildren.at(tabIndex)->TabButton->SetFillColor(UI_BUTTON_ALT_STATE_COLOR);
		currentTab = tabIndex;
	}
	else		
		LOGW(LOGTAG_INPUT,"Cannot set tab number to %d, only %d children",tabIndex,TabChildren.size());
}

void TabDisplay::Draw(Mat * rgbaImage)
{
	if (!IsVisible() || !layoutDefined)
		return;

	if (collapseEnabled && collapseButton != NULL)
	{
		collapseButton->Draw(rgbaImage);
	}

	if (!collapseEnabled || !isCollapsed)
	{
		for (int i=0;i<TabChildren.size();i++)
		{
			TabChildren[i]->TabButton->Draw(rgbaImage);
		}

		if (currentTab >= 0)
		{
			if (TabChildren.at(currentTab)->TabContent->IsVisible())
				TabChildren.at(currentTab)->TabContent->Draw(rgbaImage);
		}
	}
}


