
#include "DataDisplay.hpp"

DataDisplay::DataDisplay(const char * _format, cv::Scalar _textColor, cv::Scalar _backgroundColor) : GridLayout(Size2i(0,0))
{
	formatString = new std::string(_format);
	textColor = _textColor;
	backgroundColor = _backgroundColor;
	lastSize = Size2i(-1,-1);
}

static void WriteMatrixCell(std::stringstream & textStream, Mat * matrix, int i, int j)
{
	switch(matrix->depth())
	{
	case CV_8U:
		textStream << matrix->at<unsigned char>(i,j);
		break;
	case CV_8S:
		textStream << matrix->at<signed char>(i,j);
		break;
	case CV_16S:
		textStream << matrix->at<signed short>(i,j);
		break;
	case CV_32S:
		textStream << matrix->at<int>(i,j);
		break;
	case CV_32F:
		textStream << matrix->at<float>(i,j);
		break;
	case CV_64F:
		textStream << matrix->at<double>(i,j);
		break;
	default:
		textStream << "?";
	}
}

void DataDisplay::SetData(Mat * matrix)
{
	Size2i matSize = Size2i(matrix->cols*matrix->channels(),matrix->rows);
	

	if (matSize != lastSize)
	{
		lastSize = matSize;
		Children.clear();

		Size2i gridSize = matSize;
		if (matSize.width <= matSize.height)
			gridSize = Size2i(matSize.height,matSize.width);

		ResizeGrid(controlSize, gridSize, Position);

		for (int i = 0; i < matSize.height; i++)
		{		
			for (int j = 0; j < matSize.width; j++)
			{	
				std::stringstream textStream;
				textStream.precision(2);
				textStream.setf(std::ios_base::fixed);
				WriteMatrixCell(textStream,matrix,i,j);
				Label * cellLabel = new Label(textStream.str(),Point2i(0,0),textColor,backgroundColor);
				if (matSize.width > matSize.height)
					AddChild(cellLabel,Point2i(j,i));
				else
					AddChild(cellLabel,Point2i(i,j));

				textStream.clear();
			}
		}
	}
	else
	{

		for (int i = 0; i < matSize.height; i++)
		{		
			for (int j = 0; j < matSize.width; j++)
			{	
				std::stringstream textStream;
				textStream.precision(2);
				textStream.setf(std::ios_base::fixed);
				WriteMatrixCell(textStream,matrix,i,j);
				Point2i cellPoint;
				
				if (matSize.width > matSize.height)
					cellPoint = Point2i(j,i);
				else
					cellPoint = Point2i(i,j);
				
				Label * cellLabel = (Label*)GetElementAtCell(cellPoint);
				if (cellLabel == NULL)
					LOGW(LOGTAG_INPUT,"Error! No element at cell (%d,%d)",j,i);
				else
				{
					cellLabel->SetText(textStream.str());
				}
				textStream.clear();
			}
		}
	}
}
