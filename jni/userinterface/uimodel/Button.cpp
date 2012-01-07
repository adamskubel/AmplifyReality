#include "userinterface/uimodel/Button.hpp"

Button::Button(cv::Rect _buttonBoundaries, cv::Scalar _fillColor)
{
	buttonBoundaries = _buttonBoundaries;
	FillColor = _fillColor;

	clickDelegateVector = std::vector<ClickEventDelegate>();
}

void Button::AddClickDelegate(ClickEventDelegate myDelegate)
{
	LOGD(LOGTAG_BUTTON,"Adding new delegate to button");
	clickDelegateVector.push_back(myDelegate);
}

UIElement * Button::GetChildAt(cv::Point2i point)
{
	LOGD(LOGTAG_BUTTON,"Testing point (%d,%d)",point.x,point.y);
	if (buttonBoundaries.contains(point))
	{
		LOGD(LOGTAG_BUTTON,"Point is inside!");
		return this;
	}
	return NULL;
}

void Button::HandleInput()
{
	LOGD(LOGTAG_BUTTON,"Handling input");
	for (int i=0;i<clickDelegateVector.size();i++)
	{
		EventArgs args = EventArgs();
		clickDelegateVector.at(i)(this,args);
	}
}

void Button::Update(FrameItem * item)
{
	cv::rectangle(*(item->rgbImage),buttonBoundaries,FillColor,-1);
}
