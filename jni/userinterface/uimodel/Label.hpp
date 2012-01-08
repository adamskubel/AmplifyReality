#include "model/Updateable.hpp"
#include "userinterface/uimodel/UIElement.hpp"
#include "userinterface/events/EventArgs.hpp"
#include "userinterface/events/EventDelegates.hpp"
#include <opencv2/core/core.hpp>


#ifndef LABEL_HPP_
#define LABEL_HPP_

class Label : public UIElement, public Updateable
{

public:
	Label(std::string text,  cv::Point2i center, cv::Scalar textColor,  cv::Scalar fillColor);
	UIElement* GetChildAt(Point2i p);	
	void Update(FrameItem * item);
	void HandleInput();
	
	cv::Scalar FillColor, TextColor;
	std::string Text;
	cv::Point2i Center;
	
};

#endif