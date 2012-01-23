#include "userinterface/uimodel/UIElementCollection.hpp"
#include "model/FrameItem.hpp"
#include "userinterface/uimodel/Button.hpp"
#include "userinterface/uimodel/Label.hpp"
#include "userinterface/uimodel/CertaintyIndicator.hpp"
#include "model/Engine.hpp"
#include "model/Drawable.hpp"

#ifndef GRIDLAYOUT_HPP_
#define GRIDLAYOUT_HPP_

using namespace cv;

class GridLayout : public UIElementCollection, public Drawable
{
public:	
	GridLayout(Engine * engine, Size_<int> gridSize);
	~GridLayout();

	//Point determines (x,y) position in grid. Top-left cell is (0,0)
	void AddChild(CertaintyIndicator * child, Point2i position, Size_<int> gridSpan = Size_<int>(1,1));
	void AddChild(Button * child, Point2i position, Size_<int> gridSpan = Size_<int>(1,1));
	void AddChild(Label * child, Point2i position, Size_<int> gridSpan = Size_<int>(1,1));

	void Draw(Mat * rgbaImage);

private:
	vector<Drawable*> childDrawElements;
	Size_<int> gridSize, cellSize;

	bool CheckGridFit(Point2i gridPoint, Size_<int> gridSpan);

};

#endif