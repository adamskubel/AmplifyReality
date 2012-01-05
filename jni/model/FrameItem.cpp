#include <model/FrameItem.hpp>

void FrameItem::setPreviousFrame(FrameItem * frame)
{
	lastFrame = new FrameItem();
	lastFrame->finderPatterns = frame->finderPatterns; 
	delete frame; //probably deletes thing above
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