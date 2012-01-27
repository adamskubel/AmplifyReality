#include "userinterface/uimodel/UIElementCollection.hpp"
#include "model/FrameItem.hpp"
#include "model/Engine.hpp"
#include "model/Drawable.hpp"


#ifndef GRIDLAYOUT_HPP_
#define GRIDLAYOUT_HPP_



using namespace cv;


class GridCompatible : public UIElement, public Drawable
{
public:
	virtual void DoGridLayout(Point2i offset, Size2i cellSize, Point2i gridPoint, Size2i gridSpan)
	{
		;
	}

	Point2i gridPoint;
	Size2i gridSpan;
};


class GridLayout : public GridCompatible
{
public:	
	GridLayout();
	GridLayout(Size2i windowSize, Size_<int> gridSize, Point2i position = Point2i(0,0));
	~GridLayout();

	//Point determines (x,y) position in grid. Top-left cell is (0,0)
	void AddChild(GridCompatible * child, Point2i position, Size_<int> gridSpan = Size_<int>(1,1));
	
	void Draw(Mat * rgbaImage);
	void DoGridLayout(Point2i offset, Size2i cellSize, Point2i gridPoint, Size2i gridSpan);

	UIElement * GetElementByName(std::string name);
	UIElement * GetElementAt(cv::Point2i point);

	GridCompatible * GetElementAtCell(cv::Point2i gridPoint);

protected:
	Point2i Position;
	vector<GridCompatible*> Children;
	Size_<int> gridSize, cellSize;

	bool CheckGridFit(Point2i gridPoint, Size_<int> gridSpan);

};

#endif