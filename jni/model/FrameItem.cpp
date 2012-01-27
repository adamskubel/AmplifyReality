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
	qrCode = NULL;
}

FrameItem::~FrameItem()
{
	clearOldData();
}