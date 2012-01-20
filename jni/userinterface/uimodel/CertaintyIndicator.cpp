#include "userinterface/uimodel/CertaintyIndicator.hpp"


CertaintyIndicator::CertaintyIndicator(float initialCertainty)
{
	SetCertainty(initialCertainty);
	maxRadius = 1;
	CenterPoint = Point2i(0,0);
}


CertaintyIndicator::CertaintyIndicator(float initialCertainty, float _maxRadius)
{
	SetCertainty(initialCertainty);
	maxRadius = _maxRadius;
	CenterPoint = Point2i(0,0);
}

cv::Scalar CertaintyIndicator::determineColor()
{
	if (percentRadius > 0.8f)
	{
		return cv::Scalar(200,117,51,255);
	}
	if (percentRadius > 0.4f)
	{
		return cv::Scalar(191,230,77,255);
	}
	return cv::Scalar(0,255,0,255);

}

//Constrain certainty
void CertaintyIndicator::SetCertainty(float certainty)
{
	if (certainty < 0.0f)
		percentRadius = 1.0f;
	else if (certainty > 0.8f)
		percentRadius = 0.2f;
	else
		percentRadius = 1.0f - certainty;
}

void CertaintyIndicator::SetMaxRadius(float _maxRadius)
{
	maxRadius = _maxRadius;
}

void CertaintyIndicator::Draw(cv::Mat * rgbaImage)
{
	LOGD(LOGTAG_INPUT,"Radius=%f",maxRadius*percentRadius);
	cv::circle(*rgbaImage,CenterPoint,maxRadius*percentRadius,determineColor(),-1,CV_AA);
}
