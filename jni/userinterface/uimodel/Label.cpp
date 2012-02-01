#include "userinterface/uimodel/Label.hpp"


Label::Label(std::string text,  cv::Point2i position, cv::Scalar textColor,  cv::Scalar fillColor)
{
	Position = position;
	TextColor = textColor;
	FillColor = fillColor;
	Text = text;

	FontScale = 1.5;
	FontThickness = 2;
	FontFace = FONT_HERSHEY_SIMPLEX;

	fontBaseline = 0;
}


Label::~Label()
{
	;
}

UIElement * Label::GetElementAt(cv::Point2i point)
{
	return NULL;
}

void Label::HandleInput()
{
	return;
}

void Label::SetCenter(Point2i centerPoint)
{	
	LOGD(LOGTAG_INPUT,"Setting label center (%d,%d)",centerPoint.x,centerPoint.y);
	Size2i size = GetTextSize();
	Position = Point2i(centerPoint.x - size.width/2, centerPoint.y + size.height/2);
	
	if (Position.x < 0 || Position.y < 0)
		Position = Point2i(0,0);

	LOGD(LOGTAG_INPUT,"New position is (%d,%d)",Position.x,Position.y);
}

void Label::FitTextToBoundary(Size2f limits)
{
	Size2i size = GetTextSize();

	/*if (FontScale > 2 && size.width <= limits.width && size.height <= limits.height)
		return;*/

	float ySpace = limits.height/(float)size.height;
	float xSpace = limits.width/(float)size.width;
		
	FontScale *= std::min(xSpace,ySpace);

	if (FontScale > 2.0f)
		FontScale = 2.0f;
	if (FontScale <= 0)
		FontScale = 0.1f;
	
	LOGD(LOGTAG_INPUT,"New FontScale is %f",FontScale);
}

void Label::SetText(std::string newText)
{
	Size2i size = GetTextSize();
	Point2i originalPosition = Point2i(Position.x + size.width/2, Position.y - size.height/2);
	Text = newText;
	SetCenter(originalPosition);
}

cv::Size2i Label::GetTextSize()
{	
	fontBaseline = 0;
	Size textSize = getTextSize(Text.c_str(), FontFace, FontScale, FontThickness, &fontBaseline);
	fontBaseline += FontThickness;
	return textSize;
}

void Label::Draw(Mat * rgbaImage)
{	
	//If FillColor has nonzero alpha, draw background rectangle
	if(FillColor[3] > 0)
	{
		Size2i fontSize = GetTextSize();
		rectangle(*rgbaImage,Position,Point2i(Position.x + fontSize.width, Position.y - fontSize.height),FillColor,-1,CV_AA);
	}

	putText(*rgbaImage, Text.c_str(), Position, FontFace, FontScale, TextColor, FontThickness, 8);	
}

void Label::DoLayout(Rect boundaryRectangle)
{
	
	LOGI(LOGTAG_INPUT,"Adding myself(Label) to layout. Rect = (%d,%d,%d,%d)",boundaryRectangle.x,boundaryRectangle.y,boundaryRectangle.width,boundaryRectangle.height);

	Point2i newPoint = Point2i(boundaryRectangle.x + boundaryRectangle.width/2, boundaryRectangle.y + boundaryRectangle.height/2);
	
	FitTextToBoundary(Size2f(boundaryRectangle.width,boundaryRectangle.height));
	
	SetCenter(newPoint);	
}


