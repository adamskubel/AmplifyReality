#include "userinterface/uimodel/GridLayout.hpp"


GridLayout::GridLayout()
{
	cellSize = Size2i(10,10);
	gridSize = Size2i(1,1);
	Position = Point2i(0,0);
}


GridLayout::GridLayout(Size2i windowSize, Size_<int> _gridSize, Point2i position)
{
	cellSize = Size_<int>((int)((float)windowSize.width/_gridSize.width),(int)((float)windowSize.height/_gridSize.height));
	gridSize = _gridSize;
	Position = position;

	LOGD(LOGTAG_INPUT,"Created Grid with cell size[%d,%d]",cellSize.width,cellSize.height);
}

GridLayout::~GridLayout()
{
	LOGD(LOGTAG_INPUT,"Deleting GridLayout");

	Children.clear();

	LOGD(LOGTAG_INPUT,"Gridlayout deleted successfully.");
}

void GridLayout::ResizeGrid(Size2i windowSize, Size2i _gridSize, Point2i position)
{
	cellSize = Size_<int>((int)((float)windowSize.width/_gridSize.width),(int)((float)windowSize.height/_gridSize.height));
	gridSize = _gridSize;
	Position = position;

	//Layout children
	for (int i=0;i<Children.size();i++)
	{
		GraphicalUIElement * gridChild = Children.at(i);
		gridChild->DoLayout(GetRectangleFromGridData(gridChild->gridPoint,gridChild->gridSpan));
	}
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
	Size2i windowSize = Size2i(boundaries.width, boundaries.height);
	Position = Point2i(boundaries.x,boundaries.y);
	
	ResizeGrid(windowSize,gridSize,Position);
}

void GridLayout::Draw(Mat * rgbaImage)
{
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