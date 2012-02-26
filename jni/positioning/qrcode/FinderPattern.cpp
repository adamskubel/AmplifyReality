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
	Point2i pt = patternCorners.back();
	LOGV(LOGTAG_QR,"Farthest point is (%d,%d)",pt.x,pt.y);
	return pt;
}

//Clockwise starting at farthest (patternCorners.back())
void FinderPattern::SortCorners()
{
	if (patternCorners.size() == 4)
	{
		LOGD(LOGTAG_QR,"Sorting FP corners");
		Point2i tmp;

		//sort out middle two
		if (!IsClockWise(patternCorners[1],patternCorners.back(), patternCorners.front()))
		{
			//If not clockwise, swap
			tmp = patternCorners[1];
			patternCorners[1] = patternCorners[3];
			patternCorners[3] = tmp;
		}
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