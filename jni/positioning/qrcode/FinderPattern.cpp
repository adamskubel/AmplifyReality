#include "FinderPattern.hpp"


FinderPattern::FinderPattern(Point2i _patternCenter, Size2i _patternSize)
{
	FinderPattern::instanceCount++;
	pt = _patternCenter;
	patternSize = _patternSize;
	size = MAX(patternSize.width,patternSize.height);
}

Point2i FinderPattern::getFarthestCorner(Point2i point)
{
	if (patternCorners.size() == 0)
	{
		return pt;
	}

	LOGV(LOGTAG_QR,"Sorting FP points by distance from (%d,%d)",point.x,point.y);
	std::sort(patternCorners.begin(),patternCorners.end(),PointCompare(point));
	Point2i outerPoint = patternCorners.front();
	LOGV(LOGTAG_QR,"Farthest point is (%d,%d)",outerPoint.x,outerPoint.y);
	return outerPoint;
}

//Clockwise starting at farthest (patternCorners.front())
void FinderPattern::SortCorners()
{
	if (patternCorners.size() == 4)
	{
		LOGD(LOGTAG_QR,"Sorting FP corners");
		Point2i tmp;

		//first point in vector is farthest corner
		vector<Point2i> tmpCorners;
		tmpCorners.push_back(patternCorners[1]);
		tmpCorners.push_back(patternCorners[2]);
		tmpCorners.push_back(patternCorners[3]);

		std::sort(tmpCorners.begin(),tmpCorners.end(),PointCompare(patternCorners.front()));
		//farthest is first in tmpCorners
		//sort out middle two
		if (!IsClockWise(tmpCorners.front(),patternCorners.front(), tmpCorners[1]))
		{
			//If not clockwise, swap
			patternCorners[1] = tmpCorners[2];
			patternCorners[2] = tmpCorners[0];
			patternCorners[3] = tmpCorners[1];
		}
		else
		{
			patternCorners[1] = tmpCorners[1];
			patternCorners[2] = tmpCorners[0];
			patternCorners[3] = tmpCorners[2];		
		}
		
		LOGD(LOGTAG_QR,"Finished sorting corners");
	}
}

FinderPattern::~FinderPattern()
{	   
	FinderPattern::instanceCount--;
}

FinderPattern::FinderPattern(FinderPattern & copy)
{
	FinderPattern::instanceCount++;
	pt = copy.pt;
	patternSize = copy.patternSize;
}