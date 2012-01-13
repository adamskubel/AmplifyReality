#include "userinterface/uimodel/Button.hpp"

Button::Button(std::string _label, cv::Rect _buttonBoundaries, cv::Scalar _fillColor)
{
	buttonBoundaries = _buttonBoundaries;
	FillColor = _fillColor;
	label = _label;
	isPressed = false;
	PressColor = Scalar(124,225,252,255);
	clickDelegateVector = std::vector<ClickEventDelegate>();
}

void Button::AddClickDelegate(ClickEventDelegate myDelegate)
{
	LOGD(LOGTAG_INPUT,"Adding new delegate to button");
	clickDelegateVector.push_back(myDelegate);
}

UIElement * Button::GetChildAt(cv::Point2i point)
{
	LOGD(LOGTAG_INPUT,"Button: Testing point (%d,%d)",point.x,point.y);
	if (buttonBoundaries.contains(point))
	{
		LOGD(LOGTAG_INPUT,"Button: Point is inside!");
		return this;
	}
	return NULL;
}

void Button::HandleInput(TouchEventArgs args)
{
	LOGD(LOGTAG_INPUT,"Button: Handling input");
	if (args.InputType == ARInput::Press)
	{
		isPressed = false;
		for (int i=0;i<clickDelegateVector.size();i++)
		{
			EventArgs args = EventArgs();
			clickDelegateVector.at(i)(this,args);
		}
	}
	else if (args.InputType == ARInput::FingerDown)
	{
		isPressed = true;
	}
}


void Button::Update(FrameItem * item)
{
	//Draw button background
	cv::rectangle(*(item->rgbImage),buttonBoundaries,(isPressed) ? PressColor : FillColor,CV_FILLED);
	//Draw border
	cv::rectangle(*(item->rgbImage),buttonBoundaries,Scalar::all(0),2,CV_AA);


	//Draw button label
	int fontFace = FONT_HERSHEY_SIMPLEX;
	double fontScale = 1.2;
	int thickness = 2;
	int baseline = 0;
	Size textSize = getTextSize(label.c_str(), fontFace, fontScale, thickness, &baseline);

	Point2i textLocation = Point2i(buttonBoundaries.x + (buttonBoundaries.width - textSize.width)/2,
		baseline + buttonBoundaries.y + (buttonBoundaries.height - textSize.height)/2);
	putText(*(item->rgbImage), label.c_str(), textLocation, fontFace, fontScale, Scalar::all(255), thickness, CV_AA);
}

