#ifndef PAGE_DISPLAY_HPP_
#define PAGE_DISPLAY_HPP_

#include "UIElement.hpp"
#include "GridLayout.hpp"
#include "Button.hpp"

class PageDisplay : public GraphicalUIElement
{
public:
	PageDisplay();
	~PageDisplay();
	void AddChild(GraphicalUIElement * child);
	void Draw(Mat * rgbaImage);
	void DoGridLayout(Point2i offset, Size2i cellSize, Point2i gridPoint, Size2i gridSpan);
	void DoLayout(Rect boundaryRectangle);
	void SetPage(int pageNumber);
	UIElement * GetElementAt(cv::Point2i p);

	void PreviousPage(void * sender, EventArgs args);
	void NextPage(void * sender, EventArgs args);


private:
	static const float DefaultButtonSize = 150;
	vector<GraphicalUIElement*> Children;
	int currentPage;
	Button * nextPage, * previousPage;

	Rect contentRect;
	
	void AdjustButtons();

};


#endif