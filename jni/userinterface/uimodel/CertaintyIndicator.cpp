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
		return Colors::Red;
	}
	if (percentRadius > 0.5f)
	{
		return Colors::Orange;
	}
	if (percentRadius > 0.2f)
	{
		return Colors::Yellow;
	}
	return Colors::Lime;

}

void CertaintyIndicator::DoLayout(Rect boundaryRectangle)
{
	float newRadius = 0.5f * std::min(boundaryRectangle.width, boundaryRectangle.height);
	if (newRadius < maxRadius || maxRadius < MAX_CERTAINTY_INDICATOR_GROW_RADIUS)
		maxRadius = newRadius;

	CenterPoint = boundaryRectangle.tl() + Point2i(maxRadius,maxRadius);	
	LOGD(LOGTAG_INPUT,"Adding myself (CertaintyIndicator) to grid. Center=(%d,%d), Radius=(%f)",CenterPoint.x, CenterPoint.y, maxRadius);
}

//Constrain certainty
void CertaintyIndicator::SetCertainty(float certainty)
{
	if (certainty < 0.0f)
		percentRadius = 1.0f;
	else if (certainty == 1.0f)
		percentRadius = 0.2f;
	else if (certainty > 0.7f)
		percentRadius = 0.3f;
	else
		percentRadius = 1.0f - certainty;
}

void CertaintyIndicator::SetMaxRadius(float _maxRadius)
{
	maxRadius = _maxRadius;
}

void CertaintyIndicator::Draw(cv::Mat * rgbaImage)
{
	cv::circle(*rgbaImage,CenterPoint,maxRadius*percentRadius,determineColor(),-1,CV_AA);
}
