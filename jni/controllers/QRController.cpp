#include "controllers/QRController.hpp"

QRController::QRController()
{
	LOGD(QRCONTROLLER_LOGTAG,"QRController Initialized");
}

void QRController::drawDebugOverlay(FrameItem * item)
{
	if (item->foundQRCodes)
	{
		Point_<int> * pointArray = item->finderPatterns[0];
		int numPoints = 4;
		fillConvexPoly(*(item->rgbImage),item->finderPatterns[0], 4, Scalar(0, 255, 0, 255));
	}
	else
	{
		for (size_t i = 0; i < item->finderPatterns.size(); i++)
		{
			int npts = 4;
			const Point_<int> * pArray[] = {item->finderPatterns[i]};
			polylines(*(item->rgbImage),pArray,&npts,1,true,Scalar(0,0,255,255),4);
		}
	}
	for (size_t i = 0; i < item->ratioMatches.size(); i++)
	{
		circle(*(item->rgbImage), Point2i((item->ratioMatches)[i].x, (item->ratioMatches)[i].y), (item->ratioMatches)[i].z, Scalar(255, 0, 0, 255), 1);
	}
}

void QRController::getImages(Engine * engine, FrameItem * item)
{
	struct timespec start, end;	

	//Retrieve image from the camera
	engine->imageCollector->newFrame();
	SET_TIME(&start);
	engine->imageCollector->getImage(&(item->rgbImage), ImageCollector::RGBA);
	engine->imageCollector->getImage(&(item->grayImage), ImageCollector::GRAY);	
	SET_TIME(&end);
	LOG_TIME("Image Capture", start, end);

	//Threshold captured image
	ImageProcessor::SimpleThreshold(item);

	
	//If the draw mode is not RGB, then copy the appropriate image to the RGB image
	if (engine->drawMode == Configuration::GrayImage)
	{
		SET_TIME(&start)
		cvtColor(*(item->grayImage), *(item->rgbImage), CV_GRAY2RGBA, 4);
		SET_TIME(&end);
		LOG_TIME("Gray->RGBA", start, end);
	} else if (engine->drawMode == Configuration::BinaryImage)
	{
		SET_TIME(&start)
		cvtColor(*(item->binaryImage), *(item->rgbImage), CV_GRAY2RGBA, 4);
		SET_TIME(&end);
		LOG_TIME("Binary->RGBA", start, end);
	}

}


void QRController::ProcessFrame(Engine * engine, FrameItem * item)
{
	LOGD(QRCONTROLLER_LOGTAG,"Processing frame");
	getImages(engine,item);
	bool wasFound = QRFinder::locateQRCode(*(item->binaryImage), item->finderPatterns, item->ratioMatches);
	drawDebugOverlay(item);
}