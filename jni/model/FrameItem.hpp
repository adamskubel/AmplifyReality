#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include <opencv2/core/core.hpp>
#include "Engine.hpp"

#ifndef FRAME_ITEM_HPP_
#define FRAME_ITEM_HPP_


//Stores the processing inputs and outputs for a given frame.
//Is designed to be reused indefinitely. This saves the need to reallocate memory. 
class FrameItem
{
	
public:
	FrameItem();
	cv::Mat *rgbImage;
	cv::Mat *grayImage;
	cv::Mat *binaryImage;
	std::vector<Point_<int>* > finderPatterns;	
	std::vector<Point3i> ratioMatches;
	Configuration::DrawMode drawMode;
	bool foundQRCodes;

	void setPreviousFrame(FrameItem * frame);
	void clearOldData();
	~FrameItem();


private:
	FrameItem * lastFrame;


};

#endif
