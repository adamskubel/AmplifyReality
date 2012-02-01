#include "controllers/ARController.hpp"


ARController::ARController()
{
	LOGI(LOGTAG_ARCONTROLLER,"ARController created. Using predefined camera matrix.");
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

	LOGI(LOGTAG_ARCONTROLLER,"Created %d frame items",numItems);

	drawMode = Configuration::DefaultDrawMode;

	augmentedView = NULL;

	isInitialized = false;
	isExpired = false;
	LOGI(LOGTAG_ARCONTROLLER,"ARController Instantiation Complete");
}

ARController::ARController(Mat cameraMatrix, Mat distortionMatrix)
{
	LOGI(LOGTAG_ARCONTROLLER,"ARController created. Using calculated camera and distortion matrices.");
	qrLocator = new QRLocator(cameraMatrix,distortionMatrix);
	
	isInitialized = false;
	LOGI(LOGTAG_ARCONTROLLER,"ARController Instantiation Complete");
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

	LOGI(LOGTAG_ARCONTROLLER,"Creating debug UI");
	debugUI = new ARControllerDebugUI(engine,Point2i(10,10));
	collection->AddChild(debugUI);
	drawObjects.push_back(debugUI);
	deletableObjects.push_back(debugUI);

	LOGI(LOGTAG_ARCONTROLLER,"Creating ARConfig");
	config = new ARConfigurator(engine);
	collection->AddChild(config);
	drawObjects.push_back(config);
		
	InputScaler * inputScaler = new InputScaler(engine->ImageSize(),engine->ScreenSize(),collection);
		
	engine->inputHandler->SetRootUIElement(inputScaler);
	
	GridLayout * grid = new GridLayout(Size2i(engine->imageWidth,engine->imageHeight),Size_<int>(4,4));	
	drawObjects.push_back(grid);
	collection->AddChild(grid);
			
	Button * toggleConfigButton = new Button("Config", Colors::MidnightBlue);
	toggleConfigButton->AddClickDelegate(ClickEventDelegate::from_method<ARConfigurator,&ARConfigurator::ToggleVisibility>(config));
	grid->AddChild(toggleConfigButton,Point2i(3,0));

	engine->inputHandler->AddGlobalTouchDelegate(TouchEventDelegate::from_method<ARController,&ARController::HandleTouchInput>(this));	
	//End UI
	
	positionSelector = new PositionSelector(config);
	
	initializeARView();

	//Initialize textured quad to render camera image
	quadBackground = new QuadBackground(engine->ImageSize());

	deletableObjects.push_back(collection);
	deletableObjects.push_back(inputScaler);
	deletableObjects.push_back(grid);
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
	objLoader loader;

	LOGI(LOGTAG_ARCONTROLLER, "Loading model from file");
	std::string fileName = std::string("/sdcard/objtest/cube.obj");
	loader.load(fileName.c_str());
	LOGI(LOGTAG_ARCONTROLLER, "Creating GL object from file");
	ARObject * fromFile = new ARObject(WavefrontGLObject::FromObjFile(loader));
	LOGI(LOGTAG_ARCONTROLLER, "Load complete");
	fromFile->scale = Point3f(30,30,30);
	augmentedView->AddObject(fromFile);	


	ARObject * myCube = new ARObject(OpenGLHelper::CreateMultiColorCube(30),Point3f(0,0,0));
	//augmentedView->AddObject(myCube);	
	myCube = new ARObject(OpenGLHelper::CreateSolidColorCube(20,Colors::MediumSeaGreen),Point3f(0,0,-40));
	//augmentedView->AddObject(myCube);	
	
	LOGD(LOGTAG_ARCONTROLLER,"AR View Initialization Complete");
}

void ARController::readGyroData(Engine * engine, FrameItem * item)
{
	cv::Mat rotationVector = engine->sensorCollector->GetRotation();
	rotationVector *= (180.0/PI);
	debugUI->SetRotation(&rotationVector);
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
	
	//lol static. THIS IS BAD!
	AlignmentPatternHelper::MinimumAlignmentPatternScore = config->MinAlignmentScore;
	FinderPatternHelper::MinimumFinderPatternScore = config->MinFinderPatternScore;

	vector<Drawable*> debugVector;
	item->qrCode = QRFinder::LocateQRCodes(*binaryImage, debugVector);
	
	if (item->qrCode != NULL && item->qrCode->validCodeFound)
	{
		qrLocator->transformPoints(item->qrCode,*(item->rotationMatrix),*(item->translationMatrix));
		debugUI->SetTranslation(item->translationMatrix);
		debugUI->SetRotation(item->rotationMatrix);
	}	

	LOGV(LOGTAG_ARCONTROLLER,"Drawing QRCode");
	item->qrCode->Draw(rgbImage);
	for (int i=0;i<debugVector.size();i++)
	{
		debugVector.at(i)->Draw(rgbImage);
	}
	debugVector.clear();

		
	//Read data from gyroscope
	//readGyroData(engine,item);

	//Evaluate the position
	
	float resultCertainty = positionSelector->UpdatePosition(item);
	debugUI->SetPositionCertainty(resultCertainty);
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
