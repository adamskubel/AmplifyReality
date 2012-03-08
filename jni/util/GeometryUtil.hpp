#ifndef GEOMETRY_UTIL_HPP_
#define GEOMETRY_UTIL_HPP_

#include <opencv2/core/core.hpp>


static int ipow(int base, int exp)
{
	int result = 1;
	while (exp)
	{
		if (exp & 1)
			result *= base;
		exp >>= 1;
		base *= base;
	}

	return result;
}

#define idiv(a,b) ((int)round(((float)a)/((float)b)))

static int GetSquaredDistance(cv::Point2i pt0, cv::Point2i pt1)
{
	return ipow(pt0.x-pt1.x,2) + ipow(pt0.y-pt1.y,2);
} 


static int GetSquaredDistance(cv::Point3f pt0, cv::Point3f pt1)
{
	return pow(pt0.x-pt1.x,2) + pow(pt0.y-pt1.y,2) + pow(pt0.z - pt1.z,2);
} 


static bool IsClockWise(cv::Point2f p1, cv::Point2f p2, cv::Point2f p0)
{
	return ((p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x)) > 0;
}

static float GetPointDistance_Slow(cv::Point2f p1, cv::Point2f p2)
{
	return sqrt(pow(abs(p1.x-p2.x),2) + pow(abs(p1.y-p2.y),2));
}

static void ConstrainRectangle(cv::Mat & matrix, cv::Rect & rectangle)
{
	if (rectangle.x < 0)
		rectangle.x = 0;	

	if (rectangle.x >= matrix.cols)
		rectangle.x = 0;

	if ((rectangle.x+rectangle.width) >= matrix.cols)
		rectangle.width = matrix.cols - rectangle.x - 1;
			
	if (rectangle.y < 0)
		rectangle.y = 0;	

	if (rectangle.y >= matrix.rows)
		rectangle.y = 0;

	if ((rectangle.y + rectangle.height) >= matrix.rows)
		rectangle.height = matrix.rows - rectangle.y - 1;

}

class PointCompare
{
public:
	PointCompare(cv::Point2i _center)
	{
		center = _center;
	}
	bool operator()(const cv::Point2i pt0, const cv::Point2i pt1)
	{
		return (ipow(pt0.x-center.x,2) + ipow(pt0.y-center.y,2)) > (ipow(pt1.x-center.x,2) + ipow(pt1.y-center.y,2)) ;
	}
private:
	cv::Point2i center;
};

class PointCompare_Ascending
{
public:
	PointCompare_Ascending(cv::Point2i _center)
	{
		center = _center;
	}
	bool operator()(const cv::Point2i pt0, const cv::Point2i pt1)
	{
		return (ipow(pt0.x-center.x,2) + ipow(pt0.y-center.y,2)) < (ipow(pt1.x-center.x,2) + ipow(pt1.y-center.y,2)) ;
	}
private:
	cv::Point2i center;
};


#endif