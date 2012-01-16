#include "userinterface/uimodel/Label.hpp"

Label::Label(std::string text,  cv::Point2i center, cv::Scalar textColor,  cv::Scalar fillColor)
{
	Center = center;
	TextColor = textColor;
	FillColor = fillColor;
	Text = text;
}

Label::~Label()
{
	;
}

UIElement * Label::GetChildAt(cv::Point2i point)
{
	return NULL;
}

void Label::HandleInput()
{
	return;
}

void Label::Update(FrameItem * item)
{
	//Draw label
	int fontFace = FONT_HERSHEY_SIMPLEX;
	double fontScale = 1.5;
	int thickness = 2;
	int baseline = 0;
	Size textSize = getTextSize(Text.c_str(), fontFace, fontScale, thickness, &baseline);
	baseline += thickness;

	//Point2i textLocation = Point2i(Center.x - textSize.width/2,Center.y - textSize.height/2);
	
	putText(*(item->rgbImage), Text.c_str(), Center, fontFace, fontScale, TextColor, thickness, 8);
}

