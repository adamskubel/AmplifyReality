#include "TextBox.hpp"

bool ITextListener::HandleKeyEvent(KeyEventArgs a)
{
	;
}

void ITextListener::SetFocus(bool focused)
{

}

void ITextListener::LostFocus()
{

}

void ITextListener::VirtualKeyboardEvent(bool opened)
{
	;
}

void ITextListener::AddFocusChangedListener(ITextListener * focusDelegate)
{

}
void ITextListener::RemoveFocusChangedListener(ITextListener * focusDelegate)
{

}

TextBox::TextBox(Size2i imageSize, std::string startText)
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
	textLabel = new Label("",Point2i(0,0),TextColor,Colors::Transparent);
	textLabel->SetLayout(false,true,true);
	obfuscateMode = false;
	
	SetText(startText);
}



void TextBox::HandleInput(TouchEventArgs args)
{
	if (args.InputType == ARInput::Press)
	{
		SetFocus(!isFocused);
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
	
	//Unfocus on KB close
	if (isFocused && !keyboardOpen)
		SetFocus(false);
}

void TextBox::LostFocus()
{	
	isFocused = false;	
	RefreshLabelText();
}

void TextBox::SetFocus(bool focus)
{
	isFocused = focus;	
	RefreshLabelText();
	
	if (focus)
	{
		for (int i=0;i<focusListenerVector.size();i++)
		{
			focusListenerVector[i]->LostFocus();
		}
	}
}

void TextBox::SetText(std::string text)
{
	currentText = text;
	RefreshLabelText();
}

std::string TextBox::GetText()
{
	return currentText;
}

void TextBox::AddFocusChangedListener(ITextListener * focusListener)
{
	focusListenerVector.push_back(focusListener);
}


void TextBox::RemoveFocusChangedListener(ITextListener * focusListener)
{
	for (int i=0;i<focusListenerVector.size();i++)
	{
		if (focusListenerVector[i] == focusListener)
		{
			focusListenerVector.erase(focusListenerVector.begin() + i);
			i--;
		}
	}
}


void TextBox::RefreshLabelText()
{
	if (obfuscateMode)
	{
		if (!isFocused)
		{
			std::string obfString = "";
			for (int i=0;i<currentText.size();i++)
			{
				obfString.push_back('*');
			}
			textLabel->SetText(obfString);
		}
		else
		{
			textLabel->SetText(currentText);
		}
	}
	else
	{
		textLabel->SetText(currentText);
	}
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


bool TextBox::HandleKeyEvent(KeyEventArgs args)
{
	if (isFocused)
	{				
		if (args.hasCharacter)
			currentText.push_back(args.KeyCharacter);
		else if (args.keyCode == AKEYCODE_DEL)
		{
			if (currentText.size() >= 2)
			{
				LOGV(LOGTAG_INPUT,"Erasing last character");
				currentText.erase(currentText.size() - 1,-1);
			}
			else
			{
				currentText.clear();
			}
		}

		RefreshLabelText();
		return true;
	}
	return false;
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
