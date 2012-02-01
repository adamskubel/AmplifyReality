#include "GridLayout.hpp"
#include "Label.hpp"

#ifndef DATA_DISPLAY_HPP_
#define DATA_DISPLAY_HPP_


class DataDisplay : public GridLayout
{
public:	
	DataDisplay(const char * _format, cv::Scalar textColor, cv::Scalar backgroundColor = Colors::Transparent);		
	void SetData(Mat * data);

private: 
	cv::Scalar textColor, backgroundColor;
	std::string * formatString;
	Size2i lastSize;
};

#endif