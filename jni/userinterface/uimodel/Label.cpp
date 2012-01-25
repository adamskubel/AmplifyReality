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


void Label::DoGridLayout(Point2i offset, Size2i cellSize, Point2i gridPoint, Size_<int> gridSpan)
{	
	Point2i newPoint = Point2i(gridPoint.x * cellSize.width + (gridSpan.width*cellSize.width/2),
		gridPoint.y * cellSize.height  + (gridSpan.height*cellSize.height/2));
	
	newPoint += offset;
	SetCenter(newPoint);	

	LOGI(LOGTAG_INPUT,"Adding myself(Label) to grid. Position = (%d,%d)",newPoint.x,newPoint.y);
}

