#include "ImageCollector.hpp"


int focusCount = 100;
void ImageCollector::newFrame()
{	
	myCapture->grab();
	/*if (focusCount-- == 0)
	{
		try
		{
		LOGI(LOGTAG_IMAGECAPTURE,"Starting autofocus with QCAR");
		bool result = QCAR::CameraDevice::getInstance().startAutoFocus();
		LOGI(LOGTAG_IMAGECAPTURE,"Focus result = %d",result);
		focusCount = 100;
		}
		catch (exception & e)
		{
			LOGI(LOGTAG_IMAGECAPTURE,"Exception while focusing: %s",e.what());
		}
	}*/
}

bool ImageCollector::IsReady()
{
	if (myCapture != NULL && myCapture->isOpened())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ImageCollector::SetAutograb(bool _autoGrab)
{
	double newPropVal = (double)_autoGrab;
	LOGI(LOGTAG_IMAGECAPTURE,"Setting autograb to %lf",newPropVal);
	myCapture->set(CV_CAP_PROP_AUTOGRAB,newPropVal);
	autoGrab = _autoGrab;
}

ImageCollector::ImageCollector(int width, int height)
{
	this->width = width;
	this->height = height;
	myCapture = NULL;


	this->grayOptMode = true;

	//Mat myuv(height + height / 2, width, CV_8UC1);
	mrgba = new Mat(height, width, CV_8UC4);
	mgray = new Mat(height, width, CV_8UC1);
	mbin = new Mat(height, width, CV_8UC1);
	distortionMatrix = NULL;
	cameraMatrix = NULL;

	LOGI(LOGTAG_IMAGECAPTURE,"Initializing VideoCapture");

	myCapture = new VideoCapture(CV_CAP_ANDROID);
		
	resolutionVector.clear();		

	if (!myCapture->isOpened())
	{
		LOGE("VideoCapture failed to open.");		
	}
	else
	{
		//LOGI(LOGTAG_IMAGECAPTURE,"Getting some props:");
		//LOGI(LOGTAG_IMAGECAPTURE,"CaptureMode=%lf,ConvertRGB=%lf,Sharpness=%lf,FPS=%lf",myCapture->get(CV_CAP_PROP_MODE),myCapture->get(CV_CAP_PROP_CONVERT_RGB),myCapture->get(CV_CAP_PROP_SHARPNESS),myCapture->get(CV_CAP_PROP_FPS));
		//double fps = myCapture->get(CV_CAP_PROP_FPS);
		//LOGI(LOGTAG_IMAGECAPTURE,"FPS=%lf",0);
		//double mode = myCapture->get(CV_CAP_PROP_MODE);
		//LOGI(LOGTAG_IMAGECAPTURE,"Mode=%lf",mode);

		double addr = myCapture->get(CV_CAP_PROP_SUPPORTED_PREVIEW_SIZES_STRING);
		char* result = *((char**)&addr);
		
		string sizes(result);
		LOGI(LOGTAG_IMAGECAPTURE,"Allowed preview sizes = %s",sizes.c_str());
		bool parsing = true;
		size_t found = 0;	



		while(parsing)
		{
			LOGD(LOGTAG_IMAGECAPTURE,"Found ',' at %d",found);			

			if (found != string::npos)
			{
				size_t resEnd = sizes.find(',',found+1);
				if (resEnd == string::npos)
				{
					resEnd = sizes.size();
					parsing = false;
				}
				LOGD(LOGTAG_IMAGECAPTURE,"Found next ',' at %d",resEnd);			
				string resString = sizes.substr(found,(resEnd-found));

				LOGD(LOGTAG_IMAGECAPTURE,"Resubstring=%s",resString.c_str());

				size_t dimSeperator = resString.find('x');

				if (dimSeperator != string::npos)
				{
					int width_res = (int)std::atol(resString.substr(0,dimSeperator).c_str());
					int height_res = (int)std::atol(resString.substr(dimSeperator+1).c_str());
					resolutionVector.push_back(Size2i(width_res,height_res));
					LOGI(LOGTAG_IMAGECAPTURE,"Found resolution: %d,%d",width_res,height_res);
				}
				found = resEnd+1;
			}
			else
			{
				break;
			}
		}
		
		try
		{
		LOGI(LOGTAG_IMAGECAPTURE,"Setting FPS to 30");
		myCapture->set(CV_CAP_PROP_FPS, 30);
		LOGI(LOGTAG_IMAGECAPTURE,"Setting FPS Complete");
		}
		catch (exception & e)
		{
			LOGW(LOGTAG_IMAGECAPTURE,"Exception: %s",e.what());
		}
		myCapture->set(CV_CAP_PROP_FRAME_WIDTH, width);
		myCapture->set(CV_CAP_PROP_FRAME_HEIGHT, height);
		
		SetAutograb(true);


		LOGI(LOGTAG_IMAGECAPTURE,"VideoCapture Initialized with width=%d and height=%d", width, height);		
	}
}

vector<Size2i> ImageCollector::GetResolutions()
{
	return resolutionVector;
}

void ImageCollector::SetResolution(Size2i resolution)
{
	myCapture->set(CV_CAP_PROP_FRAME_WIDTH, resolution.width);
	myCapture->set(CV_CAP_PROP_FRAME_HEIGHT, resolution.height);
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
		LOGE("UndistortImage: %s", e.what());
	}
}

void ImageCollector::getColorCameraImage(Mat & rgbImage)
{
	myCapture->retrieve(rgbImage, CV_CAP_ANDROID_COLOR_FRAME_RGBA);
}

//Get gray image only
void ImageCollector::getGrayCameraImage(Mat & grayImage)
{
	myCapture->retrieve(grayImage, CV_CAP_ANDROID_GREY_FRAME);
}
void ImageCollector::teardown()
{
	try
	{
		myCapture->release();
	}
	catch (std::exception& e)
	{
		LOGE("Error shutting down VC! -> %s", e.what());
	}
	LOGI(LOGTAG_IMAGECAPTURE,"Video Capture shutdown successfully");
}


