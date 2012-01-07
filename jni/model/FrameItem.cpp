#include <model/FrameItem.hpp>

void FrameItem::setPreviousFrame(FrameItem * frame)
{
	lastFrame = frame;
}


//Empty lists/vectors
void FrameItem::clearOldData()
{
	while (!finderPatterns.empty())
	{
		delete finderPatterns.back();
		finderPatterns.pop_back();
	}
	ratioMatches.clear();

}

FrameItem::FrameItem()
{
	rgbImage = new Mat();
	grayImage = new Mat();
	binaryImage = new Mat();
}

FrameItem::~FrameItem()
{
	delete rgbImage;
	delete grayImage;
	delete binaryImage;

	while (!finderPatterns.empty())
	{
		delete finderPatterns.back();
		finderPatterns.pop_back();
	}
}