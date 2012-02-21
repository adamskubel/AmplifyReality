#include "QRCode.hpp"




QRCode::QRCode(vector<FinderPattern*> _finderPatterns, bool codeFound, Point2i _alignmentPattern) 
{ 
	QRCode::instanceCount++;
	finderPatterns = _finderPatterns;
	validCodeFound = codeFound;
	alignmentPattern = _alignmentPattern;
}



QRCode::~QRCode() 
{		
	QRCode::instanceCount--;

	LOGD(LOGTAG_QR,"Deleting %d FPs",finderPatterns.size());
	while (!finderPatterns.empty())
	{
		delete finderPatterns.back();
		finderPatterns.pop_back();
	}

}

vector<cv::Point2i> QRCode::getPatternsAsPoints()
{
	std::vector<cv::Point2i> pVec; 
	for (int i=0;i<finderPatterns.size();i++)
	{
		pVec.push_back(finderPatterns.at(i)->pt);
	}
	return pVec;
}

void QRCode::Draw(Mat * rgbaImage)
{
	if (validCodeFound)
	{
		//For each of the 3 finder patterns, store the point in an array to form the debug rectangle
		Point2i points[4];

		points[0] = (finderPatterns.at(0))->pt;
		points[1] = (finderPatterns.at(1))->pt;

		//Alignment Pattern is bottom right point
		points[2] = alignmentPattern;

		points[3] = (finderPatterns.at(2))->pt;

		int npts = 4;
		const Point2i * pArray[] = {points};
		polylines(*rgbaImage,pArray,&npts,1,true,Colors::Lime,6);
	}
	else 
	{
		for (size_t i = 0; i < finderPatterns.size(); i++)
		{
			FinderPattern *  pattern = finderPatterns.at(i);
			if (pattern == NULL)
				LOGW(LOGTAG_QR,"Null pattern(%d)!",i);

			//Use the pattern size to estimate a rectangle to draw
			float size = pattern->size / 2.0f;

			//Create an array to pass to the polyline function
			Point2i points[4];
			points[0].x = (int) pattern->pt.x - size;
			points[0].y = (int) pattern->pt.y - size;

			points[1].x = (int) pattern->pt.x + size;
			points[1].y = (int) pattern->pt.y - size;

			points[2].x = (int) pattern->pt.x + size;
			points[2].y = (int) pattern->pt.y + size;

			points[3].x = (int) pattern->pt.x - size;
			points[3].y = (int) pattern->pt.y + size;

			int npts = 4;
			const Point2i * pArray[] = {points};

			polylines(*rgbaImage,pArray,&npts,1,true,Colors::Blue,4);

		}
	}
}

int QRCode::FPDistance(FinderPattern * a,  FinderPattern * b)
{
	long xd = a->pt.x - b->pt.x;
	long yd = a->pt.y - b->pt.y;

	return (long) sqrt((double) (xd * xd + yd * yd));
}

int QRCode::FPAcuteAngleGeometry(FinderPattern* a, FinderPattern* b, FinderPattern* c)
{
	return (b->pt.x - a->pt.x) * (c->pt.y - a->pt.y) - (b->pt.y - a->pt.y) * (c->pt.x - a->pt.x);
}

//Use cross product to arrange finder patterns into correct order
QRCode * QRCode::CreateFromFinderPatterns(vector<FinderPattern*> & finderPatterns)
{
	FinderPattern * bottom_left, * top_left, * top_right;

	long d0to1 = FPDistance(finderPatterns.at(0), finderPatterns.at(1));
	long d1to2 = FPDistance(finderPatterns.at(1), finderPatterns.at(2));
	long d0to2 = FPDistance(finderPatterns.at(0), finderPatterns.at(2));

	if (d1to2 > d0to1 && d1to2 > d0to2)
	{
		top_left = finderPatterns.at(0);
		bottom_left = finderPatterns.at(1);
		top_right = finderPatterns.at(2);
	} else if (d0to1 > d1to2 && d0to1 > d0to2)
	{
		top_left = finderPatterns.at(2);
		bottom_left = finderPatterns.at(1);
		top_right = finderPatterns.at(0);
	}  else if (d0to2 > d1to2 && d0to2 > d0to1)
	{
		top_left = finderPatterns.at(1);
		bottom_left = finderPatterns.at(0);
		top_right = finderPatterns.at(2);
	}
	else
	{
		LOGW("QRFinder","Unable to resolve finder pattern order by distance");
		//no clear winner, probably some significant perspective. 
		if (FPAcuteAngleGeometry(finderPatterns.at(0), finderPatterns.at(1), finderPatterns.at(2)) >= 0)
		{
			top_left = finderPatterns.at(0);
			top_right = finderPatterns.at(1);
			bottom_left = finderPatterns.at(2);
		}
		else if (FPAcuteAngleGeometry(finderPatterns.at(1), finderPatterns.at(0), finderPatterns.at(2)) >= 0)
		{
			top_left = finderPatterns.at(1);
			top_right = finderPatterns.at(0);
			bottom_left = finderPatterns.at(2);
		}
		else if (FPAcuteAngleGeometry(finderPatterns.at(2), finderPatterns.at(0), finderPatterns.at(1)) >= 0)
		{
			top_left = finderPatterns.at(2);
			top_right = finderPatterns.at(0);
			bottom_left = finderPatterns.at(1);
		}
		else
		{
			LOGE("Error determining finder pattern order. This shouldn't happen.");
			top_left = finderPatterns.at(0);
			bottom_left = finderPatterns.at(1);
			top_right = finderPatterns.at(2);
		}
	}

	if (FPAcuteAngleGeometry(bottom_left, top_left, top_right) < 0)
	{
		FinderPattern * t = bottom_left;
		bottom_left = top_right;
		top_right = t;
	}

	vector<FinderPattern*> patternVector;
	patternVector.push_back(top_left);
	patternVector.push_back(top_right);
	patternVector.push_back(bottom_left);


	int fpSize = (int)round(top_left->size/2.0f);

	Point2i alignmentGuess;
	alignmentGuess.x = (top_right->pt.x - top_left->pt.x) + bottom_left->pt.x - fpSize;
	alignmentGuess.y = (top_right->pt.y - top_left->pt.y) + bottom_left->pt.y - fpSize;								

	return new QRCode(patternVector, true,alignmentGuess);
}


