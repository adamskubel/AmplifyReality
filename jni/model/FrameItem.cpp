#include <model/FrameItem.hpp>


//Empty lists/vectors
void FrameItem::clearOldData()
{
	if (qrCode != NULL)
	{
		delete qrCode;		
		qrCode = NULL;
	}
	ratioMatches.clear(); //Is this safe if vector isn't declard??
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