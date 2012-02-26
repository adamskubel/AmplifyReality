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

	scaleToFit = true;
	centerX = true;
	centerY = true;
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

void Label::SetLayout(bool _centerX, bool _centerY, bool _scaleToFit)
{
	centerX = _centerX;
	centerY = _centerY;
	scaleToFit = _scaleToFit;
}

void Label::SetCenter(Point2i centerPoint)
{	
	lastPosition = centerPoint;
	Size2i size = GetTextSize();
	Position = Point2i(centerPoint.x, centerPoint.y);
	if (centerX)
		Position.x = centerPoint.x - size.width/2;
	if (centerY)
		Position.y = centerPoint.y + size.height/2;

	if (Position.x < 0 || Position.y < 0)
	{
		LOGW(LOGTAG_INPUT,"Attempted to place label outside of screen! (%d,%d)",Position.x,Position.y);
		Position = Point2i(0,0);
	}

}

void Label::FitTextToBoundary(Size2f limits)
{
	lastSize = limits;
	Size2i size = GetTextSize();


	float ySpace = limits.height/(float)size.height;
	float xSpace = limits.width/(float)size.width;

	FontScale *= std::min(xSpace,ySpace);

	if (FontScale > 1.0f)
		FontScale = 1.0f;
	if (FontScale <= 0.1f)
		FontScale = 0.1f;

}

void Label::SetText(std::string newText)
{
	if (newText.compare(Text) == 0)
		return;

	Size2i size = GetTextSize();
	Text = newText;

	if (lastSize.width > 0 && lastSize.height > 0)
		FitTextToBoundary(lastSize);	

	if (lastPosition.x > 0 && lastPosition.y > 0)
		SetCenter(lastPosition);
	
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
	if(FillColor[3] > 0)
	{
		//If FillColor has nonzero alpha, draw background outline 
		putText(*rgbaImage, Text.c_str(), Position, FontFace, FontScale, FillColor, FontThickness*3, 8);
	}
	putText(*rgbaImage, Text.c_str(), Position, FontFace, FontScale, TextColor, FontThickness, 8);	
}


void Label::DoLayout(Rect boundaryRectangle)
{	
	LOGD(LOGTAG_INPUT,"Adding myself(Label) to layout. Rect = (%d,%d,%d,%d)",boundaryRectangle.x,boundaryRectangle.y,boundaryRectangle.width,boundaryRectangle.height);
	
	if (scaleToFit)
	{
		FitTextToBoundary(Size2f(boundaryRectangle.width,boundaryRectangle.height));
	}


	Point2i newPoint = Point2i( boundaryRectangle.x, boundaryRectangle.y);
	if (centerX || centerY)
	{
		if (centerY)
			newPoint.y =  boundaryRectangle.y + boundaryRectangle.height/2;
		if (centerX)
			newPoint.x = boundaryRectangle.x + boundaryRectangle.width/2;

		SetCenter(newPoint);	
	}
	else
	{
		Position = newPoint;
	}

}



