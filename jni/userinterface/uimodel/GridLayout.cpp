#include "userinterface/uimodel/GridLayout.hpp"


GridLayout::GridLayout(Engine * engine, Size_<int> _gridSize)
{
	cellSize = Size_<int>((int)((float)engine->imageWidth/_gridSize.width),(int)((float)engine->imageHeight/_gridSize.height));
	gridSize = _gridSize;

	LOGD(LOGTAG_INPUT,"Created Grid with cell size[%d,%d]",cellSize.width,cellSize.height);

	childUIElements = vector<UIElement*>();
	childDrawElements = vector<Drawable*>();
}

GridLayout::~GridLayout()
{
	LOGD(LOGTAG_INPUT,"Deleting GridLayout");
	while (!childDrawElements.empty())
	{
		delete childDrawElements.back();
		childDrawElements.pop_back();
	}

	while(!childUIElements.empty())
	{
		childUIElements.pop_back(); //Don't delete again here since UI elements were already deleted above via update vector
	}
	LOGD(LOGTAG_INPUT,"Gridlayout deleted successfully.");
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

void GridLayout::AddChild(CertaintyIndicator * child, Point2i gridPoint, Size_<int> gridSpan)
{
	CheckGridFit(gridPoint,gridSpan);

	Point2i newPoint = Point2i(gridPoint.x * cellSize.width,gridPoint.y * cellSize.height); //top-left corner
	float maxRadius = 0.5f * std::min(cellSize.width, cellSize.height);
	newPoint = newPoint - Point2i(maxRadius,maxRadius);


	child->CenterPoint = newPoint;
	child->SetMaxRadius(maxRadius);

	LOGD(LOGTAG_INPUT,"Adding certainty indicator to grid. Center=(%d,%d), Radius=(%f)",child->CenterPoint.x, child->CenterPoint.y, maxRadius);
	
	childUIElements.push_back(child);
	childDrawElements.push_back(child);

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
	childDrawElements.push_back(child);

}

void GridLayout::AddChild(Label * child, Point2i gridPoint, Size_<int> gridSpan)
{
	CheckGridFit(gridPoint,gridSpan);

	Point2i newPoint = Point2i(gridPoint.x * cellSize.width,gridPoint.y * cellSize.height);

	child->Center.x = newPoint.x;
	child->Center.y = newPoint.y;

	childUIElements.push_back(child);
	childDrawElements.push_back(child);
}

void GridLayout::Draw(Mat * rgbaImage)
{
	for (int i=0;i<childDrawElements.size();i++)
	{
		childDrawElements.at(i)->Draw(rgbaImage);
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