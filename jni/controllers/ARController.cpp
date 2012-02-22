#include "controllers/ARController.hpp"

int DebugRectangle::instanceCount = 0;
int DebugCircle::instanceCount = 0;
int QRCode::instanceCount = 0;
int FinderPattern::instanceCount = 0;

ARController::ARController(Mat cameraMatrix, Mat distortionMatrix)
{	
	double data[] = DEFAULT_CAMERA_MATRIX;
	double data2[] = DEFAULT_DISTORTION_MATRIX;

	if (cameraMatrix.empty())
	{
		LOGI(LOGTAG_ARCONTROLLER,"Using predefined camera matrix.");		
		cameraMatrix =  Mat(3,3,CV_64F,&data);
	}
	if (distortionMatrix.empty())
	{
		LOGI(LOGTAG_ARCONTROLLER,"Using predefined distortion matrix.");
		distortionMatrix = Mat(1,5,CV_64F,&data2);
	}	

	LOG_Mat(ANDROID_LOG_INFO,LOGTAG_ARCONTROLLER,"Using Camera Matrix",&cameraMatrix);
	LOG_Mat(ANDROID_LOG_INFO,LOGTAG_ARCONTROLLER,"Using Distortion Matrix",&distortionMatrix);
	
	//Create locator object
	qrLocator = new QRLocator(cameraMatrix,distortionMatrix);

	//Initialize image matrices
	grayImage = new Mat();
	rgbImage = new Mat();
	binaryImage = new Mat();
		
	//Populate circular list with frames
	const int NumFrameItems = 6;
	frameList = new CircularList<FrameItem*>(NumFrameItems);
	for (int i=0;i < frameList->getMaxSize(); i ++)
	{
		frameList->add(new FrameItem());
	}
	LOGD(LOGTAG_ARCONTROLLER,"Created %d frame items",frameList->getMaxSize());
			
	//Set default values
	augmentedView = NULL;
	controllerState = ControllerStates::Loading;
	isInitialized = false;
	isExpired = false;

	QRCode::instanceCount = 0;
	DebugCircle::instanceCount = 0;
	DebugRectangle::instanceCount = 0;
		
	frameCount = 0;
	fpsAverage = 0;
	paused = false;
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

void ARController::initializeUI(Engine * engine)
{
	//Initialize UI
	UIElementCollection * collection = new UIElementCollection();

	LOGI(LOGTAG_ARCONTROLLER,"Creating debug UI");
	debugUI = new ARControllerDebugUI(engine,Point2i(10,10));
	collection->AddChild(debugUI);
	drawObjects.push_back(debugUI);
	deletableObjects.push_back(debugUI);

	
	//Create QRFinder
	qrFinder = new QRFinder(debugUI);
			
	InputScaler * inputScaler = new InputScaler(engine->ImageSize(),engine->ScreenSize(),collection);
		
	engine->inputHandler->SetRootUIElement(inputScaler);
		
	fpsLabel = new Label("[FPS]",Point2i(0,0),Colors::MidnightBlue,Colors::White);
	fpsLabel->DoLayout(Rect(10,10,30,15));


	deletableObjects.push_back(collection);
	deletableObjects.push_back(inputScaler);
	deletableObjects.push_back(debugUI);

	LOGI(LOGTAG_ARCONTROLLER,"UI initialization complete.");
}

void ARController::Initialize(Engine * engine)
{
	if (isInitialized)
		return;		
	LOGD(LOGTAG_ARCONTROLLER,"Initializing ARController");	

	initializeUI(engine);

	//Sensors - enable gyroscope
	engine->sensorCollector->EnableSensors(false,false,false);
	
	//WorldLoader Instance
	worldLoader = new WorldLoader();

	//FastQRFinder
	fastQRFinder = new FastQRFinder(debugUI);

	//Position Selector instance
	positionSelector = new PositionSelector(debugUI);	
	
	//Initialize textured quad to render camera image
	quadBackground = new QuadBackground(engine->ImageSize());
	deletableObjects.push_back(quadBackground);

	engine->inputHandler->AddGlobalButtonDelegate(ButtonEventDelegate::from_method<ARController,&ARController::HandleButtonPress>(this));

	isInitialized = true;
	drawingLevel = 3;
	LOGD(LOGTAG_ARCONTROLLER,"Initialization complete");
}

void ARController::HandleButtonPress(void * sender, PhysicalButtonEventArgs args)
{
	if (args.ButtonCode == AKEYCODE_SEARCH)
	{
		drawingLevel++;
		drawingLevel = drawingLevel % 4; //0 = no drawing, 1 = debug draw only, 2 = UI only, 3 = all drawings
	}
	else if (args.ButtonCode == AKEYCODE_MENU)
	{
		paused = !paused;
	}
}

void ARController::Teardown(Engine * engine)
{
	if (!isInitialized)
		return;

	delete fpsLabel;
	LOGI(LOGTAG_ARCONTROLLER,"Teardown complete");
}

void ARController::initializeARView()
{
	LOGI(LOGTAG_ARCONTROLLER,"Initializing AR View");
	//Create Augmented View
	double data[] = DEFAULT_CAMERA_MATRIX;
	augmentedView = new AugmentedView(Mat(3,3,CV_64F,&data));

	//Add some cubes
	//objLoader loader;

	//LOGI(LOGTAG_ARCONTROLLER, "Loading model from file");
	//std::string fileName = std::string("/sdcard/objtest/cube.obj");
	//loader.load(fileName.c_str());
	//LOGI(LOGTAG_ARCONTROLLER, "Creating GL object from file");
	//ARObject * fromFile = new ARObject(WavefrontGLObject::FromObjFile(loader));
	//LOGI(LOGTAG_ARCONTROLLER, "Load complete");
	//fromFile->scale = Point3f(30,30,30);
	//augmentedView->AddObject(fromFile);	


	ARObject * myCube = new ARObject(OpenGLHelper::CreateMultiColorCube(20),Point3f(0,0,0));
	augmentedView->AddObject(myCube);	

	//myCube = new ARObject(OpenGLHelper::CreateSolidColorCube(20,Colors::MediumSeaGreen),Point3f(0,0,-40));
	//augmentedView->AddObject(myCube);	
	
	LOGI(LOGTAG_ARCONTROLLER,"AR View Initialization Complete");
}

void ARController::readGyroData(Engine * engine, FrameItem * item)
{
	cv::Mat rotationVector = engine->sensorCollector->GetRotation();
	rotationVector *= (180.0/PI);
	debugUI->SetRotation(&rotationVector);
}


void ARController::SetState(ControllerStates::ControllerState newState)
{
	LOGD(LOGTAG_ARCONTROLLER,"State changed from %d -> %d",controllerState,newState);
	controllerState = newState;
}


void ARController::ProcessFrame(Engine * engine)
{
	if (!isInitialized)
		return;

	frameCount++;
	//Calc FPS
	struct timespec currentTime;
	SET_TIME(&currentTime);
	double frameTimeMicrosec = calc_time_double(lastFrameTime,currentTime);
	lastFrameTime = currentTime;
	float frameFps = (float)(1000000.0/frameTimeMicrosec);
	fpsAverage = (fpsAverage+frameFps)/2.0f;
	char fpsString[10];
	sprintf(fpsString,"%3.1f",fpsAverage);
	fpsLabel->SetText(fpsString);


	//This section is the default per-frame operations
	FrameItem * item = frameList->next();
	item->clearOldData();
	//engine->getTime(&item->time);
	
	//LOGV(LOGTAG_ARCONTROLLER,"Processing frame #%d, FPS=%f. Loose objects: QR=%d, Rect=%d, Circ=%d, FP=%d",
		//frameCount,fpsAverage,(int)QRCode::instanceCount,DebugRectangle::instanceCount,DebugCircle::instanceCount, FinderPattern::instanceCount);

	//If paused, keep reusing image until unpaused
	if (!paused)
		getImages(engine);
	else
	{
		//Need to refresh the RGB image
		cvtColor(*grayImage, *rgbImage, CV_GRAY2RGBA, 4);
	}
		
	AlignmentPatternHelper::MinimumAlignmentPatternScore = 190;// debugUI->GetParameter("MinAlignScore");//lol static. THIS IS BAD!
	QRFinder::MinimumFinderPatternScore = 180;// debugUI->GetParameter("MinFPScore");

	vector<Drawable*> debugVector;

	bool decode = (controllerState == ControllerStates::Loading && (worldLoader != NULL && worldLoader->GetState() == WorldStates::LookingForCode));
	LOGV(LOGTAG_ARCONTROLLER,"Decoding=%d",decode);

	item->qrCode = qrFinder->LocateQRCodes(*grayImage, debugVector,decode);
	
	fastQRFinder->EnhanceQRCodes(*grayImage, item->qrCode, debugVector);
	
	LOGV(LOGTAG_ARCONTROLLER,"Drawing debug items");
	
	if (drawingLevel == 1 || drawingLevel == 3)
	{
		/*if (item->qrCode != NULL)
			item->qrCode->Draw(rgbImage);*/
		struct timespec draw_start, draw_end;
		SET_TIME(&draw_start);
		while (!debugVector.empty())
		{
			debugVector.back()->Draw(rgbImage);
			delete debugVector.back();
			debugVector.pop_back();
		}
		SET_TIME(&draw_end);
		LOG_TIME_PRECISE("DebugDrawing",draw_start,draw_end);
	}
	else //Still need to clean up debug vector!!!!
	{
		struct timespec draw_start, draw_end;
		SET_TIME(&draw_start);
		while (!debugVector.empty())
		{
			delete debugVector.back();
			debugVector.pop_back();
		}
		SET_TIME(&draw_end);
		LOG_TIME_PRECISE("DebugCleanup",draw_start,draw_end);
	}

	//What happens past here depends on the state
	if (controllerState == ControllerStates::Loading)
	{		
		//Update world loader
		worldLoader->Update(engine);

		WorldStates::WorldState worldState = worldLoader->GetState();

		if (worldState == WorldStates::LookingForCode)
		{
			debugUI->SetStateDisplay("Searching");
			if (item->qrCode != NULL && item->qrCode->validCodeFound && item->qrCode->TextValue.length() > 2)
			{
				LOGI(LOGTAG_ARCONTROLLER,"Code found, initializing realm with text=%s",item->qrCode->TextValue.c_str());
				//TODO: Move decoding to here
				worldLoader->LoadRealm(item->qrCode->TextValue);
			}
		}
		else if (worldState == WorldStates::WaitingForRealm || worldState == WorldStates::WaitingForResources)
		{		
			if (worldState == WorldStates::WaitingForRealm)
				debugUI->SetStateDisplay("WaitRealm");
			else
				debugUI->SetStateDisplay("WaitRsrc");

		}
		//The world is ready and loaded, so do normal AR processing
		else if (worldState == WorldStates::WorldReady)
		{
			debugUI->SetStateDisplay("LoadCompl");
			initializeARView();
			LOGD(LOGTAG_ARCONTROLLER,"Populating ARView using loaded world");
			worldLoader->PopulateARView(augmentedView);
			SetState(ControllerStates::Running);
			delete worldLoader;
			worldLoader = NULL;
		}	
		else
		{
			char stateString[100];
			sprintf(stateString,"WrldState=%d",(int)worldState);
			debugUI->SetStateDisplay(stateString);
			LOGW(LOGTAG_ARCONTROLLER,"Unexpected state");
		}
	}
	else if (controllerState == ControllerStates::Running)
	{
		debugUI->SetStateDisplay("Run");

		if (item->qrCode != NULL && item->qrCode->validCodeFound)
		{
			qrLocator->transformPoints(item->qrCode,*(item->rotationMatrix),*(item->translationMatrix));
			debugUI->SetTranslation(item->translationMatrix);
			debugUI->SetRotation(item->rotationMatrix);
		}	

		//Evaluate the position	
		float resultCertainty = positionSelector->UpdatePosition(item);
		debugUI->SetPositionCertainty(resultCertainty);

		if (resultCertainty > 0 && augmentedView != NULL)
		{
			//Update the 3D AR layer, but only if position certainty is non-zero
			augmentedView->Update(item);
		}
	}
	else
	{
		char stateString[100];
		sprintf(stateString,"ContState=%d",(int)controllerState);
		debugUI->SetStateDisplay(stateString);
		LOGW(LOGTAG_ARCONTROLLER,"Unexpected state");
	}

	//Do final processing
	Draw(rgbImage);
}

void ARController::Draw(Mat * rgbaImage)
{
	fpsLabel->Draw(rgbaImage);
	if (drawingLevel == 2 || drawingLevel == 3)
	{
		struct timespec start,end;
		SET_TIME(&start);
		//Draw objects onto camera texture
		for (int i=0;i<drawObjects.size();i++)
		{
			if (drawObjects.at(i)->IsVisible())
			{
				LOGV(LOGTAG_ARCONTROLLER,"Drawing object %d",i);
				drawObjects.at(i)->Draw(rgbaImage);
			}
		}
		SET_TIME(&end);
		LOG_TIME_PRECISE("ARController Drawing",start,end);
	}
	//Update textured quad 
	quadBackground->SetImage(rgbaImage);
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


void ARController::getImages(Engine * engine)
{
	struct timespec start, end;
	
	//Retrieve image from the camera	
	if (debugUI->currentDrawMode == DrawModes::ColorImage)
	{		
		engine->imageCollector->newFrame();
		engine->imageCollector->getCameraImages(*rgbImage, *grayImage);
	} 
	else if (debugUI->currentDrawMode == DrawModes::GrayImage || debugUI->currentDrawMode == DrawModes::BinaryImage)
	{
		SET_TIME(&start);
		engine->imageCollector->newFrame();
		engine->imageCollector->getGrayCameraImage(*grayImage);
		SET_TIME(&end);
		LOG_TIME_PRECISE("Image Capture", start, end);

		//Copy gray image to RGB image to be used by the following stages (rendering, debug overlay, etc)
		SET_TIME(&start)
		cvtColor(*grayImage, *rgbImage, CV_GRAY2RGBA, 4);
		SET_TIME(&end);
		LOG_TIME_PRECISE("Gray->RGBA", start, end);
	} 
	
	//if (USE_FEEDBACK_THRESH)
	//	ImageProcessor::FeedbackBinarization(item);
	//else
//	ImageProcessor::SimpleThreshold(grayImage, binaryImage);
	
	if (debugUI->currentDrawMode == DrawModes::BinaryImage)
	{
		SET_TIME(&start)
		cvtColor(*binaryImage, *rgbImage, CV_GRAY2RGBA, 4);
		SET_TIME(&end);
		LOG_TIME("Binary->RGBA", start, end);
	}
}


