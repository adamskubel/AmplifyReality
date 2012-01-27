#include <opencv2/core/core.hpp>
#include "model/Drawable.hpp"


#ifndef DEBUG_SHAPE_HPP_
#define DEBUG_SHAPE_HPP_

class DebugRectangle : public Drawable
{
public:
	DebugRectangle(Point2i center, int width, Scalar _color = Colors::Blue, bool fill = false)
	{
		rect = Rect(center.x - width, center.y - width, center.x + width, center.y + width);
		color = _color;
		thickness = (fill) ? -1 : (width > 40) ? 2 : 1;
	}

	DebugRectangle(Rect _rect, Scalar _color = Colors::Blue, bool fill = false)
	{
		rect = _rect;
		color = _color;
		thickness = (fill) ? -1 : (rect.width > 20) ? 2 : 1;
	}

	void Draw(Mat * rgbaImage)
	{
		rectangle(*rgbaImage,rect,color,thickness);
	}

private:
	Rect rect;
	Scalar color;
	int thickness;
	
};

class DebugCircle : public Drawable
{
public:
	DebugCircle(Point2i _center, int _radius, Scalar _color = Colors::Red, bool fill = false)
	{
		center = _center;
		radius = _radius;
		color = _color;
		thickness = (fill) ? -1 : (radius > 15) ? 2 : 1;
	}

	void Draw(Mat * rgbaImage)
	{		
		circle(*rgbaImage,center,radius,color,thickness);
	}

private:
	Point2i center;
	int radius, thickness;
	Scalar color;
	
	
};

#endif