#include "controllers/ARController.hpp"


ARController::ARController()
{
	LOGD(LOGTAG_QR,"ARController created. Using predefined camera matrix.");
	double data[] = DEFAULT_CAMERA_MATRIX;
	double data2[] = DEFAULT_DISTORTION_MATRIX;

	qrLocator = new QRLocator(Mat(3,3,CV_64F,&data), Mat(1,5,CV_64F,&data2));


	initialized = false;
	LOGD(LOGTAG_QR,"ARController Instantiation Complete");
}

ARController::ARController(Mat cameraMatrix, Mat distortionMatrix)
{
	LOGD(LOGTAG_QR,"ARController created. Using calculated camera and distortion matrices.");
	qrLocator = new QRLocator(cameraMatrix,distortionMatrix);
	
	initialized = false;
	LOGD(LOGTAG_QR,"ARController Instantiation Complete");
}


ARController::~ARController()
{
	LOGI(LOGTAG_QR,"Deleting ARController...");
	
	delete qrLocator;
	
	if (!initialized)
		return;

	while (!updateObjects.empty())
	{
		delete updateObjects.back();
		updateObjects.pop_back();
	}
	
	//while(!renderObjects.empty())
	//{
	//	delete renderObjects.back();
	//	renderObjects.pop_back();
	//}
	delete defaultPosition;
	delete defaultRotation;
	delete quadBackground;
	LOGI(LOGTAG_QR,"ARController deleted successfully");
}



void ARController::Initialize(Engine * engine)
{
	if (initialized)
		return;
		
	LOGD(LOGTAG_QR,"Initializing ARController");

	//Set default position and rotation
	double defaultRotationData[] = {0,0,0};
	double defaultPositionData[] = {0,0,20};
	defaultPosition = new Mat(3,1,CV_64F,&defaultPositionData);
	defaultRotation = new Mat(3,1,CV_64F,&defaultRotationData);
	
	//Sensors - enable gyroscope
	engine->sensorCollector->EnableSensors(false,true,false);
	
	//Initialize UI
	GridLayout * grid = new GridLayout(engine,Size_<int>(3,3));
	translationVectorLabel = new Label("T",Point2i(0,0),Scalar::all(255),Scalar::all(0));
	grid->AddChild(translationVectorLabel,Point2i(0,1));

	gyroDataLabel = new Label("Gyro",Point2i(0,0),Scalar::all(255),Scalar::all(0));
	grid->AddChild(gyroDataLabel,Point2i(0,2));

	//engine->inputHandler->SetRootUIElement(grid);
	updateObjects.push_back(grid);
	//End UI

	initializeARView();

	//Initialize textured quad to render camera image
	quadBackground = new QuadBackground(engine->imageWidth,engine->imageHeight);

	initialized = true;
	LOGD(LOGTAG_QR,"Initialization complete");
}

void ARController::initializeARView()
{
	LOGD(LOGTAG_QR,"Initializing AR View");
	//Create Augmented View
	double data[] = DEFAULT_CAMERA_MATRIX;
	AugmentedView * augmentedView = new AugmentedView(Mat(3,3,CV_64F,&data));

	//Add some cubes
	ARObject * myCube = new ARObject(OpenGLHelper::CreateCube(30,Scalar(255,0,0,100)),Point3f(0,0,0));
	augmentedView->AddObject(myCube);	
	
	renderObjects.push_back(augmentedView);
	updateObjects.push_back(augmentedView);
	LOGD(LOGTAG_QR,"AR View Initialization Complete");
}

void ARController::readGyroData(Engine * engine, FrameItem * item)
{
	cv::Mat rotationVector = engine->sensorCollector->GetRotation();
	rotationVector *= (180.0/PI);
	char string[100];
	sprintf(string,"[%4.2lf,%4.2lf,%4.2lf]",rotationVector.at<double>(0,0),rotationVector.at<double>(0,1),rotationVector.at<double>(0,2));
	gyroDataLabel->Text = std::string(string);
}

void ARController::ProcessFrame(Engine * engine, FrameItem * item)
{
	if (!initialized)
		return;
	
	double defaultRotationData[] = {0,0,0};
	double defaultPositionData[] = {0,0,-100};
	defaultRotation = new Mat(1,3,CV_64F,&defaultRotationData);
	defaultPosition = new Mat(1,3,CV_64F,&defaultPositionData);
	
	LOGV(LOGTAG_QR,"Processing frame");
	getImages(engine,item);
	item->qrCode = QRFinder::locateQRCode(*(item->binaryImage), item->ratioMatches);
	if (item->qrCode != NULL && item->qrCode->validCodeFound)
	{
		qrLocator->transformPoints(item->qrCode,*(item->rotationMatrix),*(item->translationMatrix));
		if (translationVectorLabel != NULL)
		{
			char string[300];
			sprintf(string,"[%.3lf,%.3lf,%.3lf]",item->translationMatrix->at<double>(0,0),item->translationMatrix->at<double>(0,1),item->translationMatrix->at<double>(0,2));
			translationVectorLabel->Text = std::string(string);
		}	
	}
	drawDebugOverlay(item);	

	if (item->rotationMatrix->size().area() < 3)
	{
		LOGD(LOGTAG_QR,"Using default rotation");
		defaultRotation->copyTo(*(item->rotationMatrix));
	}
	if (item->translationMatrix->size().area() < 3)
	{		
		LOGD(LOGTAG_QR,"Using default position");
		defaultPosition->copyTo(*(item->translationMatrix));
	}
	readGyroData(engine,item);
		
	for (int i=0;i<updateObjects.size();i++)
	{
		updateObjects.at(i)->Update(item);
	}

	quadBackground->SetImage(item->rgbImage);
}

void ARController::Render(OpenGL * openGL)
{
	LOGV(LOGTAG_QR,"Rendering ARController. %d objects to render.", renderObjects.size()+1);
	if (!initialized)
		return;

	quadBackground->Render(openGL);

	for (int i=0;i<renderObjects.size();i++)
	{		
		renderObjects.at(i)->Render(openGL);
	}
}


void ARController::getImages(Engine * engine, FrameItem * item)
{
	struct timespec start, end;
	
	//Retrieve image from the camera	
	if (item->drawMode == Configuration::ColorImage)
	{		
		engine->imageCollector->newFrame();
		engine->imageCollector->getCameraImages(*(item->rgbImage), *(item->grayImage));
	} 
	else if (item->drawMode == Configuration::GrayImage || item->drawMode == Configuration::BinaryImage)
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
	
	if (USE_FEEDBACK_THRESH)
		ImageProcessor::FeedbackBinarization(item);
	else
		ImageProcessor::SimpleThreshold(item);
	
	if (item->drawMode == Configuration::BinaryImage)
	{
		SET_TIME(&start)
		cvtColor(*(item->binaryImage), *(item->rgbImage), CV_GRAY2RGBA, 4);
		SET_TIME(&end);
		LOG_TIME("Binary->RGBA", start, end);
	}
	SET_TIME(&end);
}



void ARController::drawDebugOverlay(FrameItem * item)
{
	LOGV(LOGTAG_QR,"Starting debug draw");
	struct timespec start, end;
	SET_TIME(&start);
	if (item->qrCode != NULL && item->qrCode->validCodeFound)
	{
		//For each of the 4 finder patterns, store the point in an array to form the debug rectangle
		Point2i * points = new Point_<int> [4];
		for (size_t i = 0; i < item->qrCode->finderPatterns->size(); i++)
		{
			FinderPattern  pattern = *(item->qrCode->finderPatterns->at(i));
			points[i] = pattern.pt;
		}

		fillConvexPoly(*(item->rgbImage),points, 4, Scalar(0, 255, 0, 255));	
		delete points;
	}
	else if (item->qrCode != NULL)
	{
		for (size_t i = 0; i < item->qrCode->finderPatterns->size(); i++)
		{
			FinderPattern  pattern = *(item->qrCode->finderPatterns->at(i));

			//Use the pattern size to estimate a rectangle to draw
			int size = pattern.size / 2;
			//Create an array to pass to the polyline function
			Point2i * points = new Point_<int> [4];
			points[0].x = (int) pattern.pt.x - size;
			points[0].y = (int) pattern.pt.y - size;

			points[1].x = (int) pattern.pt.x + size;
			points[1].y = (int) pattern.pt.y - size;

			points[2].x = (int) pattern.pt.x + size;
			points[2].y = (int) pattern.pt.y + size;

			points[3].x = (int) pattern.pt.x - size;
			points[3].y = (int) pattern.pt.y + size;

			int npts = 4;
			//const Point_<int> * pArray[] = {&(item->qrCode->finderPatterns.at(i)->pt)};
			const Point2i * pArray[] = {points};

			polylines(*(item->rgbImage),pArray,&npts,1,true,Scalar(0,0,255,255),4);
			delete points;
		}
	}
	for (size_t i = 0; i < item->ratioMatches.size(); i++)
	{
		circle(*(item->rgbImage), Point2i((item->ratioMatches)[i].x, (item->ratioMatches)[i].y), (item->ratioMatches)[i].z, Scalar(255, 0, 0, 255), 1);
	}

	SET_TIME(&end);
	LOG_TIME("Debug Draw", start, end);
}

