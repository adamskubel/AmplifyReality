#include "QRCode.hpp"




QRCode::QRCode(vector<FinderPattern*> _finderPatterns) 
{ 
	QRCode::instanceCount++;
	finderPatterns = _finderPatterns;
	trackingCorners.clear();
}

QRCode::~QRCode() 
{		
	QRCode::instanceCount--;
	while (!finderPatterns.empty())
	{
		delete finderPatterns.back();
		finderPatterns.pop_back();
	}	
}

static Point2i ExtendLine(Point2i pt0, Point2i pt1, float scale)
{
	Point2f diff = pt1 - pt0;
	Point2f floatEnd = Point2f(pt0.x,pt0.y) + (diff * scale);
	return Point2i((int)floatEnd.x,(int)floatEnd.y);
}

void QRCode::SetAlignmentCorners(vector<Point2i> & alignmentCorners)
{
	std::sort(alignmentCorners.begin(),alignmentCorners.end(),PointCompare(codeCenter));
	alignmentPattern = alignmentCorners.back();
	LOGD(LOGTAG_QR,"Selected alignment pattern (%d,%d)",alignmentPattern.x,alignmentPattern.y);
}

float QRCode::getAvgPatternSize()
{
	float avgPatternSize = 0;
	for (int i=0;i<finderPatterns.size();i++)
	{
		avgPatternSize += finderPatterns[i]->size;
	}
	avgPatternSize = idiv(avgPatternSize,finderPatterns.size());
	return avgPatternSize;
}

bool QRCode::GuessAlignmentPosition(Point2i & result, Rect & searchArea)
{
	if (finderPatterns.size() == 3)
	{	
		
		
		if (finderPatterns[1]->patternCorners.size() != 4 || finderPatterns[2]->patternCorners.size() != 4)
		{			
			int fpSize = (int)round(finderPatterns[0]->size/2.0f);
			result.x = (finderPatterns[1]->pt.x - finderPatterns[0]->pt.x) + finderPatterns[2]->pt.x - fpSize;
			result.y = (finderPatterns[1]->pt.y - finderPatterns[0]->pt.y) + finderPatterns[2]->pt.y - fpSize;		
			alignmentPattern = result;
			
			int startX = result.x - fpSize, endX = result.x + fpSize;
			int startY = result.y- fpSize, endY = result.y + fpSize;

			searchArea = Rect(Point2i(startX,startY),Point2i(endX,endY));
			
			return false;
		}
		else
		{
			sortCorners();
			for (int i=0;i<finderPatterns.size();i++)
			{
				finderPatterns[i]->SortCorners();
			}
			
			//Get AP-aligning corners
			Point2i upperRight0 = finderPatterns[1]->patternCorners[3];
			Point2i upperRight1 = finderPatterns[1]->patternCorners[2];

			Point2i lowerLeft0 = finderPatterns[2]->patternCorners[1];
			Point2i lowerLeft1 = finderPatterns[2]->patternCorners[2];

			LOGV(LOGTAG_QR,"Calculating AP position");

			Point2i closePosition1 = ExtendLine(upperRight0,upperRight1,3.0f);
			//Point2i closePosition2 = ExtendLine(lowerLeft0,lowerLeft0,2);

			//Point2i farPosition1 = ExtendLine(upperRight0,upperRight1,2.43f);
			Point2i farPosition2 = ExtendLine(lowerLeft0,lowerLeft0,3.43f);


			//Abitrary choosing points
			result = farPosition2;		
			alignmentPattern = result;
			searchArea = Rect(closePosition1,farPosition2);
			return true;
		}
	}
	else
	{
		LOGW(LOGTAG_QR,"Not enough finder patterns to guess alignment pattern. Size=%d",finderPatterns.size());
		return false;
	}
}


void QRCode::sortCorners()
{
	LOGD(LOGTAG_QR,"Sorting corners for %d patterns",finderPatterns.size());
	//Top left, top right, bottom left
	codeCenter = Point2i(0,0);
	trackingCorners.clear();
	for (int i=0;i<finderPatterns.size();i++)
	{
		codeCenter += finderPatterns[i]->pt;
	}
	codeCenter = Point2i(codeCenter.x/(float)finderPatterns.size(),codeCenter.y/(float)finderPatterns.size());

	for (int i=0;i<finderPatterns.size();i++)
	{
		trackingCorners.push_back(finderPatterns[i]->getFarthestCorner(codeCenter));
	}
}

bool QRCode::isValidCode()
{
	return finderPatterns.size() == 3 && alignmentPattern.x != 0 && alignmentPattern.y != 0;
}

bool QRCode::isDecoded()
{
	return isValidCode() && TextValue.length() > 0;
}

void QRCode::getTrackingPoints(vector<cv::Point2f> & points)
{
	if (trackingCorners.size() < 3)
		sortCorners();

	points.push_back(Point2f(trackingCorners.at(0).x,trackingCorners.at(0).y));
	points.push_back(Point2f(trackingCorners.at(1).x,trackingCorners.at(1).y));
	points.push_back(Point2f(alignmentPattern.x,alignmentPattern.y));
	points.push_back(Point2f(trackingCorners.at(2).x,trackingCorners.at(2).y));	
}

void QRCode::Draw(Mat * rgbaImage)
{
	if (isValidCode())
	{
		if (isDecoded())
		{
			DebugLabel(finderPatterns[1]->pt,TextValue,Colors::Black,2.0f,Colors::Beige).Draw(rgbaImage);
		}
		//if (trackingCorners.size() != 3)
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
			polylines(*rgbaImage,pArray,&npts,1,true,Colors::MediumTurquoise,2);
		}
		/*else
		{
			vector<Point2i> drawPoints;
			drawPoints.push_back(Point2f(trackingCorners.at(0).x,trackingCorners.at(0).y));
			drawPoints.push_back(Point2f(trackingCorners.at(1).x,trackingCorners.at(1).y));
			drawPoints.push_back(Point2f(alignmentPattern.x,alignmentPattern.y));
			drawPoints.push_back(Point2f(trackingCorners.at(2).x,trackingCorners.at(2).y));	
			DebugPoly(drawPoints,Colors::Olive,2).Draw(rgbaImage);
		}*/
	}
	else 
	{
		for (size_t i = 0; i < finderPatterns.size(); i++)
		{
			FinderPattern *  pattern = finderPatterns.at(i);

			/*if (pattern->patternCorners.size() > 0)
			{
				DebugPoly(pattern->patternCorners,Colors::Orchid,2).Draw(rgbaImage);
			}
			else*/
			{
				Point2i halfSize(idiv(pattern->patternSize.width,2), idiv(pattern->patternSize.height,2));
				DebugRectangle(Rect(pattern->pt - halfSize,pattern->pt + halfSize),Colors::Purple,2).Draw(rgbaImage);
			}
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


						

	return new QRCode(patternVector);
}


