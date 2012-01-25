#include "controllers/ARController.hpp"


ARController::ARController()
{
	LOGD(LOGTAG_ARCONTROLLER,"ARController created. Using predefined camera matrix.");
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
	LOGD(LOGTAG_ARCONTROLLER,"ARController Instantiation Complete");
}

ARController::ARController(Mat cameraMatrix, Mat distortionMatrix)
{
	LOGD(LOGTAG_ARCONTROLLER,"ARController created. Using calculated camera and distortion matrices.");
	qrLocator = new QRLocator(cameraMatrix,distortionMatrix);
	
	isInitialized = false;
	LOGD(LOGTAG_ARCONTROLLER,"ARController Instantiation Complete");
}


ARController::~ARController()
{
	LOGI(LOGTAG_ARCONTROLLER,"Deleting ARController...");
	
	delete qrLocator;
	delete rgbImage, grayImage, binaryImage;
	
	if (!isInitialized)
	{
		LOGW(LOGTAG_ARCONTROLLER,"Attempted to delete uninitialized controller");
		return;
	}
	
	deletableObjects.clear();

	delete quadBackground;
	delete positionSelector;
	LOGI(LOGTAG_ARCONTROLLER,"ARController deleted successfully");
}



void ARController::Initialize(Engine * engine)
{
	if (isInitialized)
		return;
		
	LOGD(LOGTAG_ARCONTROLLER,"Initializing ARController");
		
	//Sensors - enable gyroscope
	//engine->sensorCollector->EnableSensors(false,true,false);
	
	//Initialize UI

	UIElementCollection * collection = new UIElementCollection();
		
	float scaleFactor = max((float)(engine->imageWidth)/(float)(engine->glRender->screenWidth),(float)(engine->imageHeight)/(float)(engine->glRender->screenHeight));
	InputScaler * inputScaler = new InputScaler(scaleFactor,collection);
		
	engine->inputHandler->SetRootUIElement(inputScaler);
	
	grid = new GridLayout(Size2i(engine->imageWidth,engine->imageHeight),Size_<int>(4,4));	
	drawObjects.push_back(grid);
	collection->AddChild(grid);

	translationVectorLabel = new Label("T",Point2i(0,0),Scalar::all(255),Scalar::all(0));
	translationVectorLabel->FontScale = 0.9f;
	grid->AddChild(translationVectorLabel,Point2i(0,2));

	gyroDataLabel = new Label("R",Point2i(0,0),Scalar::all(255),Scalar::all(0));
	gyroDataLabel->FontScale = 0.9f;
	grid->AddChild(gyroDataLabel,Point2i(0,3));	

	positionCertainty = new CertaintyIndicator(0);
	grid->AddChild(positionCertainty,Point2i(3,3));
	positionCertainty->SetMaxRadius(20); //Override radius set by grid <<<<---- Bad practice! ...but saves time

	config = new ARConfigurator(engine, collection,Point2i(0,0));
	drawObjects.push_back(config);
	
	Button * toggleConfigButton = new Button("Config", Colors::MidnightBlue);
	toggleConfigButton->AddClickDelegate(ClickEventDelegate::from_method<ARConfigurator,&ARConfigurator::ToggleVisibility>(config));
	grid->AddChild(toggleConfigButton,Point2i(3,0));

	engine->inputHandler->AddGlobalTouchDelegate(TouchEventDelegate::from_method<ARController,&ARController::HandleTouchInput>(this));	
	//End UI
	
	positionSelector = new PositionSelector(config);
	
	initializeARView();

	//Initialize textured quad to render camera image
	quadBackground = new QuadBackground(engine->imageWidth,engine->imageHeight);

	deletableObjects.push_back(collection);
	deletableObjects.push_back(inputScaler);
	deletableObjects.push_back(grid);
	deletableObjects.push_back(gyroDataLabel);
	deletableObjects.push_back(quadBackground);
	deletableObjects.push_back(config);
	//Finished initialization
	isInitialized = true;
	LOGD(LOGTAG_ARCONTROLLER,"Initialization complete");
}

void ARController::Teardown(Engine * engine)
{
	if (!isInitialized)
		return;

	engine->inputHandler->RemoveDelegate(TouchEventDelegate::from_method<ARController,&ARController::HandleTouchInput>(this));
	LOGI(LOGTAG_ARCONTROLLER,"Teardown complete");
}

void ARController::initializeARView()
{
	LOGD(LOGTAG_ARCONTROLLER,"Initializing AR View");
	//Create Augmented View
	double data[] = DEFAULT_CAMERA_MATRIX;
	augmentedView = new AugmentedView(Mat(3,3,CV_64F,&data));

	//Add some cubes
	ARObject * myCube = new ARObject(OpenGLHelper::CreateMultiColorCube(30),Point3f(0,0,0));
	augmentedView->AddObject(myCube);	
	myCube = new ARObject(OpenGLHelper::CreateSolidColorCube(20,Colors::MediumSeaGreen),Point3f(0,0,-40));
	augmentedView->AddObject(myCube);	
	
	LOGD(LOGTAG_ARCONTROLLER,"AR View Initialization Complete");
}

void ARController::readGyroData(Engine * engine, FrameItem * item)
{
	cv::Mat rotationVector = engine->sensorCollector->GetRotation();
	rotationVector *= (180.0/PI);
	char string[100];
	sprintf(string,"[%.2lf,%.2lf,%.2lf]",rotationVector.at<double>(0,0),rotationVector.at<double>(0,1),rotationVector.at<double>(0,2));
	gyroDataLabel->SetText(string);
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
		
	LOGV(LOGTAG_ARCONTROLLER,"Processing frame");
	getImages(engine,item);
	
	QRFinder::minAlignmentScore = config->MinAlignmentScore;
	item->qrCode = QRFinder::locateQRCode(*binaryImage, item->ratioMatches, config->MinFinderPatternScore);
	
	if (item->qrCode != NULL && item->qrCode->validCodeFound)
	{
		qrLocator->transformPoints(item->qrCode,*(item->rotationMatrix),*(item->translationMatrix));
		if (translationVectorLabel != NULL)
		{
			char string[300];
			sprintf(string,"[%.2lf, %.2lf, %.2lf]",item->translationMatrix->at<double>(0,0),item->translationMatrix->at<double>(0,1),item->translationMatrix->at<double>(0,2));
			translationVectorLabel->SetText(string);
		}	
		if (gyroDataLabel != NULL)
		{		
			char string[300];
			sprintf(string,"[%.2lf,%.2lf,%.2lf]",item->rotationMatrix->at<double>(0,0),item->rotationMatrix->at<double>(0,1),item->rotationMatrix->at<double>(0,2));
			gyroDataLabel->SetText(string);		
		}
	}
	//Draw debugging overlay
	drawDebugOverlay(item);	
		
	//Read data from gyroscope
	//readGyroData(engine,item);

	//Evaluate the position
	
	float resultCertainty = positionSelector->UpdatePosition(item);
	positionCertainty->SetCertainty(resultCertainty);
	if (resultCertainty > 0 && augmentedView != NULL)
	{
		//Update the 3D AR layer, but only if position certainty is non-zero
		augmentedView->Update(item);
	}
	//Draw objects onto camera texture
	for (int i=0;i<drawObjects.size();i++)
	{
		if (drawObjects.at(i)->IsVisible())
		{
			LOGV(LOGTAG_ARCONTROLLER,"Drawing object %d",i);
			drawObjects.at(i)->Draw(rgbImage);
		}
	}

	//Update textured quad 
	quadBackground->SetImage(rgbImage);
}

void ARController::Render(OpenGL * openGL)
{
	if (!isInitialized)
		return;
	
	LOGV(LOGTAG_ARCONTROLLER,"Rendering ARController. %d objects to render.", renderObjects.size()+1);
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
	LOGV(LOGTAG_ARCONTROLLER,"Starting debug draw");
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

		fillConvexPoly(*rgbImage,points, 4, Colors::Lime);	
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
			const Point2i * pArray[] = {points};

			if (!pattern.isAlignment)				
				polylines(*rgbImage,pArray,&npts,1,true,Colors::Blue,4);
			else
				polylines(*rgbImage,pArray,&npts,1,true,Colors::Red,4);
			
			
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
