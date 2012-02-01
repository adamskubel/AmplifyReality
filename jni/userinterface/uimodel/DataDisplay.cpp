
#include "DataDisplay.hpp"

DataDisplay::DataDisplay(const char * _format, cv::Scalar _textColor, cv::Scalar _backgroundColor) : GridLayout(Size2i(0,0))
{
	formatString = new std::string(_format);
	textColor = _textColor;
	backgroundColor = _backgroundColor;
	lastSize = Size2i(-1,-1);
}

void DataDisplay::SetData(Mat * matrix)
{
	Size2i matSize = Size2i(matrix->cols*matrix->channels(),matrix->rows);
	if (matSize != lastSize)
	{
		lastSize = matSize;
		Children.clear();
		LOGD(LOGTAG_INPUT,"Resizing grid to fit matrix. Newsize = [%d,%d]",matSize.width,matSize.height);
		ResizeGrid(controlSize, matSize, Position);

		for (int i = 0; i < matSize.height; i++)
		{		
			for (int j = 0; j < matSize.width; j++)
			{		
				char string[300];
				sprintf(string, formatString->c_str(),matrix->at<double>(i,j));
				LOGD(LOGTAG_INPUT,"Creating label with string %s in position (%d,%d)",string,j,i);
				Label * cellLabel = new Label(string,Point2i(0,0),textColor,backgroundColor);
				AddChild(cellLabel,Point2i(j,i));
			}
		}
		LOGD(LOGTAG_INPUT,"Data setting complete");

	}
	else
	{
		LOGD(LOGTAG_INPUT,"Matrix size not changed, setting new values");
		for (int i = 0; i < matSize.height; i++)
		{		
			for (int j = 0; j < matSize.width; j++)
			{		
				char string[300];
				sprintf(string, formatString->c_str(),matrix->at<double>(i,j));
				LOGD(LOGTAG_INPUT,"Setting text to '%s' for label in position (%d,%d)",string,j,i);

				Label * cellLabel = (Label*)GetElementAtCell(Point2i(j,i));
				if (cellLabel == NULL)
					LOGW(LOGTAG_INPUT,"Error! No element at cell (%d,%d)",j,i);
				else
				{
					cellLabel->SetText(string);
				}
			}
		}
	}

}
