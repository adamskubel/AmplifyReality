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
	
	
	const int NumFrameItems = 6;

	frameList = new CircularList<FrameItem*>(NumFrameItems);
	for (int i=0;i < frameList->getMaxSize(); i ++)
	{
		frameList->add(new FrameItem());
	}

	LOGD(LOGTAG_ARCONTROLLER,"Created %d frame items",frameList->getMaxSize());
			
	augmentedView = NULL;
	state = ControllerStates::Loading;
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

void ARController::initializeUI(Engine * engine)
{
	//Initialize UI
	UIElementCollection * collection = new UIElementCollection();

	LOGI(LOGTAG_ARCONTROLLER,"Creating debug UI");
	debugUI = new ARControllerDebugUI(engine,Point2i(10,10));
	collection->AddChild(debugUI);
	drawObjects.push_back(debugUI);
	deletableObjects.push_back(debugUI);
			
	InputScaler * inputScaler = new InputScaler(engine->ImageSize(),engine->ScreenSize(),collection);
		
	engine->inputHandler->SetRootUIElement(inputScaler);
	
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

	//Position Selector instance
	positionSelector = new PositionSelector(debugUI);	

	//Initialize textured quad to render camera image
	quadBackground = new QuadBackground(engine->ImageSize());
	deletableObjects.push_back(quadBackground);

	isInitialized = true;
	LOGD(LOGTAG_ARCONTROLLER,"Initialization complete");
}

void ARController::Teardown(Engine * engine)
{
	if (!isInitialized)
		return;

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
	LOGD(LOGTAG_ARCONTROLLER,"State changed from %d -> %d",state,newState);
	state = newState;
}

void ARController::ProcessFrame(Engine * engine)
{
	if (!isInitialized)
		return;

	//This section is the default per-frame operations
	FrameItem * item = frameList->next();
	item->clearOldData();
	//engine->getTime(&item->time);
	
	LOGV(LOGTAG_ARCONTROLLER,"Processing frame");
	getImages(engine,item);
		
	AlignmentPatternHelper::MinimumAlignmentPatternScore = debugUI->MinAlignmentScore;//lol static. THIS IS BAD!
	FinderPatternHelper::MinimumFinderPatternScore = debugUI->MinFinderPatternScore;

	vector<Drawable*> debugVector;
	item->qrCode = QRFinder::LocateQRCodes(*binaryImage, debugVector);
	item->qrCode->Draw(rgbImage);
	while (!debugVector.empty())
	{
		debugVector.back()->Draw(rgbImage);
		delete debugVector.back();
		debugVector.pop_back();
	}
	

	//What happens past here depends on the state
	if (state == ControllerStates::Loading)
	{		
		//Update world loader
		worldLoader->Update(engine);

		WorldStates::WorldState state = worldLoader->GetState();

		if (state == WorldStates::LookingForCode)
		{
			debugUI->SetStateDisplay("Searching");
			if (item->qrCode != NULL && item->qrCode->validCodeFound && item->qrCode->TextValue.length() > 2)
			{
				LOGI(LOGTAG_ARCONTROLLER,"Code found, initializing realm.");
				//TODO: Move decoding to here
				worldLoader->LoadRealm(item->qrCode->TextValue);
			}
		}
		else if (state == WorldStates::WaitingForRealm || state == WorldStates::WaitingForResources)
		{		
			if (state == WorldStates::WaitingForRealm)
				debugUI->SetStateDisplay("WaitRealm");
			else
				debugUI->SetStateDisplay("WaitRsrc");

		}
		//The world is ready and loaded, so do normal AR processing
		else if (state == WorldStates::WorldReady)
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
			sprintf(stateString,"State=%d",(int)state);
			debugUI->SetStateDisplay(stateString);
			LOGW(LOGTAG_ARCONTROLLER,"Unexpected state");
		}
	}
	else if (state == ControllerStates::Running)
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

	//Do final processing
	Draw(rgbImage);
}

void ARController::Draw(Mat * rgbaImage)
{
	//Draw objects onto camera texture
	for (int i=0;i<drawObjects.size();i++)
	{
		if (drawObjects.at(i)->IsVisible())
		{
			LOGV(LOGTAG_ARCONTROLLER,"Drawing object %d",i);
			drawObjects.at(i)->Draw(rgbaImage);
		}
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


void ARController::getImages(Engine * engine, FrameItem * item)
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
	
	if (debugUI->currentDrawMode == DrawModes::BinaryImage)
	{
		SET_TIME(&start)
		cvtColor(*binaryImage, *rgbImage, CV_GRAY2RGBA, 4);
		SET_TIME(&end);
		LOG_TIME("Binary->RGBA", start, end);
	}
	SET_TIME(&end);
}


