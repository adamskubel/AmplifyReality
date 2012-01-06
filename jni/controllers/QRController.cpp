#include "controllers/QRController.hpp"

QRController::QRController()
{
	LOGD(LOGTAG_QRCONTROLLER,"QRController Initialized");
}

void QRController::drawDebugOverlay(FrameItem * item)
{
	struct timespec start, end;
	SET_TIME(&start);
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
	SET_TIME(&end);
	LOG_TIME("Debug Draw", start, end);
}

void QRController::getImages(Engine * engine, FrameItem * item)
{
	struct timespec start, end;
	
	//Retrieve image from the camera	
	if (engine->drawMode == Configuration::ColorImage)
	{		
		engine->imageCollector->newFrame();
		engine->imageCollector->getCameraImages(*(item->rgbImage), *(item->grayImage));
	} 
	else if (engine->drawMode == Configuration::GrayImage || engine->drawMode == Configuration::BinaryImage)
	{
		SET_TIME(&start);
		engine->imageCollector->newFrame();
		engine->imageCollector->getGrayCameraImage(*(item->grayImage));
		SET_TIME(&end);
		LOG_TIME("Image Capture", start, end);

		//Copy gray image to RGB image to be used by the following stages (rendering, debug overlay, etc)
		SET_TIME(&start)
		cvtColor(*(item->grayImage), *(item->rgbImage), CV_GRAY2RGBA, 4);
		SET_TIME(&end);
		LOG_TIME("Gray->RGBA", start, end);
	} 
	
	//Threshold captured image
	ImageProcessor::SimpleThreshold(item);
	
	if (engine->drawMode == Configuration::BinaryImage)
	{
		SET_TIME(&start)
		cvtColor(*(item->binaryImage), *(item->rgbImage), CV_GRAY2RGBA, 4);
		SET_TIME(&end);
		LOG_TIME("Binary->RGBA", start, end);
	}
	SET_TIME(&end);
}


void QRController::ProcessFrame(Engine * engine, FrameItem * item)
{
	LOGD(LOGTAG_QRCONTROLLER,"Processing frame");
	getImages(engine,item);
	item->foundQRCodes = QRFinder::locateQRCode(*(item->binaryImage), item->finderPatterns, item->ratioMatches);
	drawDebugOverlay(item);
}