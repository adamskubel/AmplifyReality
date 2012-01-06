#include "Element.hpp"
#include "model/Engine.hpp"
#include "model/FrameItem.hpp"

#ifndef GRIDLAYOUT_HPP_
#define GRIDLAYOUT_HPP_

class GridLayout : Element
{
public:	
	void AddChild(Element child, Point2i position);
	Element GetChildAt(Point2i p);	
	void Render(Engine* engine, FrameItem * item);

private:
	vector<Element> childElements;

};

#endif GRIDLAYOUT_HPP_