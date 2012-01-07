#include "userinterface/uimodel/UIElement.hpp"
#include "model/FrameItem.hpp"

#ifndef GRIDLAYOUT_HPP_
#define GRIDLAYOUT_HPP_

class GridLayout : public Element, public Updateable
{
public:	
	void AddChild(Element child, Point2i position);
	Element GetChildAt(Point2i p);	
	void Update(FrameItem * item);

private:
	vector<Element> childElements;

};

#endif GRIDLAYOUT_HPP_