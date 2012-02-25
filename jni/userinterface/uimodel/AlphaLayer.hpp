#include "userinterface/uimodel/GraphicalUIElement.hpp"

#ifndef ALPHA_LAYER_HPP_
#define ALPHA_LAYER_HPP_

class AlphaLayer : public GraphicalUIElement
{
public:
	AlphaLayer(GraphicalUIElement * _childElement);
	UIElement * GetElementAt(cv::Point2i p);
	void DoLayout(Rect boundaryRectangle);


private:
	float alphaScale;
	GraphicalUIElement * childElement;

};
#endif