#include <opencv2/core/core.hpp>
#include "model/Drawable.hpp"


#ifndef DEBUG_SHAPE_HPP_
#define DEBUG_SHAPE_HPP_

class DebugRectangle : public Drawable
{
public:
	static int instanceCount;

	DebugRectangle(Point2i center, int width, Scalar _color = Colors::Blue, bool fill = false)
	{
		DebugRectangle::instanceCount++;
		rect = Rect(center.x - width, center.y - width, center.x + width, center.y + width);
		color = _color;
		thickness = (fill) ? -1 : (width > 40) ? 2 : 1;
	}

	DebugRectangle(Rect _rect, Scalar _color = Colors::Blue, bool fill = false)
	{
		DebugRectangle::instanceCount++;
		rect = _rect;
		color = _color;
		thickness = (fill) ? -1 : (rect.width > 20) ? 2 : 1;
	}

	~DebugRectangle()
	{
		DebugRectangle::instanceCount--;
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
	static int instanceCount;

	DebugCircle(Point2i _center, int _radius, Scalar _color = Colors::Red, bool fill = false)
	{
		instanceCount++;
		center = _center;
		radius = _radius;
		color = _color;
		thickness = (fill) ? -1 : (radius > 15) ? 2 : 1;
	}

	~DebugCircle()
	{
		instanceCount--;
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


class DebugPoly : public Drawable
{
public:
	static int instanceCount;
	vector<Point2i> points;


	DebugPoly(vector<Point> _points, Scalar _color, int _thickness = 1)
	{
		
		points = _points;
		fillColor = _color;
		thickness = _thickness;
	}
	
	DebugPoly(vector<Point2f> _points, Scalar _color, int _thickness = 1)
	{
		for (int i=0;i<_points.size();i++)
		{
			points.push_back(Point2i(_points[i].x,_points[i].y));
		}
		fillColor = _color;
		thickness = _thickness;
	}

	~DebugPoly()
	{
	}


	void Draw(Mat * rgbaImage)
	{		
		const Point* p = &points[0];
		int n = (int)points.size();
		polylines(*rgbaImage, &p, &n, 1, true, fillColor, thickness, CV_AA);
	}

private:
	Point2i center;
	int radius, thickness;
	Scalar fillColor;
	
	
};




#endif