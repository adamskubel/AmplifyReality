#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include <opencv2/core/core.hpp>
#include "positioning/QRCode.hpp"


#ifndef FRAME_ITEM_HPP_
#define FRAME_ITEM_HPP_

using namespace cv;

namespace Configuration
{
	enum DrawMode
	{
		ColorImage = 0, GrayImage = 1, BinaryImage = 2
	};
	static const int DrawModeSize = 3;

	static const DrawMode DefaultDrawMode = GrayImage;
	
}




//Stores the processing inputs and outputs for a given frame.
//Is designed to be reused indefinitely. This saves the need to reallocate memory. 
class FrameItem
{
	
public:
	FrameItem();
	cv::Mat *rotationMatrix, *translationMatrix;
	std::vector<Point3i> ratioMatches;
	Configuration::DrawMode drawMode;
	QRCode * qrCode;

	void clearOldData();
	~FrameItem();

	long nanotime;

};

#endif
