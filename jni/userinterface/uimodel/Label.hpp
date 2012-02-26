#include "model/Drawable.hpp"
#include "userinterface/uimodel/GridLayout.hpp"
#include "userinterface/events/EventArgs.hpp"
#include "userinterface/events/EventDelegates.hpp"
#include <opencv2/core/core.hpp>
#include "GraphicalUIElement.hpp"



#ifndef LABEL_HPP_
#define LABEL_HPP_

class Label : public GraphicalUIElement
{

public:
	Label(std::string text,  cv::Point2i center, cv::Scalar textColor,  cv::Scalar fillColor = Colors::Transparent);
	~Label();
	UIElement* GetElementAt(Point2i p);	
	void Draw(cv::Mat * rgbaImage);
	void HandleInput();
	void SetCenter(Point2i centerPoint);
	Size2i GetTextSize();
	
	void DoLayout(Rect boundaryRectangle);

	void FitTextToBoundary(Size2f limits);

	cv::Scalar FillColor, TextColor;
	//Bottom left corner
	cv::Point2i Position;
	int FontThickness, FontFace;
	float FontScale;

	void SetText(std::string newText);

	void SetLayout(bool centerX,bool centerY, bool scaleToFit);

private:
	int fontBaseline;
	Size2i lastSize;
	Point2i lastPosition;
	std::string Text;
	bool centerX,centerY, scaleToFit;
};

#endif