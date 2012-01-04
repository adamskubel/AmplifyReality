#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2\calib3d\calib3d.hpp>

#ifndef FRAME_ITEM_HPP_
#define FRAME_ITEM_HPP_


class FrameItem
{
public:
	Mat *rgbImage,*grayImage,*binaryImage;
	vector<Point_<int>*> finderPatterns;	
	vector<Point3i> ratioMatches;


};

#endif
