#include "model/Drawable.hpp"
#include "userinterface/uimodel/GridLayout.hpp"
#include "userinterface/events/EventArgs.hpp"
#include "userinterface/events/EventDelegates.hpp"
#include <opencv2/core/core.hpp>
#include "UIConstants.hpp"


#ifndef CERTAINTY_INDICATOR_HPP_
#define CERTAINTY_INDICATOR_HPP_

//A UI object that changes size and color based on its value
class CertaintyIndicator : public GraphicalUIElement
{
public:
	CertaintyIndicator(float initialCertainty);
	CertaintyIndicator(float initialCertainty, float maxRadius);
	void Draw(cv::Mat * rgbaImage);
	void SetCertainty(float certainty);
	void SetMaxRadius(float maxRadius);
	cv::Point2i CenterPoint;
	void DoLayout(Rect boundaryRectangle);

private:
	float percentRadius,maxRadius;
	cv::Scalar determineColor();

};

#endif