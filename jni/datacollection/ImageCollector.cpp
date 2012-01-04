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
	distortionMatrix = NULL;
	cameraMatrix = NULL;

	LOGI("ImageCollector","Initializing VC with width=%d and height=%d", width, height);

	myCapture = new VideoCapture(CV_CAP_ANDROID);

	double addr = myCapture->get(CV_CAP_PROP_SUPPORTED_PREVIEW_SIZES_STRING);
	char* result = *((char**)&addr);
	LOGI("ImageCollector","Allowed preview sizes are: %s",result);

	if (!myCapture->isOpened())
	{
		LOGE("ImageCollector","VC Failed to open. Whoops!");
		throw CAMERA_INITIALIZATION_EXCEPTION;
	}

	myCapture->set(CV_CAP_PROP_FRAME_WIDTH, width);
	myCapture->set(CV_CAP_PROP_FRAME_HEIGHT, height);

	LOGI("ImageCollector","VC Initialized");

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
		LOGD("ImageCollector","Undistort complete");
	}
	catch (std::exception& e)
	{
		LOGE("ImageCollector","UndistortImage: %s", e.what());
	}
}


void ImageCollector::setCorrectionMatrices(Mat* _cameraMatrix, Mat* _distortionMatrix)
{
	cameraMatrix = new Mat();
	_cameraMatrix->copyTo(*cameraMatrix);
	distortionMatrix = new Mat();
	_distortionMatrix->copyTo(*distortionMatrix);
}

void ImageCollector::teardown()
{
	myCapture->release();
}

bool ImageCollector::canUndistort()
{	
	if (ENABLE_UNDISTORT)
	{
		if (distortionMatrix == NULL || cameraMatrix == NULL) 
		{
			LOGD("ImageCollector","Cannot undistort");
			return false;
		}
			LOGD("ImageCollector","Can undistort!");
		return true;
	}
	return false;
}

void ImageCollector::generateImage(ImageType type)
{
	switch (type)
	{
	case (int) RGBA:
		if (rgbaUpdated)
			break;
		LOGD("ImageCollector","RGBA Image will be retrieved");
		if (canUndistort())
		{
			Mat * tmp = new Mat(height, width, CV_8UC4);
			mrgba = new Mat(height, width, CV_8UC4);
			myCapture->retrieve(*tmp, CV_CAP_ANDROID_COLOR_FRAME_RGBA);
			undistortImage(tmp,mrgba);
		}
		else
		{
			myCapture->retrieve(*mrgba, CV_CAP_ANDROID_COLOR_FRAME_RGBA);
		}

		LOGD("ImageCollector","Got RGBA!");
		rgbaUpdated = true;
		break;
	case (int) GRAY:
		if (grayUpdated)
			break;
		if (rgbaUpdated)
		{
			LOGD("ImageCollector","Converting RGBA to GRAY");
			cvtColor(*mrgba, *mgray, CV_RGBA2GRAY, 1);
			grayUpdated = true;
		} else if (grayOptMode)
		{
			if (canUndistort())
			{
				Mat * tmp = new Mat(height, width, CV_8UC1);
				mgray = new Mat(height, width, CV_8UC1);
				myCapture->retrieve(*tmp, CV_CAP_ANDROID_GREY_FRAME);
				undistortImage(tmp,mgray);
			}
			else
			{
				myCapture->retrieve(*mgray, CV_CAP_ANDROID_GREY_FRAME);
			}
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
			if (USE_ADAPTIVE_THRESH)
			{	
				LOGD("ImageCollector","Thresholding image in adaptive mode");
				adaptiveThreshold(*mgray,*mbin,255,ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY,5,0);
			}
			else
			{
				LOGD("ImageCollector","Thresholding image");
				if (USE_LOCAL_THRESH)
				{
					localThresholding(*mgray,*mbin,400,240);
				}
				else
				{
					threshold(*mgray, *mbin, 100, 255, THRESH_OTSU + THRESH_BINARY);
				}
				//		qrBinarization(mgray,new Mat());
			}
			otsuUpdated = true;
		} else
		{
			LOGD("ImageCollector","Generating Gray First");
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
			LOGD("ImageCollector","RGBA Output image set");
		}
		break;
	case (int) GRAY:
		if (grayUpdated)
			*outputMat = mgray;
		else
		{
			generateImage(GRAY);
			getImage(outputMat, type);
		}
		break;
	case (int) OTSU:
		if (otsuUpdated)
			*outputMat = mbin;
		else
		{
			generateImage(OTSU);
			getImage(outputMat, type);
		}
		break;
	};
}

void ImageCollector::localThresholding(Mat &inputImg, Mat &outputImg, int windowWidth, int windowHeight)
{	
	try
	{		
		int xCount = floor(inputImg.cols/windowWidth);
		int yCount = floor(inputImg.rows/windowHeight);

		LOGD("ImageCollector","xCount=%d,yCount=%d,rows=%d,cols=%d,step0=%d,step1=%d",xCount,yCount,inputImg.rows,inputImg.cols,inputImg.step[0],inputImg.step[1]);

		outputImg = Mat(inputImg.rows,inputImg.cols,inputImg.type());	

		for (int i=0;i<inputImg.rows;i++)
		{
			const unsigned char* Mi = inputImg.ptr<unsigned char>(i);			
			unsigned char * copyTo = outputImg.ptr<unsigned char>(i);
			memcpy(copyTo,Mi,inputImg.step[0]);
		}

		for (int yOffset=0; yOffset < yCount*windowHeight; yOffset += windowHeight)
		{
			for (int xOffset=0; xOffset < xCount*windowWidth; xOffset += windowWidth)
			{		
				Mat tMat = Mat(windowHeight,windowWidth,inputImg.type());
				Mat tMatThresholded = Mat(windowHeight,windowWidth,inputImg.type());
				for (int i=0; i < windowHeight;i++)
				{
					const unsigned char* Mi = inputImg.ptr<unsigned char>(i+yOffset);
					Mi = Mi + xOffset*inputImg.step[1];
					unsigned char * copyTo = tMat.ptr<unsigned char>(i);
					
					memcpy(tMat.ptr<unsigned char>(i),Mi,windowWidth);
				}
				LOGD("ImageCollector","Local Threshold: xOffset=%d, yOffset=%d",xOffset,yOffset);
				threshold(tMat, tMatThresholded, 100, 255, THRESH_OTSU + THRESH_BINARY);
				for (int i=0; i< windowHeight;i++)
				{
					const unsigned char* Mi = tMatThresholded.ptr<unsigned char>(i);
					unsigned char* copyTo = outputImg.ptr<unsigned char>(i+yOffset);

					copyTo = copyTo + xOffset*inputImg.step[1];

					memcpy(copyTo,Mi,windowWidth);
				}
			}
		}

	}
	catch (std::exception &e)
	{
		LOGE("ImageCollector","LocalThresholding Exception: %s",e.what());
	}
	LOGD("ImageCollector","Local thresholding complete");
}


void ImageCollector::qrBinarization(Mat * inputImg, Mat * outputImg)
{
	try
	{
		Mat imHist;
	
		float range[] = { 0, 255 } ;
		const float* histRange = { range };
		int histSize = 255;
		int channels[] = {0, 1};

		calcHist(inputImg,1,NULL,Mat(),imHist,1,&histSize,&histRange,true,false);
	
		double maxVal=0;
		minMaxLoc(imHist, 0, &maxVal, 0, 0);

		float wScale = 1, maxHeight = 300;

		for( int h = 0; h < histSize; h++ )
		{
			int intensity = round(maxHeight*(imHist.at<float>(h)/maxVal));
			rectangle(*inputImg, Point(h*wScale, 0), Point((h+1)*wScale - 1, intensity+10),Scalar(255),CV_FILLED);
		}
	}
	catch (std::exception &e)
	{
		LOGE("ImageCollector",e.what());
	}
}