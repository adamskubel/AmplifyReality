#include "userinterface/uimodel/Element.hpp"
#include "EventArgs.hpp"

#ifndef BUTTON_HPP_
#define BUTTON_HPP_

class Button : Element
{

	

public:
	Button();
	AddClickCallback(clickCallback);
	Element GetChildAt(Point2i p);	
	void Render(Engine* engine, FrameItem * item);

private:
	std::vector<clickCallback> callbackVector;

};

#endif