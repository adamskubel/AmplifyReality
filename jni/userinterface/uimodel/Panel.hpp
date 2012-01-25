#include "UIElementCollection.hpp"


#ifndef AMPLIFY_UI_PANEL_HPP_
#define AMPLIFY_UI_PANEL_HPP_

class Panel : public UIElementCollection
{
public:
	Panel(cv::Point2i position)
	{
		Position = position;
	}

	void AddChild(UIElement * childElement, Point2i relativeLocation)
	{
		
		Children.push_back(childElement);
	}

	Point2i Position;
};

#endif