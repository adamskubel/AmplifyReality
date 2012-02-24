#include "userinterface/uimodel/Button.hpp"

Button::Button(std::string _label, cv::Scalar _FillColor, cv::Scalar _TextColor)
{
	buttonBoundaries = Rect(0,0,1,1);
	FillColor = _FillColor;
	buttonLabel = new Label(_label,Point2i(0,0),_TextColor,Colors::White);
	isPressed = false;
	PressColor = Scalar(124,225,252,255);
	clickDelegateVector = std::vector<ClickEventDelegate>();
	isEnabled = true;
	buttonChanged = true;
	Alpha = 1.0f;
	BorderThickness = UI_BORDER_THICKNESS;
}

Button::Button(std::string _label, cv::Rect _buttonBoundaries, cv::Scalar _FillColor, cv::Scalar _TextColor)
{
	buttonBoundaries = _buttonBoundaries;
	FillColor = _FillColor;
	isPressed = false;
	PressColor = Scalar(124,225,252,255);
	clickDelegateVector = std::vector<ClickEventDelegate>();
	buttonLabel = new Label(_label,Point2i(0,0),_TextColor,Colors::White);
	buttonLabel->DoLayout(buttonBoundaries);
	isEnabled = true;
	buttonChanged = true;
	Alpha = 1.0f;
	BorderThickness = UI_BORDER_THICKNESS;
}

Button::~Button()
{	
	delete buttonLabel;
}

void Button::AddClickDelegate(ClickEventDelegate myDelegate)
{
	LOGD(LOGTAG_INPUT,"Adding new delegate to button");
	clickDelegateVector.push_back(myDelegate);
}

UIElement * Button::GetElementAt(cv::Point2i point)
{
	if (!isEnabled || !IsVisible())
	{
		LOGV(LOGTAG_INPUT,"Button disabled, aborting hit test");
		return NULL;
	}
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
		buttonChanged = true;
		isPressed = false;
		for (int i=0;i<clickDelegateVector.size();i++)
		{
			EventArgs args = EventArgs();
			clickDelegateVector.at(i)(this,args);
		}
	}
	else if (args.InputType == ARInput::FingerDown)
	{
		buttonChanged = true;
		isPressed = true;
	}
}

void Button::DoLayout(Rect boundaries)
{
	if (boundaries.width != buttonBoundaries.width || boundaries.height != buttonBoundaries.height)
	{
		alphaBuffer = Mat(boundaries.height,boundaries.width,CV_8UC4);
		buttonChanged = true;
	}
	
	buttonBoundaries = boundaries;

	
	LOGD(LOGTAG_INPUT,"Adding myself(Button) to grid. X=%d,Y=%d,W=%d,H=%d",buttonBoundaries.x,buttonBoundaries.y,
		buttonBoundaries.width,buttonBoundaries.height);

	buttonLabel->DoLayout(Rect(boundaries.x+BorderThickness,boundaries.y+BorderThickness,boundaries.width-(BorderThickness*2),boundaries.height-(BorderThickness*2)));
}

void Button::SetText(std::string text)
{
	buttonLabel->SetText(text);
}

void Button::SetFillColor(Scalar _fillColor)
{
	buttonChanged = true;
	FillColor = _fillColor;
}

static void alphaBlend(Point2i offset, Mat & src, Mat *& dst, float alpha)
{
	int width = MIN(src.cols,dst->cols);
	float invAlpha = 1.0f - alpha;
	offset.x *= 4;
	width *= 4;
	//LOGV(LOGTAG_INPUT,"alpha=%f,1-alpha=%f,offset=(%d,%d),width=%d",alpha,invAlpha,offset.x,offset.y,width);
	for (int y = 0;y < src.rows && y < dst->rows;y++)
	{
		const unsigned char * rowPtr = src.ptr<unsigned char>(y);
		unsigned char * destPtr = dst->ptr<unsigned char>(y+offset.y);

		for (int x =0;x < width; x+= 4)
		{
			int destX = x+ offset.x;		
			//LOGV(LOGTAG_INPUT,"%u * %f + %u * %f",rowPtr[x+0],alpha,destPtr[destX+0],invAlpha);
			destPtr[destX+0] = saturate_cast<unsigned char>(rowPtr[x+0] * alpha + destPtr[destX+0] * invAlpha); 
			//LOGV(LOGTAG_INPUT,"=%u",destPtr[destX+0]);
			destPtr[destX+1] = saturate_cast<unsigned char>(rowPtr[x+1] * alpha + destPtr[destX+1] * invAlpha); 
			destPtr[destX+2] = saturate_cast<unsigned char>(rowPtr[x+2] * alpha + destPtr[destX+2] * invAlpha); 
		}
	}
}

void Button::Draw(Mat * rgbaImage)
{
	if (UI_ALPHA_ENABLED && Alpha < 1.0f && Alpha > 0.0f)
	{
		Rect buttonBackground = Rect(BorderThickness,BorderThickness,buttonBoundaries.width-(BorderThickness*2),buttonBoundaries.height-(BorderThickness*2));
		Rect buttonBorder = Rect(0,0,buttonBoundaries.width,buttonBoundaries.height);

		if (buttonChanged)
		{
			cv::rectangle(alphaBuffer,buttonBackground,(isPressed) ? PressColor : FillColor,CV_FILLED);
			buttonChanged = false;
		}

		alphaBlend(Point2i(buttonBoundaries.x,buttonBoundaries.y),alphaBuffer,rgbaImage, Alpha);
		
		if (UI_BORDER_COLOR != Colors::Transparent)
		{
			if (buttonChanged)
				cv::rectangle(alphaBuffer,buttonBoundaries,UI_BORDER_COLOR,2,8);		
			alphaBlend(Point2i(buttonBoundaries.x,buttonBoundaries.y),alphaBuffer,rgbaImage, Alpha);
		}
	}
	else 
	{
		Rect buttonBackground = Rect(buttonBoundaries.x+BorderThickness,buttonBoundaries.y+BorderThickness,buttonBoundaries.width-(BorderThickness*2),buttonBoundaries.height-(BorderThickness*2));
		cv::rectangle(*rgbaImage,buttonBackground,(isPressed) ? PressColor : FillColor,CV_FILLED);
		if (UI_BORDER_COLOR != Colors::Transparent)
		{
			cv::rectangle(*rgbaImage,buttonBoundaries,UI_BORDER_COLOR,2,8);
		}
	}


	//Draw label
	buttonLabel->Draw(rgbaImage);
}

void Button::SetEnabled(bool enabled)
{
	isEnabled = enabled;
}

bool Button::IsEnabled()
{
	return isEnabled && isVisible;
}