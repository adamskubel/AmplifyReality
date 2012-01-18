#include "userinterface/uimodel/CertaintyIndicator.hpp"


CertaintyIndicator::CertaintyIndicator(float initialCertainty)
{
	certainty = initialCertainty;
	maxRadius = 1;
	CenterPoint = Point2i(0,0);
}


CertaintyIndicator::CertaintyIndicator(float initialCertainty, float _maxRadius)
{
	certainty = initialCertainty;
	maxRadius = _maxRadius;
	CenterPoint = Point2i(0,0);
}

cv::Scalar CertaintyIndicator::determineColor()
{
	if (certainty < 0.3f)
	{
		return cv::Scalar(200,117,51,255);
	}
	if (certainty < 0.6f)
	{
		return cv::Scalar(191,230,77,255);
	}
	return cv::Scalar(0,255,0,255);

}

void CertaintyIndicator::SetCertainty(float _certainty)
{
	certainty = _certainty;
}

void CertaintyIndicator::SetMaxRadius(float _maxRadius)
{
	maxRadius = _maxRadius;
}

void CertaintyIndicator::Draw(cv::Mat * rgbaImage)
{
	float percentRadius = (certainty < 0.2f) ? 0.2f : certainty;
	percentRadius = (percentRadius < 1.0f) ? percentRadius : 1.0f;

	cv::circle(*rgbaImage,CenterPoint,(maxRadius*(1.0f-percentRadius)),determineColor(),-1,CV_AA);
}
