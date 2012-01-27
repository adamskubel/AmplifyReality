#include "LogDefinitions.h"
#include "positioning/FindPattern.h"
#include <opencv2/core/core.hpp>
#include "model/Drawable.hpp"

#ifndef QRCODE_HPP_
#define QRCODE_HPP_

using namespace cv;
//Represents a fully or partially located QR code
class QRCode : public Drawable
{
public:
	//If a valid code is found, patterns are stored in clockwise order starting with the top-left
	//Otherwise, effectively random
	vector<FinderPattern*> * finderPatterns;
	Point2i alignmentPattern;
	bool validCodeFound; 

	QRCode(vector<FinderPattern*> *_finderPatterns, bool codeFound) 
	{ 
		finderPatterns = _finderPatterns;
		validCodeFound = codeFound;
	}

	std::vector<cv::Point2i> getPatternsAsPoints()
	{
		std::vector<cv::Point2i> pVec; 
		for (int i=0;i<finderPatterns->size();i++)
		{
			pVec.push_back(finderPatterns->at(i)->pt);
		}
		return pVec;
	}

	~QRCode() 
	{		
		if (finderPatterns != NULL)
		{
			while (!finderPatterns->empty())
			{
				delete finderPatterns->back();
				finderPatterns->pop_back();
			}
			delete finderPatterns;	
			finderPatterns = NULL;
		}			
	}

	void Draw(Mat * rgbaImage)
	{
		if (validCodeFound)
		{
			//For each of the 3 finder patterns, store the point in an array to form the debug rectangle
			Point2i points[4];
						
			points[0] = (finderPatterns->at(0))->pt;
			points[1] = (finderPatterns->at(1))->pt;
			
			//Alignment Pattern is bottom right point
			points[2] = alignmentPattern;

			points[3] = (finderPatterns->at(2))->pt;
						
			int npts = 4;
			const Point2i * pArray[] = {points};
			polylines(*rgbaImage,pArray,&npts,1,true,Colors::Lime,6);
		}
		else 
		{
			for (size_t i = 0; i < finderPatterns->size(); i++)
			{
				FinderPattern  pattern = *(finderPatterns->at(i));

				//Use the pattern size to estimate a rectangle to draw
				int size = pattern.size / 2;

				//Create an array to pass to the polyline function
				Point2i points[4];
				points[0].x = (int) pattern.pt.x - size;
				points[0].y = (int) pattern.pt.y - size;

				points[1].x = (int) pattern.pt.x + size;
				points[1].y = (int) pattern.pt.y - size;

				points[2].x = (int) pattern.pt.x + size;
				points[2].y = (int) pattern.pt.y + size;

				points[3].x = (int) pattern.pt.x - size;
				points[3].y = (int) pattern.pt.y + size;

				int npts = 4;
				const Point2i * pArray[] = {points};
			
				polylines(*rgbaImage,pArray,&npts,1,true,Colors::Blue,4);
				
			}
		}
	}

};

#endif 