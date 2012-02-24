#include "model/Drawable.hpp"
#include "userinterface/uimodel/UIElement.hpp"
#include "userinterface/events/EventArgs.hpp"
#include "userinterface/events/EventDelegates.hpp"
#include <opencv2/core/core.hpp>
#include "GridLayout.hpp"
#include "GraphicalUIElement.hpp"
#include "AmplifyRealityGlobals.hpp"
#include "Label.hpp"


#ifndef BUTTON_HPP_
#define BUTTON_HPP_

class Button : public GraphicalUIElement
{

public:
	Button(std::string label, cv::Scalar FillColor, cv::Scalar TextColor = Colors::Black);
	Button(std::string label, cv::Rect buttonBoundaries, cv::Scalar FillColor, cv::Scalar TextColor = Colors::Black);
	~Button();
	void AddClickDelegate(ClickEventDelegate myDelegate);
	UIElement* GetElementAt(Point2i p);	
	void Draw(Mat * rgbaImage);
	void HandleInput(TouchEventArgs args);
	void DoLayout(Rect boundaries);
	cv::Scalar FillColor, TextColor;

	void SetFillColor(Scalar color);
	
	void SetText(string text);
	void SetEnabled(bool enabled);
	bool IsEnabled();
	int BorderThickness;
	float Alpha;

private:
	cv::Rect buttonBoundaries;
	Scalar PressColor;
	std::vector<ClickEventDelegate> clickDelegateVector;
	Label * buttonLabel;
	bool isPressed, isEnabled;
	bool buttonChanged;
	Mat alphaBuffer;

};

#endif