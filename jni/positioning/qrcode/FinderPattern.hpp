#ifndef FINDER_PATTERN_HPP_
#define FINDER_PATTERN_HPP_

#include <opencv2/core/core.hpp>
#include <vector>
#include "LogDefinitions.h"
#include "util/GeometryUtil.hpp"

using namespace cv;

struct FinderPattern
{
public:	
	static int instanceCount;

	Point2i pt;
	int size;
	Size2i patternSize;
	vector<Point2i> patternCorners;

	FinderPattern(Point2i patternCenter, Size2i patternSize);
	~FinderPattern();
	FinderPattern(FinderPattern & copy);
	
	Point2i getFarthestCorner(Point2i point);	
};

#endif
