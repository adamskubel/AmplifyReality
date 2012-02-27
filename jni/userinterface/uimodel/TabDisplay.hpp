#ifndef TAB_DISPLAY_HPP_
#define TAB_DISPLAY_HPP_


#include "UIElement.hpp"
#include "GridLayout.hpp"
#include "Button.hpp"

class TabPage
{
public:
	std::string TabName;
	Button * TabButton;
	GraphicalUIElement * TabContent;
	Rect TabJoinRect;
};

class TabDisplay  : public GraphicalUIElement
{
public:
	TabDisplay(bool collapseEnabled = false, Size2i buttonSize = UI_BUTTON_SIZE);
	~TabDisplay();
	void AddChild(GraphicalUIElement * child);
	void Draw(Mat * rgbaImage);
	void DoGridLayout(Point2i offset, Size2i cellSize, Point2i gridPoint, Size2i gridSpan);
	void DoLayout(Rect boundaryRectangle);
	void SetTab(int tabIndex);
	void SetTab(std::string tabName);
	UIElement * GetElementAt(cv::Point2i p);

	void SetCollapseMode(bool enabled);

	void TabButtonPressed(void * sender, EventArgs args);
	void CollapsePressed(void * sender, EventArgs args);

	void AddTab(std::string tabName, GraphicalUIElement * tabContent);
	GraphicalUIElement * GetTabByName(string tabName);

	void SetButtonSize(Size2i size);

private:
	int currentTab;
	Rect contentRect, lastBoundaryRectangle;
	void LayoutTabButtons(Rect boundaryRectangle, Size2i buttonSize);
	bool collapseEnabled, isCollapsed;
	Button * collapseButton;
	Rect tabLineSeperatorRectangle, tabLineJoiningRectangle;
	Size2i DefaultButtonSize;
protected:	
	vector<TabPage*> TabChildren;
};

#endif