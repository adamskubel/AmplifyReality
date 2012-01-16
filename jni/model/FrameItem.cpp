#include <model/FrameItem.hpp>

void FrameItem::setPreviousFrame(FrameItem * frame)
{
	lastFrame = frame;
}


//Empty lists/vectors
void FrameItem::clearOldData()
{
	if (qrCode != NULL)
	{
		delete qrCode;		
		qrCode = NULL;
	}
	ratioMatches.clear(); //Is this safe if vector isn't declard??
}

vector<FrameItem*> FrameItem::getLastFrames()
{
	vector<FrameItem *> frameVector = vector<FrameItem *>();

	FrameItem * lastFramePointer = lastFrame;
	while (lastFramePointer != NULL && lastFramePointer != this)
	{
		frameVector.push_back(lastFramePointer);
		lastFramePointer = lastFramePointer->lastFrame;
	}
	return frameVector;
}

FrameItem::FrameItem()
{
	rgbImage = new Mat();
	grayImage = new Mat();
	binaryImage = new Mat();
	rotationMatrix = new Mat();
	translationMatrix = new Mat();
	qrCode = NULL;
	lastFrame = NULL;
}

FrameItem::~FrameItem()
{
	clearOldData();
	delete rgbImage;
	delete grayImage;
	delete binaryImage;
}