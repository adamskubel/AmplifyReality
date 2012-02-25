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
	std::sort(patternCorners.begin(),patternCorners.end(),PointCompare(point));
	return *(--patternCorners.end());
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