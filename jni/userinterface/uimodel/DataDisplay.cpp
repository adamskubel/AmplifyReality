
#include "DataDisplay.hpp"

DataDisplay::DataDisplay(const char * _format, Point2i _position, Size2i _controlSize, cv::Scalar _textColor, cv::Scalar _backgroundColor)
{
	formatString = new std::string(_format);
	controlSize = _controlSize;
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
		Children.clear();
		LOGD(LOGTAG_INPUT,"Resizing grid to fit matrix. Newsize = [%d,%d]",matSize.width,matSize.height);
		ResizeGrid(controlSize,matSize, Position);

		for (int i = 0; i < matSize.height; i++)
		{		
			for (int j = 0; j < matSize.width; j++)
			{		
				char string[300];
				sprintf(string, formatString->c_str(),matrix->at<double>(i,j));
				LOGD(LOGTAG_INPUT,"Creating label with string %s",string);
				Label * cellLabel = new Label(string,Point2i(0,0),textColor,backgroundColor);
				AddChild(cellLabel,Point2i(j,i));
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
				Label * cellLabel = (Label*)GetElementAtCell(Point2i(j,i));
				cellLabel->SetText(string);
			}
		}
	}

}
