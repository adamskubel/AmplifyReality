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
	Size2i size = GetTextSize();
	Position = Point2i(centerPoint.x - size.width/2, centerPoint.y + size.height/2);
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

	putText(*rgbaImage, Text.c_str(), Position, FontFace, FontScale, TextColor, FontThickness, CV_AA);	
}

