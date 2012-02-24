#include "model/DebugShape.hpp"





//Line
DebugLine::DebugLine(Point2i center, int width, Scalar _color, int _thickness)
{		
	pt0 = Point2i(center.x - width, center.y);
	pt1 = Point2i(center.x + width, center.y);
	fillColor = _color;
	thickness = _thickness;
}

DebugLine::DebugLine(Point2i center, Size2i size, Scalar _color, int _thickness)
{		
	pt0 = Point2i(center.x - size.width, center.y - size.height);
	pt1 = Point2i(center.x + size.width, center.y + size.height);
	fillColor = _color;
	thickness = _thickness;
}

DebugLine::DebugLine(Point2i _pt0, Point2i _pt1, Scalar _color, int _thickness)
{		
	pt0 = _pt0;
	pt1 = _pt1;
	fillColor = _color;
	thickness = _thickness;
}

void DebugLine::Draw(Mat * rgbaImage)
{		
	line(*rgbaImage,pt0,pt1,fillColor,thickness,8);
}

DebugLine::~DebugLine()
{
	;
}
	

//Rectangle
DebugRectangle::DebugRectangle(Rect _rect, Scalar _color, int _thickness)
{
	DebugRectangle::instanceCount++;
	rect = _rect;
	color = _color;
	thickness = _thickness;
}

DebugRectangle::DebugRectangle(Rect _rect, Scalar _color, bool fill)
{
	DebugRectangle::instanceCount++;
	rect = _rect;
	color = _color;
	thickness = (fill) ? -1 : (rect.width > 20) ? 2 : 1;
}

DebugRectangle::DebugRectangle(Point2i center, int width, Scalar _color, bool fill)
{
	DebugRectangle::instanceCount++;
	rect = Rect(center.x - width, center.y - width, center.x + width, center.y + width);
	color = _color;
	thickness = (fill) ? -1 : (width > 40) ? 2 : 1;
}

DebugRectangle::~DebugRectangle()
{
	DebugRectangle::instanceCount--;
}

void DebugRectangle::Draw(Mat * rgbaImage)
{
	rectangle(*rgbaImage,rect,color,thickness);
}


//Label
DebugLabel::DebugLabel(Point2i _position, std::string _text, Scalar _color, float _scale,  Scalar _outlineColor)
{
	position = _position;
	text = _text;
	color = _color;
	outlineColor = _outlineColor;
	scale = _scale;
}

void DebugLabel::Draw(Mat * rgbaImage)
{		
	if (outlineColor != Colors::Transparent)
	{
		putText(*rgbaImage, text.c_str(), position, FONT_HERSHEY_SIMPLEX, scale, outlineColor, 2, 8);	
	}
	putText(*rgbaImage, text.c_str(), position, FONT_HERSHEY_SIMPLEX, scale, color, 1, 8);	
}

//Circle

DebugCircle::DebugCircle(Point2i _center, int _radius, Scalar _color, int _thickness, bool _dotCenter)
{
	instanceCount++;
	center = _center;
	radius = _radius;
	color = _color;
	thickness = _thickness;
	dotCenter = _dotCenter;
}

DebugCircle::DebugCircle(Point2i _center, int _radius, Scalar _color, bool fill)
{
	instanceCount++;
	center = _center;
	radius = _radius;
	color = _color;
	thickness = (fill) ? -1 : (radius > 15) ? 2 : 1;
}

DebugCircle::DebugCircle(Point2f centerFloat, int _radius, Scalar _color, bool fill)
{
	instanceCount++;
	center = Point2i((int)round(centerFloat.x),(int)round(centerFloat.y));
	radius = _radius;
	color = _color;
	thickness = (fill) ? -1 : (radius > 15) ? 2 : 1;
}

DebugCircle::~DebugCircle()
{
	instanceCount--;
}


void DebugCircle::Draw(Mat * rgbaImage)
{
	if (radius > 0)
	{
		circle(*rgbaImage,center,radius,color,thickness);
		if (dotCenter)
			circle(*rgbaImage,center,1,color,1);
	}
	else
		rgbaImage->at<int32_t>(center.y,center.x) = ((unsigned char)color[3] << 24) + ((unsigned char)color[2] << 16) + ((unsigned char)color[1] << 8)  + ((unsigned char)color[0]);
}

//Polygon

DebugPoly::DebugPoly(vector<Point> _points, Scalar _color, int _thickness)
{

	points = _points;
	fillColor = _color;
	thickness = _thickness;
}

DebugPoly::DebugPoly(vector<Point2f> _points, Scalar _color, int _thickness)
{
	for (int i=0;i<_points.size();i++)
	{
		points.push_back(Point2i(_points[i].x,_points[i].y));
	}
	fillColor = _color;
	thickness = _thickness;
}

DebugPoly::~DebugPoly()
{
}


void DebugPoly::Draw(Mat * rgbaImage)
{		
	const Point* p = &points[0];
	int n = (int)points.size();
	polylines(*rgbaImage, &p, &n, 1, true, fillColor, thickness, CV_AA);
}

