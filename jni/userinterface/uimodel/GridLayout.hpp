#include "userinterface/uimodel/UIElement.hpp"
#include "model/FrameItem.hpp"
#include "userinterface/uimodel/Button.hpp"
#include "userinterface/uimodel/Label.hpp"
#include "model/Engine.hpp"

#ifndef GRIDLAYOUT_HPP_
#define GRIDLAYOUT_HPP_

using namespace cv;

class GridLayout : public UIElement, public Updateable
{
public:	
	GridLayout(Engine * engine, Size_<int> gridSize);
	~GridLayout();

	//Point determines (x,y) position in grid. Top-left cell is (0,0)
	void AddChild(Button * child, Point2i position, Size_<int> gridSpan = Size_<int>(1,1));
	void AddChild(Label * child, Point2i position, Size_<int> gridSpan = Size_<int>(1,1));

	UIElement * GetChildAt(Point2i p);	
	void Update(FrameItem * item);

private:
	vector<UIElement*> childUIElements;
	vector<Updateable*> childUpdateElements;
	Size_<int> gridSize, cellSize;

	bool CheckGridFit(Point2i gridPoint, Size_<int> gridSpan);

};

#endif