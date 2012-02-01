#include "userinterface/uimodel/GridLayout.hpp"


GridLayout::GridLayout(Size2i _gridSize)
{
	cellSize = Size2i(10,10);
	controlSize = Size2i(100,100);
	gridSize = _gridSize;
	Position = Point2i(0,0);
}


GridLayout::GridLayout(Size2i _controlSize, Size_<int> _gridSize, Point2i _position) : GraphicalUIElement(true)
{	
	ResizeGrid(_controlSize,_gridSize,_position);


}

GridLayout::~GridLayout()
{
	LOGD(LOGTAG_INPUT,"Deleting GridLayout");

	Children.clear();

	LOGD(LOGTAG_INPUT,"Gridlayout deleted successfully.");
}

void GridLayout::ResizeGrid(Size2i _controlSize, Size2i _gridSize, Point2i _position)
{
	controlSize = _controlSize;
	gridSize = _gridSize;
	Position = _position;	

	cellSize = Size_<int>((int)((float)controlSize.width/gridSize.width),(int)((float)controlSize.height/gridSize.height));
	
	LOGD(LOGTAG_INPUT,"GridLayout: New Cellsize is [%d,%d]",cellSize.width,cellSize.height);

	//Layout children
	for (int i=0;i<Children.size();i++)
	{
		GraphicalUIElement * gridChild = Children.at(i);
		gridChild->DoLayout(GetRectangleFromGridData(gridChild->gridPoint,gridChild->gridSpan));
	}

	layoutDefined = true;
}

Rect GridLayout::GetRectangleFromGridData(Point2i _gridPoint, Size2i _gridSpan)
{
	Point2i endPoint = Point2i(cellSize.width*_gridSpan.width, cellSize.height*_gridSpan.height);
	//endPoint += Position;

	Point2i startPoint = Point2i(cellSize.width *_gridPoint.x, cellSize.height* _gridPoint.y);
	startPoint += Position;
	endPoint += startPoint;

	return Rect(startPoint,endPoint);
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

void GridLayout::AddChild(GraphicalUIElement * child, Point2i gridPoint, Size_<int> gridSpan)
{
	CheckGridFit(gridPoint,gridSpan);

	child->gridPoint = gridPoint;
	child->gridSpan = gridSpan;
	if (layoutDefined)
		child->DoLayout(GetRectangleFromGridData(gridPoint,gridSpan));	
	Children.push_back(child);
}

UIElement * GridLayout::GetElementAt(Point2i point)
{
	if (!IsVisible())
		return NULL;

	for (int i=0;i<Children.size();i++)
	{
		if (Children.at(i)->IsVisible())
		{
			UIElement * child = Children.at(i)->GetElementAt(point);
			if (child != NULL)
			{
				return child;
			}
		}
	}
	return NULL;
}

UIElement * GridLayout::GetElementByName(std::string name)
{
	for (int i=0;i<Children.size();i++)
	{
		if (!Children.at(i)->Name.empty() && name.compare(Children.at(i)->Name) == 0)
			return Children.at(i);
	}
	return NULL;
}

void GridLayout::DoLayout(Rect boundaries)
{
	LOGD(LOGTAG_INPUT,"Laying out Grid, Rect=[%d,%d,%d,%d]",boundaries.x,boundaries.y,boundaries.width,boundaries.height);
	
	Size2i windowSize = Size2i(boundaries.width, boundaries.height);
	Position = Point2i(boundaries.x,boundaries.y);
	
	ResizeGrid(windowSize,gridSize,Position);
}

void GridLayout::Draw(Mat * rgbaImage)
{
	if (!layoutDefined || !IsVisible())
		return;

	for (int i=0;i<Children.size();i++)
	{
		if (Children.at(i)->IsVisible())
			Children.at(i)->Draw(rgbaImage);
	}
}

GraphicalUIElement * GridLayout::GetElementAtCell(Point2i gridPoint)
{
	for (int i=0;i<Children.size();i++)
	{
		if (Children.at(i)->gridPoint == gridPoint)
			return Children.at(i);
	}
	return NULL;
}