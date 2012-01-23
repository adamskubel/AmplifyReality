#include "model/Drawable.hpp"
#include "userinterface/uimodel/UIElement.hpp"
#include "userinterface/events/EventArgs.hpp"
#include "userinterface/events/EventDelegates.hpp"
#include <opencv2/core/core.hpp>


#ifndef LABEL_HPP_
#define LABEL_HPP_

class Label : public UIElement, public Drawable
{

public:
	Label(std::string text,  cv::Point2i center, cv::Scalar textColor,  cv::Scalar fillColor = Colors::Transparent);
	~Label();
	UIElement* GetElementAt(Point2i p);	
	void Draw(cv::Mat * rgbaImage);
	void HandleInput();
	void SetCenter(Point2i centerPoint);
	Size2i GetTextSize();
	
	cv::Scalar FillColor, TextColor;
	std::string Text;
	//Bottom left corner
	cv::Point2i Position;
	int FontThickness, FontFace;
	float FontScale;

private:
	int fontBaseline;
};

#endif