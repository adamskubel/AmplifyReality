#include "model/Drawable.hpp"
#include "userinterface/uimodel/UIElement.hpp"
#include "userinterface/events/EventArgs.hpp"
#include "userinterface/events/EventDelegates.hpp"
#include <opencv2/core/core.hpp>


#ifndef BUTTON_HPP_
#define BUTTON_HPP_

class Button : public UIElement, public Drawable
{

public:
	Button(std::string label, cv::Scalar fillColor);
	Button(std::string label, cv::Rect buttonBoundaries, cv::Scalar fillColor);
	~Button();
	void AddClickDelegate(ClickEventDelegate myDelegate);
	UIElement* GetElementAt(Point2i p);	
	void Draw(Mat * rgbaImage);
	void HandleInput(TouchEventArgs args);
	cv::Rect buttonBoundaries;
	cv::Scalar FillColor;
	std::string label;

private:
	Scalar PressColor;
	std::vector<ClickEventDelegate> clickDelegateVector;

	bool isPressed;

};

#endif