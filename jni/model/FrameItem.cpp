#include <model/FrameItem.hpp>


//Empty lists/vectors
void FrameItem::clearOldData()
{
	if (qrCode != NULL)
	{
		delete qrCode;		
		qrCode = NULL;
	}
	nanotime = 0;
}


FrameItem::FrameItem()
{	
	rotationMatrix = new Mat();
	translationMatrix = new Mat();
	gyroRotation = Mat::eye(4,4,CV_64F);
	qrCode = NULL;
}

FrameItem::~FrameItem()
{
	clearOldData();
}