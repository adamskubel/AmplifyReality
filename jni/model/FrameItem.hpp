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
}

namespace PositioningMethods
{
	enum PositioningMethod
	{
		QRCode, MotionSensors, Momentum
	};

}

/*
- How was the position determined?
- What was the position?
- What is the estimated accuracy of the position?
*/
class PositioningResults
{
public:
	PositioningMethods::PositioningMethod positioningMethod;
	cv::Mat Rotation, Position;
	cv::Mat RotationError, PositionError;
};

//Stores the processing inputs and outputs for a given frame.
//Is designed to be reused indefinitely. This saves the need to reallocate memory. 
class FrameItem
{
	
public:
	FrameItem();
	cv::Mat *rgbImage, *binaryImage, *grayImage;
	cv::Mat *rotationMatrix, *translationMatrix;
	std::vector<Point3i> ratioMatches;
	Configuration::DrawMode drawMode;
	QRCode * qrCode;

	void setPreviousFrame(FrameItem * frame);
	vector<FrameItem*> getLastFrames();
	void clearOldData();
	~FrameItem();

	long nanoTimeStamp;

	PositioningResults * positioningResults;

private:
	FrameItem * lastFrame;


};

#endif
