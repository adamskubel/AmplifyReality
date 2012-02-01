#include "model/FrameItem.hpp"
#include "model/Engine.hpp"
#include "model/Drawable.hpp"


#ifndef GRAPHICAL_UIELEMENT_HPP_
#define GRAPHICAL_UIELEMENT_HPP_

using namespace cv;


class GraphicalUIElement : public UIElement, public Drawable
{
public:
	GraphicalUIElement()
	{
		isVisible = true;
		layoutDefined = false;
	}

	GraphicalUIElement(bool layout)
	{
		layoutDefined = layout;
		isVisible = true;
	}

	virtual void DoLayout(Rect boundaryRectangle)
	{
		;
	}
	
	Point2i gridPoint;
	Size2i gridSpan;

	virtual void SetVisible(bool visible)
	{
		isVisible = visible;
	}

	virtual bool IsVisible()
	{
		return isVisible;
	}

protected:
	bool isVisible;
	bool layoutDefined;

};

#endif