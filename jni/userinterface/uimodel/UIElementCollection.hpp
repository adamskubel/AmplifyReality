#include "UIElement.hpp"


#ifndef UIELEMENTCOLLECTION_HPP_
#define UIELEMENTCOLLECTION_HPP_

class UIElementCollection : public UIElement
{
public:
	UIElementCollection()
	{
		;
	}
			
	void AddChild(UIElement * child)
	{
		Children.push_back(child);
	}

	UIElement * GetElementByName(std::string name)
	{
		for (int i=0;i<Children.size();i++)
		{
			if (!Children.at(i)->Name.empty() && name.compare(Children.at(i)->Name) == 0)
				return Children.at(i);
		}
		return NULL;
	}

	UIElement * GetElementAt(cv::Point2i point)
	{
		for (int i=0;i<Children.size();i++)
		{
			UIElement * child = Children.at(i)->GetElementAt(point);
			if (child != NULL)
				return child;
		}
		return NULL;
	}

protected:
	vector<UIElement*> Children;

};

#endif