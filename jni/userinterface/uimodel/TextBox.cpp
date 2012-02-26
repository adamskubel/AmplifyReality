#include "TextBox.hpp"

void ITextListener::HandleKeyEvent(KeyEventArgs a)
{
	;
}

void ITextListener::VirtualKeyboardEvent(bool opened)
{
	;
}

TextBox::TextBox(Size2i imageSize)
{
	keyboardOpenRectangle = Rect(0,0,imageSize.width,imageSize.height/2);
	isFocused = false;
	isPressed = false;
	keyboardOpen = false;
	TextColor = Colors::Black;
	FillColor = Colors::White;
	BorderThickness = 2;
	PressColor = Colors::MediumTurquoise;
	FocusedColor = Colors::OrangeRed;
	textLabel = new Label("X",Point2i(0,0),TextColor,Colors::Transparent);
	textLabel->SetLayout(false,true,true);
}

void TextBox::HandleInput(TouchEventArgs args)
{
	if (args.InputType == ARInput::Press)
	{
		isFocused = !isFocused;
		isPressed = false;
	}
	else if (args.InputType == ARInput::FingerDown)
	{
		isPressed = true;
	}
}

void TextBox::VirtualKeyboardEvent(bool opened)
{
	keyboardOpen = opened;
}

void TextBox::DoLayout(Rect boundaries)
{
	boundaryRectangle = boundaries;	
	textLabel->DoLayout(Rect(boundaries.x+BorderThickness,boundaries.y+BorderThickness,boundaries.width-(BorderThickness*2),boundaries.height-(BorderThickness*2)));
}



UIElement * TextBox::GetElementAt(cv::Point2i point)
{
	if (!IsVisible())
	{
		return NULL;
	}
	if (boundaryRectangle.contains(point))
	{
		return this;
	}
	else
	{
		return NULL;
	}
}


void TextBox::HandleKeyEvent(KeyEventArgs args)
{
	if (isFocused)
	{
		int32_t keyCode = args.keyCode;

		bool validKey = false;
		
		if (args.hasCharacter)
			currentText.push_back(args.KeyCharacter);
		else if (keyCode == AKEYCODE_DEL)
		{
			if (currentText.size() >= 2)
			{
				currentText.erase(currentText.size() - 2,1);
			}
			else
			{
				currentText.clear();
			}
		}

		textLabel->SetText(currentText);
	}
}

void TextBox::Draw(Mat * rgbaImage)
{
	Rect boundaryRectangle_draw = (isFocused && keyboardOpen) ? keyboardOpenRectangle : boundaryRectangle;

	Rect backgroundRect = Rect(boundaryRectangle_draw.x+BorderThickness,boundaryRectangle_draw.y+BorderThickness,
		boundaryRectangle_draw.width-(BorderThickness*2),boundaryRectangle_draw.height-(BorderThickness*2));
	cv::rectangle(*rgbaImage,backgroundRect,(isPressed) ? PressColor : FillColor,-1);
	cv::rectangle(*rgbaImage,boundaryRectangle_draw,(isFocused) ? FocusedColor : TextColor,BorderThickness,8);	
	
	if  (isFocused && keyboardOpen) 
	{
		textLabel->DoLayout(boundaryRectangle_draw);
		textLabel->Draw(rgbaImage);
		textLabel->DoLayout(boundaryRectangle);
	}
	else
	{
		textLabel->Draw(rgbaImage);
	}
	

}
