#include "userinterface/uimodel/UIElementCollection.hpp"
#include "model/FrameItem.hpp"
#include "model/Engine.hpp"
#include "model/Drawable.hpp"
#include "GraphicalUIElement.hpp"


#ifndef GRIDLAYOUT_HPP_
#define GRIDLAYOUT_HPP_



using namespace cv;


class GridLayout : public GraphicalUIElement
{
public:	
	GridLayout();
	GridLayout(Size2i windowSize, Size_<int> gridSize, Point2i position = Point2i(0,0));
	~GridLayout();

	void ResizeGrid(Size2i windowSize, Size2i gridSize, Point2i position = Point2i(0,0));

	//Point determines (x,y) position in grid. Top-left cell is (0,0)
	void AddChild(GraphicalUIElement * child, Point2i position, Size_<int> gridSpan = Size_<int>(1,1));
	
	void Draw(Mat * rgbaImage);
	void DoLayout(Rect boundaries);

	UIElement * GetElementByName(std::string name);
	UIElement * GetElementAt(cv::Point2i point);

	GraphicalUIElement * GetElementAtCell(cv::Point2i gridPoint);

protected:
	Point2i Position;
	vector<GraphicalUIElement*> Children;
	Size_<int> gridSize, cellSize;
	Rect GetRectangleFromGridData(Point2i gridPoint, Size2i gridSpan);
	bool CheckGridFit(Point2i gridPoint, Size_<int> gridSpan);

};

#endif