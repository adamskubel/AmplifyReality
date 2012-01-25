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

bool GridLayout::CheckGridFit(Point2i gridPoint, Size_<int> gridSpan)
{
	if ((gridPoint.x + gridSpan.width) > gridSize.width || (gridPoint.x) < 0
		|| (gridPoint.y+gridSpan.height) > gridSize.height || (gridPoint.y) < 0)
	{
		LOGE("GridLayout: Point is outside of grid! Grid[%d,%d] Point[%d,%d] Span[%d,%d]",gridSize.width,gridSize.height,gridPoint.x,gridPoint.y,gridSpan.width,gridSpan.height);
		throw Exception(0,"Requested point exceeds grid size","AddChild","GridLayout.cpp",0);
	}
}

void GridLayout::AddChild(GridCompatible * child, Point2i gridPoint, Size_<int> gridSpan)
{
	CheckGridFit(gridPoint,gridSpan);

	child->gridPoint = gridPoint;
	child->gridSpan = gridSpan;
	child->DoGridLayout(Position, cellSize, gridPoint,gridSpan);	
	Children.push_back(child);
}

UIElement * GridLayout::GetElementAt(Point2i point)
{
	for (int i=0;i<Children.size();i++)
	{
		UIElement * child = Children.at(i)->GetElementAt(point);
		if (child != NULL)
			return child;
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

void GridLayout::DoGridLayout(Point2i offset, Size2i parentCellSize, Point2i gridPoint, Size2i gridSpan)
{
	Size2i windowSize = Size2i(parentCellSize.width * gridSpan.width, parentCellSize.height * gridSpan.height);
	cellSize = Size_<int>((int)((float)windowSize.width/gridSize.width),(int)((float)windowSize.height/gridSize.height));
	Position = offset + Point2i(gridPoint.x * parentCellSize.width,gridPoint.y * parentCellSize.height);
	
	LOGD(LOGTAG_INPUT,"Adding self(GridLayout) to grid. WindowSize=[%d,%d], Position=(%d,%d)",windowSize.width, windowSize.height, Position.x,Position.y);


	for (int i=0;i<Children.size();i++)
	{
		GridCompatible * gridChild = Children.at(i);
		gridChild->DoGridLayout(Position,cellSize,gridChild->gridPoint,gridChild->gridSpan);
	}

}

void GridLayout::Draw(Mat * rgbaImage)
{
	for (int i=0;i<Children.size();i++)
	{
		if (Children.at(i)->IsVisible())
			Children.at(i)->Draw(rgbaImage);
	}
}