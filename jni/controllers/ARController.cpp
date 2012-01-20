#include "controllers/ARController.hpp"


ARController::ARController()
{
	LOGD(LOGTAG_QR,"ARController created. Using predefined camera matrix.");
	double data[] = DEFAULT_CAMERA_MATRIX;
	double data2[] = DEFAULT_DISTORTION_MATRIX;

	qrLocator = new QRLocator(Mat(3,3,CV_64F,&data), Mat(1,5,CV_64F,&data2));

	grayImage = new Mat();
	rgbImage = new Mat();
	binaryImage = new Mat();

	currentFrameItem = numItems-1;
	items = new FrameItem*[numItems];
	for (int i=0;i < numItems; i ++)
	{
		items[i] = new FrameItem();
	}

	LOGI(LOGTAG_MAIN,"Created %d frame items",numItems);

	drawMode = Configuration::DefaultDrawMode;

	isInitialized = false;
	isExpired = false;
	LOGD(LOGTAG_QR,"ARController Instantiation Complete");
}

ARController::ARController(Mat cameraMatrix, Mat distortionMatrix)
{
	LOGD(LOGTAG_QR,"ARController created. Using calculated camera and distortion matrices.");
	qrLocator = new QRLocator(cameraMatrix,distortionMatrix);
	
	isInitialized = false;
	LOGD(LOGTAG_QR,"ARController Instantiation Complete");
}


ARController::~ARController()
{
	LOGI(LOGTAG_QR,"Deleting ARController...");
	
	delete qrLocator;
	delete rgbImage, grayImage, binaryImage;
	
	if (!isInitialized)
	{
		LOGW(LOGTAG_QR,"Attempted to delete uninitialized controller");
		return;
	}

	drawObjects.clear();

	/*while (!drawObjects.empty())
	{
		delete drawObjects.back();
		drawObjects.pop_back();
	}*/
	
	delete defaultPosition;
	delete defaultRotation;
	delete quadBackground;
	delete positionSelector;
	LOGI(LOGTAG_QR,"ARController deleted successfully");
}



void ARController::Initialize(Engine * engine)
{
	if (isInitialized)
		return;
		
	LOGD(LOGTAG_QR,"Initializing ARController");
		
	//Sensors - enable gyroscope
	engine->sensorCollector->EnableSensors(false,true,false);
	
	//Initialize UI
	GridLayout * grid = new GridLayout(engine,Size_<int>(4,3));	
	engine->inputHandler->SetRootUIElement(grid);
	drawObjects.push_back(grid);

	translationVectorLabel = new Label("T",Point2i(0,0),Scalar::all(255),Scalar::all(0));
	grid->AddChild(translationVectorLabel,Point2i(0,1));

	gyroDataLabel = new Label("Gyro",Point2i(0,0),Scalar::all(255),Scalar::all(0));
	grid->AddChild(gyroDataLabel,Point2i(0,2));	

	positionCertainty = new CertaintyIndicator(0);
	grid->AddChild(positionCertainty,Point2i(3,2));
	positionCertainty->SetMaxRadius(20); //Override radius set by grid <-- BAD

	engine->inputHandler->AddGlobalTouchDelegate(TouchEventDelegate::from_method<ARController,&ARController::HandleTouchInput>(this));	
	//End UI
	
	positionSelector = new PositionSelector();
	
	initializeARView();

	//Initialize textured quad to render camera image
	quadBackground = new QuadBackground(engine->imageWidth,engine->imageHeight);
	
	//Finished initialization
	isInitialized = true;
	LOGD(LOGTAG_QR,"Initialization complete");
}

void ARController::Teardown(Engine * engine)
{
	if (!isInitialized)
		return;

	engine->inputHandler->RemoveDelegate(TouchEventDelegate::from_method<ARController,&ARController::HandleTouchInput>(this));
	LOGI(LOGTAG_QR,"Teardown complete");
}

void ARController::initializeARView()
{
	LOGD(LOGTAG_QR,"Initializing AR View");
	//Create Augmented View
	double data[] = DEFAULT_CAMERA_MATRIX;
	augmentedView = new AugmentedView(Mat(3,3,CV_64F,&data));

	//Add some cubes
	ARObject * myCube = new ARObject(OpenGLHelper::CreateCube(30,Scalar(255,0,0,100)),Point3f(0,0,0));
	augmentedView->AddObject(myCube);	
	
	//renderObjects.push_back(augmentedView);
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

void ARController::ProcessFrame(Engine * engine)
{
	if (!isInitialized)
		return;

	//Prepare the frame item
	int lastFrameItem = currentFrameItem;
	currentFrameItem = (currentFrameItem + 1) % numItems;
	LOGV(LOGTAG_MAIN,"Using item %d",currentFrameItem);
	FrameItem * item = items[currentFrameItem];


	LOGV(LOGTAG_MAIN,"Clearing old data from item");
	item->clearOldData();
	item->drawMode = drawMode;
	
	//Set frame timestamp
	struct timespec time;
	engine->getTime(&time);
	item->nanotime = time.tv_nsec;

	
	//double defaultRotationData[] = {0,0,0};
	//double defaultPositionData[] = {0,0,-100};
	//defaultRotation = new Mat(1,3,CV_64F,&defaultRotationData);
	//defaultPosition = new Mat(1,3,CV_64F,&defaultPositionData);
	
	LOGV(LOGTAG_QR,"Processing frame");
	getImages(engine,item);
	item->qrCode = QRFinder::locateQRCode(*binaryImage, item->ratioMatches);
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
	//Draw debugging overlay
	drawDebugOverlay(item);	
		
	//Read data from gyroscope
	readGyroData(engine,item);

	//Evaluate the position
	
	float resultCertainty = positionSelector->UpdatePosition(item);
	LOGD(LOGTAG_QR,"Position certainty of %f",resultCertainty);
	positionCertainty->SetCertainty(resultCertainty);
	if (positionCertainty > 0 && augmentedView != NULL)
	{
		//Update the 3D AR layer, but only if position certainty is non-zero
		LOGD(LOGTAG_QR,"Updating ARView");
		augmentedView->Update(item);
		LOGD(LOGTAG_QR,"Update complete");
	}
	//Draw objects onto camera texture
	for (int i=0;i<drawObjects.size();i++)
	{
		LOGV(LOGTAG_QR,"Drawing object %d",i);
		drawObjects.at(i)->Draw(rgbImage);
	}

	//Update textured quad 
	quadBackground->SetImage(rgbImage);
}

void ARController::Render(OpenGL * openGL)
{
	if (!isInitialized)
		return;
	
	LOGV(LOGTAG_QR,"Rendering ARController. %d objects to render.", renderObjects.size()+1);
	quadBackground->Render(openGL);

	if (augmentedView != NULL)
		augmentedView->Render(openGL);

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
		engine->imageCollector->getCameraImages(*rgbImage, *grayImage);
	} 
	else if (item->drawMode == Configuration::GrayImage || item->drawMode == Configuration::BinaryImage)
	{
		SET_TIME(&start);
		engine->imageCollector->newFrame();
		engine->imageCollector->getGrayCameraImage(*grayImage);
		SET_TIME(&end);
		LOG_TIME("Image Capture", start, end);

		//Copy gray image to RGB image to be used by the following stages (rendering, debug overlay, etc)
		SET_TIME(&start)
		cvtColor(*grayImage, *rgbImage, CV_GRAY2RGBA, 4);
		SET_TIME(&end);
		LOG_TIME("Gray->RGBA", start, end);
	} 
	
	if (USE_FEEDBACK_THRESH)
		ImageProcessor::FeedbackBinarization(item);
	else
		ImageProcessor::SimpleThreshold(grayImage, binaryImage);
	
	if (item->drawMode == Configuration::BinaryImage)
	{
		SET_TIME(&start)
		cvtColor(*binaryImage, *rgbImage, CV_GRAY2RGBA, 4);
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

		fillConvexPoly(*rgbImage,points, 4, Scalar(0, 255, 0, 255));	
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

			polylines(*rgbImage,pArray,&npts,1,true,Scalar(0,0,255,255),4);
			delete points;
		}
	}
	for (size_t i = 0; i < item->ratioMatches.size(); i++)
	{
		circle(*rgbImage, Point2i((item->ratioMatches)[i].x, (item->ratioMatches)[i].y), (item->ratioMatches)[i].z, Scalar(255, 0, 0, 255), 1);
	}

	SET_TIME(&end);
	LOG_TIME("Debug Draw", start, end);
}

void ARController::HandleTouchInput(void* sender, TouchEventArgs args)
{
	LOGI(LOGTAG_MAIN,"Received touch event: %d", args.InputType);
	switch (drawMode)
	{
	case(Configuration::BinaryImage):
		drawMode = Configuration::ColorImage;
		break;
	case(Configuration::GrayImage):
		drawMode = Configuration::BinaryImage;
		break;
	case(Configuration::ColorImage):
		drawMode = Configuration::GrayImage;
		break;
	}
}