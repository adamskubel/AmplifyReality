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

	DebugCircle(Point2f centerFloat, int _radius, Scalar _color = Colors::Red, bool fill = false)
	{
		instanceCount++;
		center = Point2i((int)round(centerFloat.x),(int)round(centerFloat.y));
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
	int radius, thickness;
	Scalar fillColor;
	
	
};

class DebugLine : public Drawable
{
public:
	
	DebugLine(Point2i _pt0, Point2i _pt1, Scalar _color, int _thickness = 1)
	{		
		pt0 = _pt0;
		pt1 = _pt1;
		fillColor = _color;
		thickness = _thickness;
	}
	
	~DebugLine()
	{
	}
	
	void Draw(Mat * rgbaImage)
	{		
		line(*rgbaImage,pt0,pt1,fillColor,thickness,8);
	}

private:
	int thickness;
	Scalar fillColor;
	Point2i pt0,pt1;	
};

class DebugLabel : public Drawable
{
public:
	DebugLabel(Point2i _position, std::string _text, Scalar _color = Colors::Black, float _scale = 0.5f,  Scalar _outlineColor = Colors::White)
	{
		position = _position;
		text = _text;
		color = _color;
		outlineColor = _outlineColor;
		scale = _scale;
	}

	void Draw(Mat * rgbaImage)
	{		
		if (outlineColor != Colors::Transparent)
		{
			putText(*rgbaImage, text.c_str(), position, FONT_HERSHEY_SIMPLEX, scale, outlineColor, 2, 8);	
		}
		putText(*rgbaImage, text.c_str(), position, FONT_HERSHEY_SIMPLEX, scale, color, 1, 8);	
	}

private:
	Point2i position;
	std::string text;
	Scalar color, outlineColor;
	float scale;
};



#endif