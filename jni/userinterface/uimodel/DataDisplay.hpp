#include "GridLayout.hpp"
#include "Label.hpp"

#ifndef DATA_DISPLAY_HPP_
#define DATA_DISPLAY_HPP_


class DataDisplay 
{
public:	
	DataDisplay(const char * _format, Point2i position, Size2i size, cv::Scalar textColor, cv::Scalar backgroundColor = Colors::Transparent);
		
	void SetData(Mat * data);

	//void SetData(Point2i * data);
	//void SetData(Point2f * data);

	//void SetData(Point3i * data);
	//void SetData(Point3f * data);

private: 
	Point2i position;
	cv::Scalar textColor, backgroundColor;
	GridLayout * grid;
	Size2i size;
	std::string * formatString;
	GridLayout * grid;
	Size2i lastSize;
};

DataDisplay::DataDisplay(const char * _format, Point2i _position, Size2i _size, cv::Scalar _textColor, cv::Scalar _backgroundColor)
{
	grid = NULL;
	formatString = new std::string(_format);
	size = _size;
	position = _position;
	textColor = _textColor;
	backgroundColor = _backgroundColor;
	lastSize = Size2i(0,0);
}

void DataDisplay::SetData(Mat * matrix)
{
	Size2i matSize = Size2i(matrix->cols*matrix->channels(),matrix->rows);
	if (matSize != lastSize)
	{
		lastSize = matSize;
		if (grid != NULL)
			delete grid;

		grid = new GridLayout(size,matSize,position);			

		for (int i = 0; i < matSize.height; i++)
		{		
			for (int j = 0; j < matSize.width; j++)
			{		
				char string[300];
				sprintf(string, formatString->c_str(),matrix->at<double>(i,j));
				Label * cellLabel = new Label(string,Point2i(0,0),textColor,backgroundColor);
				grid->AddChild(cellLabel,Point2i(j,i));
			}
		}

	}
	else
	{
		for (int i = 0; i < matSize.height; i++)
		{		
			for (int j = 0; j < matSize.width; j++)
			{		
				char string[300];
				sprintf(string, formatString->c_str(),matrix->at<double>(i,j));
				Label * cellLabel = (Label*)grid->GetElementAtCell(Point2i(i,j));
				cellLabel->SetText(string);
			}
		}
	}

}










#endif