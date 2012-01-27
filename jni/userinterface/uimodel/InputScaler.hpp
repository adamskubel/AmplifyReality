#include "userinterface/uimodel/UIElement.hpp"

#ifndef INPUTSCALER_HPP_
#define INPUTSCALER_HPP_

//Scales touch input using linear coefficients
class InputScaler : public UIElement
{
public:
	InputScaler(float scale, UIElement * _childElement)
	{
		xScale = scale;
		yScale = scale;
		childElement = _childElement;
		LOGI(LOGTAG_INPUT,"Created InputScaler, xScale=%f,yScale=%f",xScale,yScale);
	}

	InputScaler(float _xScale, float _yScale, UIElement * _childElement)
	{
		xScale = _xScale;
		yScale = _yScale;
		childElement = _childElement;
		LOGI(LOGTAG_INPUT,"Created InputScaler, xScale=%f,yScale=%f",xScale,yScale);
	}

	InputScaler(cv::Size2i target, cv::Size2i screen, UIElement * _childElement, bool maintainRatio = true)
	{
		xScale = (float)target.width/(float)screen.width;
		yScale = (float)target.height/(float)screen.height;
		
		if (maintainRatio)
		{
			float scale = max(xScale,yScale);
			xScale = yScale = scale;
		}

		childElement = _childElement;
		LOGI(LOGTAG_INPUT,"Created InputScaler, xScale=%f,yScale=%f",xScale,yScale);
	}

	UIElement * GetElementAt(cv::Point2i p)
	{
		float newX = (float)p.x * xScale;
		float newY = (float)p.y * yScale;
		cv::Point2i newPoint  = cv::Point2i((int)newX,(int)newY);
		LOGV(LOGTAG_INPUT,"Translated (%d,%d) to (%d,%d)",p.x,p.y,newPoint.x,newPoint.y);
		return childElement->GetElementAt(newPoint);
	}

private:
	float xScale,yScale;
	UIElement * childElement;

};
#endif