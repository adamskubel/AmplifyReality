#ifndef TEXT_BOX_HPP_
#define TEXT_BOX_HPP_

#include "userinterface/uimodel/Label.hpp"
#include "userinterface/events/EventArgs.hpp"
#include "ITextListener.hpp"


class TextBox : public ITextListener, public GraphicalUIElement
{
public:
	TextBox(Size2i imageSize, std::string startText = "");
	bool HandleKeyEvent(KeyEventArgs keyEvent);
	void Draw(Mat * rgbaImage);
	void HandleInput(TouchEventArgs args);
	void VirtualKeyboardEvent(bool opened);
	Scalar FillColor, TextColor, FocusedColor, PressColor;
	int BorderThickness;
	void DoLayout(Rect rectangle);
	UIElement * GetElementAt(Point2i point);
	void SetFocus(bool focus);
	
	void SetText(std::string text);
	std::string GetText();

	void LostFocus();
	void AddFocusChangedListener(ITextListener * focusListener);
	void RemoveFocusChangedListener(ITextListener * focusListener);
	
	bool obfuscateMode;

private:
	Label * textLabel;
	bool isFocused, isPressed, keyboardOpen;
	Rect boundaryRectangle, keyboardOpenRectangle;
	std::string currentText;
	void RefreshLabelText();
	vector<ITextListener*> focusListenerVector;

};

#endif