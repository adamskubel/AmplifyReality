#include "model/DebugShape.hpp"


void DebugCircle::Draw(Mat * rgbaImage)
{
	if (radius > 0)
		circle(*rgbaImage,center,radius,color,thickness);
	else
		rgbaImage->at<int32_t>(center.y,center.x) = ((unsigned char)color[3] << 24) + ((unsigned char)color[2] << 16) + ((unsigned char)color[1] << 8)  + ((unsigned char)color[0]);
}

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