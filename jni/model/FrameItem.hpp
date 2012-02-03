#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include <opencv2/core/core.hpp>
#include "positioning/qrcode/QRCode.hpp"


#ifndef FRAME_ITEM_HPP_
#define FRAME_ITEM_HPP_



//Stores the processing inputs and outputs for a given frame.
//Is designed to be reused indefinitely. This saves the need to reallocate memory. 
class FrameItem
{
	
public:
	FrameItem();
	cv::Mat *rotationMatrix, *translationMatrix;
	QRCode * qrCode;

	void clearOldData();
	~FrameItem();

	long nanotime;

};

#endif
