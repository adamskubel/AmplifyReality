#include "LogDefinitions.h"
#include "FindPattern.h"
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

	std::string TextValue;

	QRCode(vector<FinderPattern*> *_finderPatterns, bool codeFound, Point2i _alignmentPattern = Point2i(0,0)) 
	{ 
		finderPatterns = _finderPatterns;
		validCodeFound = codeFound;
		alignmentPattern = _alignmentPattern;
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

	//Use cross product to arrange finder patterns into correct order
	static QRCode CreateFromFinderPatterns(FinderPattern_vector fpv)
	{
		FinderPattern * bottom_left, * top_left, * top_right;

		long d0to1 = FinderPattern::Distance(*fpv[0], *fpv[1]);
		long d1to2 = FinderPattern::Distance(*fpv[1], *fpv[2]);
		long d0to2 = FinderPattern::Distance(*fpv[0], *fpv[2]);

		if (d1to2 > d0to1 && d1to2 > d0to2)
		{
			top_left = fpv[0];
			bottom_left = fpv[1];
			top_right = fpv[2];
		} else if (d0to1 > d1to2 && d0to1 > d0to2)
		{
			top_left = fpv[2];
			bottom_left = fpv[1];
			top_right = fpv[0];
		}  else if (d0to2 > d1to2 && d0to2 > d0to1)
		{
			top_left = fpv[1];
			bottom_left = fpv[0];
			top_right = fpv[2];
		}
		else
		{
			LOGW("QRFinder","Unable to resolve finder pattern order by distance");
			//no clear winner, probably some significant perspective. 
			if (FinderPattern::AcuteAngleGeometry(*fpv[0], *fpv[1], *fpv[2]) >= 0)
			{
				top_left = fpv[0];
				top_right = fpv[1];
				bottom_left = fpv[2];
			}
			else if (FinderPattern::AcuteAngleGeometry(*fpv[1], *fpv[0], *fpv[2]) >= 0)
			{
				top_left = fpv[1];
				top_right = fpv[0];
				bottom_left = fpv[2];
			}
			else if (FinderPattern::AcuteAngleGeometry(*fpv[2], *fpv[0], *fpv[1]) >= 0)
			{
				top_left = fpv[2];
				top_right = fpv[0];
				bottom_left = fpv[1];
			}
			else
			{
				LOGE("Error determining finder pattern order. This shouldn't happen.");
				top_left = fpv[0];
				bottom_left = fpv[1];
				top_right = fpv[2];
			}
		}

		if (FinderPattern::AcuteAngleGeometry(*bottom_left, *top_left, *top_right) < 0)
		{
			FinderPattern * t = bottom_left;
			bottom_left = top_right;
			top_right = t;
		}

		vector<FinderPattern*> * patternVector = new vector<FinderPattern*>();
		patternVector->push_back(top_left);
		patternVector->push_back(top_right);
		patternVector->push_back(bottom_left);

		
		int fpSize = (int)round(top_left->size/2.0f);

		Point2i alignmentGuess;
		alignmentGuess.x = (top_right->pt.x - top_left->pt.x) + bottom_left->pt.x - fpSize;
		alignmentGuess.y = (top_right->pt.y - top_left->pt.y) + bottom_left->pt.y - fpSize;								
						
		return QRCode(patternVector, true,alignmentGuess);
	}


};

#endif 