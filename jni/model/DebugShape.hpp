#include <opencv2/core/core.hpp>
#include "model/Drawable.hpp"


#ifndef DEBUG_SHAPE_HPP_
#define DEBUG_SHAPE_HPP_

class DebugRectangle : public Drawable
{
public:
	static int instanceCount;

	DebugRectangle(Point2i center, int width, Scalar _color = Colors::Blue, bool fill = false);
	DebugRectangle(Rect _rect, Scalar _color = Colors::Blue, bool fill = false);
	DebugRectangle(Rect _rect, Scalar _color, int thickness);
	~DebugRectangle();
	void Draw(Mat * rgbaImage);

private:
	Rect rect;
	Scalar color;
	int thickness;
	
};

class DebugCircle : public Drawable
{
public:
	static int instanceCount;
	DebugCircle(Point2i _center, int _radius, Scalar _color, int _thickness);
	DebugCircle(Point2i _center, int _radius, Scalar _color = Colors::Red, bool fill = false);
	DebugCircle(Point2f centerFloat, int _radius, Scalar _color = Colors::Red, bool fill = false);
	~DebugCircle();
	void Draw(Mat * rgbaImage);

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
	DebugPoly(vector<Point> _points, Scalar _color, int _thickness = 1);	
	DebugPoly(vector<Point2f> _points, Scalar _color, int _thickness = 1);
	~DebugPoly();
	void Draw(Mat * rgbaImage);
private:
	int radius, thickness;
	Scalar fillColor;	
};

class DebugLine : public Drawable
{
public:
	
	DebugLine(Point2i _pt0, Point2i _pt1, Scalar _color, int _thickness = 1);	
	DebugLine(Point2i center, int width, Scalar _color, int _thickness = 1);
	DebugLine(Point2i center, Size2i size, Scalar _color, int _thickness = 1);
	
	~DebugLine();	
	void Draw(Mat * rgbaImage);

private:
	int thickness;
	Scalar fillColor;
	Point2i pt0,pt1;	
};

class DebugLabel : public Drawable
{
public:
	DebugLabel(Point2i _position, std::string _text, Scalar _color = Colors::Black, float _scale = 0.5f,  Scalar _outlineColor = Colors::White);
	void Draw(Mat * rgbaImage);
private:
	Point2i position;
	std::string text;
	Scalar color, outlineColor;
	float scale;
};



#endif