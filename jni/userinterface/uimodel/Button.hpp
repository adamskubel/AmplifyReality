#include "model/Drawable.hpp"
#include "userinterface/uimodel/UIElement.hpp"
#include "userinterface/events/EventArgs.hpp"
#include "userinterface/events/EventDelegates.hpp"
#include <opencv2/core/core.hpp>
#include "GridLayout.hpp"

#ifndef BUTTON_HPP_
#define BUTTON_HPP_

class Button : public GridCompatible
{

public:
	Button(std::string label, cv::Scalar fillColor);
	Button(std::string label, cv::Rect buttonBoundaries, cv::Scalar fillColor);
	~Button();
	void AddClickDelegate(ClickEventDelegate myDelegate);
	UIElement* GetElementAt(Point2i p);	
	void Draw(Mat * rgbaImage);
	void HandleInput(TouchEventArgs args);
	void DoGridLayout(Point2i offset, Size2i windowSize, Point2i position, Size_<int> gridSpan);
	cv::Rect buttonBoundaries;
	cv::Scalar FillColor;
	std::string label;

	void SetEnabled(bool enabled);
	bool IsEnabled();

private:
	Scalar PressColor;
	std::vector<ClickEventDelegate> clickDelegateVector;

	bool isPressed, isEnabled;

};

#endif