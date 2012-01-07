#include "ImageCollector.hpp"



void ImageCollector::newFrame()
{
	myCapture->grab();
	//grayUpdated = false;
	//rgbaUpdated = false;
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
	distortionMatrix = NULL;
	cameraMatrix = NULL;

	LOGI(LOGTAG_IMAGECAPTURE,"Initializing VideoCapture with width=%d and height=%d", width, height);

	myCapture = new VideoCapture(CV_CAP_ANDROID);
	
	double addr = myCapture->get(CV_CAP_PROP_SUPPORTED_PREVIEW_SIZES_STRING);

	char* result = *((char**)&addr);
	LOGI(LOGTAG_IMAGECAPTURE,"Allowed preview sizes are: %s",result);

	if (!myCapture->isOpened())
	{
		LOGE(LOGTAG_IMAGECAPTURE,"VideoCapture Failed to open. Whoops!");
		throw CAMERA_INITIALIZATION_EXCEPTION;
	}

	myCapture->set(CV_CAP_PROP_FRAME_WIDTH, width);
	myCapture->set(CV_CAP_PROP_FRAME_HEIGHT, height);

	LOGI(LOGTAG_IMAGECAPTURE,"VideoCapture Initialized");

}

void ImageCollector::undistortImage(Mat* inputImage, Mat* outputImage)
{	
	try
	{			
		struct timespec start, end; 
		SET_TIME(&start);
		undistort(*inputImage,*outputImage,*cameraMatrix,*distortionMatrix);
		SET_TIME(&end);
		LOG_TIME("Undistort",start,end);
		LOGD(LOGTAG_IMAGECAPTURE,"Undistort complete");
	}
	catch (std::exception& e)
	{
		LOGE(LOGTAG_IMAGECAPTURE,"UndistortImage: %s", e.what());
	}
}

void ImageCollector::getCameraImages(Mat & rgbImage, Mat & grayImage)
{
	struct timespec start,end;
	SET_TIME(&start);
	myCapture->retrieve(rgbImage, CV_CAP_ANDROID_COLOR_FRAME_RGBA);
	cvtColor(rgbImage, grayImage, CV_RGBA2GRAY, 1);
	SET_TIME(&end);
	LOG_TIME("Image Capture", start, end);
}

//Get gray image only
void ImageCollector::getGrayCameraImage(Mat & grayImage)
{
	myCapture->retrieve(grayImage, CV_CAP_ANDROID_GREY_FRAME);
}
void ImageCollector::teardown()
{
	myCapture->release();
}



//void ImageCollector::setCorrectionMatrices(Mat* _cameraMatrix, Mat* _distortionMatrix)
//{
//	cameraMatrix = new Mat();
//	_cameraMatrix->copyTo(*cameraMatrix);
//	distortionMatrix = new Mat();
//	_distortionMatrix->copyTo(*distortionMatrix);
//}
//

//bool ImageCollector::canUndistort()
//{	
//	if (ENABLE_UNDISTORT)
//	{
//		if (distortionMatrix == NULL || cameraMatrix == NULL) 
//		{
//			LOGD(LOGTAG_IMAGECAPTURE,"Cannot undistort");
//			return false;
//		}
//			LOGD(LOGTAG_IMAGECAPTURE,"Can undistort!");
//		return true;
//	}
//	return false;
//}
//
//
//void ImageCollector::generateImage(ImageType type)
//{	
//	switch (type)
//	{
//	case (int) RGBA:
//		if (rgbaUpdated)
//			break;
//		LOGD(LOGTAG_IMAGECAPTURE,"RGBA Image will be retrieved");
//		if (canUndistort())
//		{
//			Mat * tmp = new Mat(height, width, CV_8UC4);
//			mrgba = new Mat(height, width, CV_8UC4);
//			myCapture->retrieve(*tmp, CV_CAP_ANDROID_COLOR_FRAME_RGBA);
//			undistortImage(tmp,mrgba);
//		}
//		else
//		{
//			myCapture->retrieve(*mrgba, CV_CAP_ANDROID_COLOR_FRAME_RGBA);
//		}
//
//		LOGD(LOGTAG_IMAGECAPTURE,"Got RGBA!");
//		rgbaUpdated = true;
//		break;
//	case (int) GRAY:
//		if (grayUpdated)
//			break;
//		if (rgbaUpdated)
//		{
//			LOGD(LOGTAG_IMAGECAPTURE,"Converting RGBA to GRAY");
//			cvtColor(*mrgba, *mgray, CV_RGBA2GRAY, 1);
//			grayUpdated = true;
//			LOGD(LOGTAG_IMAGECAPTURE,"Complete!");
//		} else if (grayOptMode)
//		{
//			if (canUndistort())
//			{
//				Mat * tmp = new Mat(height, width, CV_8UC1);
//				mgray = new Mat(height, width, CV_8UC1);
//				myCapture->retrieve(*tmp, CV_CAP_ANDROID_GREY_FRAME);
//				undistortImage(tmp,mgray);
//			}
//			else
//			{
//				myCapture->retrieve(*mgray, CV_CAP_ANDROID_GREY_FRAME);
//			}
//			grayUpdated = true;
//		} else
//		{
//			generateImage(RGBA);
//			generateImage(GRAY);
//		}
//		break;
//	};
//}
//
//void ImageCollector::getImage(Mat ** outputMat, ImageType type)
//{
//	switch (type)
//	{
//	case (int) RGBA:
//		if (rgbaUpdated)
//		{
//			*outputMat = mrgba;
//		}
//		else
//		{
//			generateImage(RGBA);
//			getImage(outputMat, type);
//			LOGD(LOGTAG_IMAGECAPTURE,"RGBA Output image set");
//		}
//		break;
//	case (int) GRAY:
//		if (grayUpdated)
//			*outputMat = mgray;
//		else
//		{
//			generateImage(GRAY);
//			getImage(outputMat, type);
//		}
//		break;
//	};
//}



//
//void ImageCollector::qrBinarization(Mat * inputImg, Mat * outputImg)
//{
//	try
//	{
//		Mat imHist;
//	
//		float range[] = { 0, 255 } ;
//		const float* histRange = { range };
//		int histSize = 255;
//		int channels[] = {0, 1};
//
//		calcHist(inputImg,1,NULL,Mat(),imHist,1,&histSize,&histRange,true,false);
//	
//		double maxVal=0;
//		minMaxLoc(imHist, 0, &maxVal, 0, 0);
//
//		float wScale = 1, maxHeight = 300;
//
//		for( int h = 0; h < histSize; h++ )
//		{
//			int intensity = round(maxHeight*(imHist.at<float>(h)/maxVal));
//			rectangle(*inputImg, Point(h*wScale, 0), Point((h+1)*wScale - 1, intensity+10),Scalar(255),CV_FILLED);
//		}
//	}
//	catch (std::exception &e)
//	{
//		LOGE(LOGTAG_IMAGECAPTURE,e.what());
//	}
//}