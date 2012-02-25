#include "AlphaLayer.hpp"


AlphaLayer::AlphaLayer(GraphicalUIElement * _childElement)
{
	childElement = _childElement;
}


void AlphaLayer::DoLayout(Rect rect)
{
	childElement->DoLayout(rect);
}

UIElement * AlphaLayer::GetElementAt(cv::Point2i p)
{
	return childElement->GetElementAt(p);
}

