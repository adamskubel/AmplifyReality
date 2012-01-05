#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include <opencv2/core/core.hpp>
#include "Engine.hpp"

#ifndef FRAME_ITEM_HPP_
#define FRAME_ITEM_HPP_



class FrameItem
{
	
public:
	cv::Mat *rgbImage;
	cv::Mat *grayImage;
	cv::Mat *binaryImage;
	std::vector<Point_<int>* > finderPatterns;	
	std::vector<Point3i> ratioMatches;
	Configuration::DrawMode drawMode;
	bool foundQRCodes;

	void setPreviousFrame(FrameItem * frame);
	~FrameItem();


private:
	FrameItem * lastFrame;


};

#endif
