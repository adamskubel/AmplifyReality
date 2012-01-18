#include "userinterface/uimodel/UIElement.hpp"

#ifndef INPUTSCALER_HPP_
#define INPUTSCALER_HPP_

//Scales touch input using linear coefficients
class InputScaler
{
public:
	InputScaler(float _xScale, float _yScale, UIElement * childElement)
	{
		xScale = _xScale;
		yScale = _yScale;
	}

	UIElement * GetChildAt(cv::Point2i p)
	{
		p = cv::Point2i((int)(p.x*xScale),(int)(p.y*yScale));
		return childElement->GetChildAt(p);
	};

private:
	float xScale,yScale;
	UIElement * childElement;

};
#endif