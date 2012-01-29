#include "GridLayout.hpp"
#include "Label.hpp"

#ifndef DATA_DISPLAY_HPP_
#define DATA_DISPLAY_HPP_


class DataDisplay : public GridLayout
{
public:	
	DataDisplay(const char * _format, Point2i position, Size2i size, cv::Scalar textColor, cv::Scalar backgroundColor = Colors::Transparent);		
	void SetData(Mat * data);

private: 
	Point2i position;
	cv::Scalar textColor, backgroundColor;
	Size2i controlSize;
	std::string * formatString;
	Size2i lastSize;
};

#endif