#include "ImageCollector.hpp"

//#define LOG_TAG "AmplifyR-ImageCollector"

void ImageCollector::newFrame()
{
	myCapture->grab();
	otsuUpdated = false;
	grayUpdated = false;
	rgbaUpdated = false;
}

ImageCollector::ImageCollector(int width, int height)
{
	this->width = width;
	this->height = height;

	this->grayOptMode = true;


	//Mat myuv(height + height / 2, width, CV_8UC1);
	mrgba = new Mat(height, width, CV_8UC4);
	mgray = new Mat(height, width, CV_8UC1);
	mbin = new Mat(height, width, CV_8UC1);

	char matData[100];
	sprintf(matData, "Initializing VC with width=%d and height=%d", width, height);
	LOGI(matData);

	myCapture = new VideoCapture(CV_CAP_ANDROID);

	double addr = myCapture->get(CV_CAP_PROP_SUPPORTED_PREVIEW_SIZES_STRING);
	char* result = *((char**)&addr);
	LOGI("Allowed preview sizes are: %s",result);

	if (!myCapture->isOpened())
	{
		LOGE("VC Failed to open. Whoops!");
		throw CAMERA_INITIALIZATION_EXCEPTION;
	}

	myCapture->set(CV_CAP_PROP_FRAME_WIDTH, width);
	myCapture->set(CV_CAP_PROP_FRAME_HEIGHT, height);

	LOGD("VC Initialized");

}

void ImageCollector::teardown()
{
	myCapture->release();
}

void ImageCollector::generateImage(ImageType type)
{
	switch (type)
	{
	case (int) RGBA:
		if (rgbaUpdated)
			break;
		LOGD("RGBA Image will be retrieved");
		myCapture->retrieve(*mrgba, CV_CAP_ANDROID_COLOR_FRAME_RGBA);
		LOGD("Got RGBA!");
		rgbaUpdated = true;
		break;
	case (int) GRAY:
		if (grayUpdated)
			break;
		if (rgbaUpdated)
		{
			LOGD("Converting RGBA to GRAY");
			cvtColor(*mrgba, *mgray, CV_RGBA2GRAY, 1);
			grayUpdated = true;
		} else if (grayOptMode)
		{
			myCapture->retrieve(*mgray, CV_CAP_ANDROID_GREY_FRAME);
			grayUpdated = true;
		} else
		{
			generateImage(RGBA);
			generateImage(GRAY);
		}
		break;
	case (int) OTSU:
		if (otsuUpdated)
			break;
		if (grayUpdated)
		{
			LOGD("Thresholding image");
			threshold(*mgray, *mbin, 100, 255, THRESH_OTSU + THRESH_BINARY);
			otsuUpdated = true;
		} else
		{
			LOGD("Generating Gray First");
			generateImage(GRAY);
			generateImage(OTSU);
		}
		break;
	};
}

void ImageCollector::getImage(Mat ** outputMat, ImageType type)
{
	switch (type)
	{
	case (int) RGBA:
		if (rgbaUpdated)
		{
			*outputMat = mrgba;
		}
		else
		{
			generateImage(RGBA);
			getImage(outputMat, type);
			LOGD("RGBA Output image set");
		}
		break;
	case (int) GRAY:
		if (grayUpdated)
			*outputMat = mgray;//*outputMat = mgray->clone();
		else
		{
			generateImage(GRAY);
			getImage(outputMat, type);
		}
		break;
	case (int) OTSU:
		if (otsuUpdated)
			*outputMat = mbin;
			//*outputMat = mbin->clone();
		else
		{
			generateImage(OTSU);
			getImage(outputMat, type);
		}
		break;
	};
}

