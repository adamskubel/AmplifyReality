#include "model/Updateable.hpp"
#include "userinterface/uimodel/UIElement.hpp"
#include "userinterface/events/EventArgs.hpp"
#include "userinterface/events/EventDelegates.hpp"
#include <opencv2/core/core.hpp>


#ifndef BUTTON_HPP_
#define BUTTON_HPP_

class Button : public UIElement, public Updateable
{

public:
	Button(std::string label, cv::Rect buttonBoundaries, cv::Scalar fillColor);
	void AddClickDelegate(ClickEventDelegate myDelegate);
	UIElement* GetChildAt(Point2i p);	
	void Update(FrameItem * item);
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