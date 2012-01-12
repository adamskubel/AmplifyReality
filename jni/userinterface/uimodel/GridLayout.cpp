#include "userinterface/uimodel/GridLayout.hpp"


GridLayout::GridLayout(Engine * engine, Size_<int> _gridSize)
{
	cellSize = Size_<int>((int)((float)engine->imageWidth/_gridSize.width),(int)((float)engine->imageHeight/_gridSize.height));
	gridSize = _gridSize;

	LOGD(LOGTAG_INPUT,"Created Grid with cell size[%d,%d]",cellSize.width,cellSize.height);

	childUIElements = vector<UIElement*>();
	childUpdateElements = vector<Updateable*>();
}

GridLayout::~GridLayout()
{
	while (!childUpdateElements.empty())
	{
		delete childUpdateElements.back();
		childUpdateElements.pop_back();
	}

	while(!childUIElements.empty())
	{
		delete childUIElements.back();
		childUIElements.pop_back();
	}
}

bool GridLayout::CheckGridFit(Point2i gridPoint, Size_<int> gridSpan)
{
	if ((gridPoint.x + gridSpan.width) > gridSize.width || (gridPoint.x) < 0
		|| (gridPoint.y+gridSpan.height) > gridSize.height || (gridPoint.y) < 0)
	{
		LOGE("GridLayout: Point is outside of grid! Grid[%d,%d] Point[%d,%d] Span[%d,%d]",gridSize.width,gridSize.height,gridPoint.x,gridPoint.y,gridSpan.width,gridSpan.height);
		throw Exception(0,"Requested point exceeds grid size","AddChild","GridLayout.cpp",0);
	}
}

void GridLayout::AddChild(Button * child, Point2i gridPoint, Size_<int> gridSpan)
{
	CheckGridFit(gridPoint,gridSpan);

	Point2i newPoint = Point2i(gridPoint.x * cellSize.width,gridPoint.y * cellSize.height);

	child->buttonBoundaries.x = newPoint.x;
	child->buttonBoundaries.y = newPoint.y;
	child->buttonBoundaries.width = cellSize.width * gridSpan.width;
	child->buttonBoundaries.height = cellSize.height * gridSpan.height;

	LOGD(LOGTAG_INPUT,"Adding button to grid. X=%d,Y=%d,W=%d,H=%d",child->buttonBoundaries.x,child->buttonBoundaries.y,
		child->buttonBoundaries.width,child->buttonBoundaries.height);
	
	childUIElements.push_back(child);
	childUpdateElements.push_back(child);

}

void GridLayout::AddChild(Label * child, Point2i gridPoint, Size_<int> gridSpan)
{
	CheckGridFit(gridPoint,gridSpan);

	Point2i newPoint = Point2i(gridPoint.x * cellSize.width,gridPoint.y * cellSize.height);

	child->Center.x = newPoint.x;
	child->Center.y = newPoint.y;

	childUIElements.push_back(child);
	childUpdateElements.push_back(child);
}

void GridLayout::Update(FrameItem * item)
{
	for (int i=0;i<childUpdateElements.size();i++)
	{
		childUpdateElements.at(i)->Update(item);
	}
}

UIElement *  GridLayout::GetChildAt(Point2i point)
{
	for (int i=0;i<childUIElements.size();i++)
	{
		UIElement * element = childUIElements.at(i)->GetChildAt(point);
		if (element != NULL)
			return element;
	}
	return NULL;
}