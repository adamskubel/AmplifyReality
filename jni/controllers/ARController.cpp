#include "controllers/ARController.hpp"

int DebugRectangle::instanceCount = 0;
int DebugCircle::instanceCount = 0;
int QRCode::instanceCount = 0;
int FinderPattern::instanceCount = 0;

ARController::ARController(Mat cameraMatrix, Mat distortionMatrix, double fov)
{	
	double data[] = DEFAULT_CAMERA_MATRIX;
	double data2[] = DEFAULT_DISTORTION_MATRIX;
	startingFOV = (float)fov;
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
	qrLocator = new QRLocator(cameraMatrix,distortionMatrix,debugUI);
	recheckNext = false;

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
	window = new UIElementCollection();

	LOGI(LOGTAG_ARCONTROLLER,"Creating debug UI");
	debugUI = new ARControllerDebugUI(engine,Point2i(10,10));
	window->AddChild(debugUI);
	drawObjects.push_back(debugUI);
	deletableObjects.push_back(debugUI);
	
	qrLocator->config = debugUI;

	debugUI->AddNewParameter("T-Alpha",0.5f,0.1f,0.1f,1.0f,"%1.1f","Tracking");
	debugUI->AddNewParameter("R-Alpha",0.5f,0.1f,0.1f,1.0f,"%1.1f","Tracking");
	debugUI->AddNewParameter("FOV",startingFOV,1,10,180,"%3.0f","Tracking");
	debugUI->AddNewParameter("UseExtraPoints",1,1,0,1,"%1.0f","Tracking");
	debugUI->AddNewParameter("UseGuess",0,1,0,1,"%1.0f","Tracking");
	debugUI->AddNewParameter("UseGyro",1,1,0,1,"%1.0f","Tracking");
	debugUI->AddNewParameter("CodeOnly",0,1,0,1,"%1.0f","Tracking");
	debugUI->AddNewParameter("Show Entire Binary",0,1,0,1,"%1.0f","Tracking");
	
	debugUI->AddNewLabel("CurrentCode","","Data");
	debugUI->AddNewLabel("State","","Data");

	currentQRSize = 1.5f;
	debugUI->AddNewParameter("ARController","ARControllerDebug",0,1,-5,5,"%2.0f","Debug");
	debugUI->AddNewParameter("QR Unit size (mm)","QRSize",currentQRSize,0.5f,1,100,"%3.0f","Tracking");
	
			
	InputScaler * inputScaler = new InputScaler(engine->ImageSize(),engine->ScreenSize(),window);
		
	engine->inputHandler->SetRootUIElement(inputScaler);
		
	fpsLabel = new Label("[FPS]",Point2i(0,0),Colors::MediumBlue,Colors::DarkTurquoise);
	fpsLabel->DoLayout(Rect(engine->imageWidth - 40,10,30,15));

	resetButton = new Button("X");
	resetButton->DoLayout(Rect(0,engine->imageHeight-60,120,60));
	//window->AddChild(resetButton);
	resetButton->AddClickDelegate(ClickEventDelegate::from_method<ARController,&ARController::ButtonPressed>(this));
	resetButton->FillColor = Colors::DarkRed;

	deletableObjects.push_back(window);
	deletableObjects.push_back(inputScaler);
	deletableObjects.push_back(debugUI);

	certaintyIndicator = new CertaintyIndicator(1.0f,20);
	certaintyIndicator->DoLayout(Rect(engine->imageWidth-60,engine->imageHeight-60,40,40));

	LOGI(LOGTAG_ARCONTROLLER,"UI initialization complete.");
}


void ARController::ButtonPressed(void * sender, EventArgs args)
{
	recheckNext = true;
}

void ARController::Initialize(Engine * engine)
{
	if (isInitialized)
		return;		
	LOGD(LOGTAG_ARCONTROLLER,"Initializing ARController");	

	initializeUI(engine);

	engine->sensorCollector->EnableSensors(false,true,false);
	
	//Initialize stateful objects
	worldLoader = new WorldLoader();
	qrFinder = new QRFinder(debugUI);
	qrDecoder = new QRDecoder(debugUI);
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
		debugUI->SetVisible((drawingLevel == 2 || drawingLevel == 3));
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

void ARController::initializeARView(Engine * engine)
{
	LOGI(LOGTAG_ARCONTROLLER,"Initializing AR View");
	//Create Augmented View
	double data[] = DEFAULT_CAMERA_MATRIX;
	augmentedView = new AugmentedView(window,engine,Mat(3,3,CV_64F,&data));

	engine->inputHandler->AddGlobalTouchDelegate(TouchEventDelegate::from_method<AugmentedView,&AugmentedView::HandleTouchInput>(augmentedView));
	engine->inputHandler->AddGlobalButtonDelegate(ButtonEventDelegate::from_method<AugmentedView,&AugmentedView::HandleButtonPress>(augmentedView));
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
string currentCode;
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

	int debugLevel = debugUI->GetIntegerParameter("ARControllerDebug");
	bool useGuess = debugUI->GetBooleanParameter("UseGuess");
	bool showEntireBinary = debugUI->GetBooleanParameter("Show Entire Binary");
	

	//This section is the default per-frame operations
	FrameItem * item = frameList->next();
	item->clearOldData();
	//engine->getTime(&item->time);
	
	LOGV(LOGTAG_ARCONTROLLER,"Processing frame #%d, FPS=%f.",frameCount,fpsAverage);
		//Loose objects: QR=%d, Rect=%d, Circ=%d, FP=%d",
		//frameCount,fpsAverage,(int)QRCode::instanceCount,DebugRectangle::instanceCount,DebugCircle::instanceCount, FinderPattern::instanceCount);

	//If paused, keep reusing image until unpaused
	if (!paused)
	{
		getImages(engine);
	}
	else
	{
		//Need to refresh the RGB image
		cvtColor(*grayImage, *rgbImage, CV_GRAY2RGBA, 4);
	}
		
	vector<Drawable*> debugVector;
		 
	item->qrCode = qrFinder->LocateQRCodes(*grayImage, debugVector, ( frameList->size() > 1) ? frameList->getRelative(-1)->qrCode : NULL);	
	
	//What happens past here depends on the state
	if (controllerState == ControllerStates::Loading)
	{		
		//Update world loader
		worldLoader->Update(engine);

		WorldStates::WorldState worldState = worldLoader->GetState();

		if (worldState == WorldStates::LookingForCode)
		{
			debugUI->SetLabelValue("State","Searching");
			
			if (item->qrCode != NULL && item->qrCode->isValidCode())
			{	
				struct timespec decodeStart,decodeEnd;
				SET_TIME(&decodeStart);
				qrDecoder->DecodeQRCode(grayImage,binaryImage,item->qrCode,debugVector);
				SET_TIME(&decodeEnd);
				LOG_TIME("Decode",decodeStart,decodeEnd);
				if (item->qrCode->isDecoded())
				{
					string codeText = "Code=";
					codeText.append(item->qrCode->TextValue);
					debugUI->SetLabelValue("CurrentCode",codeText);
					currentCode = item->qrCode->TextValue;
					
					if (debugUI->GetBooleanParameter("CodeOnly"))
					{
						LOGI(LOGTAG_ARCONTROLLER,"Code found (%s), still searching since we're in debug mode",currentCode.c_str());
					}
					else if (engine->communicator->IsConnected())
					{
						worldLoader->LoadRealm(item->qrCode->TextValue);
					}
					else //If not connected, just add a test object
					{
						LOGI(LOGTAG_ARCONTROLLER,"Starting in offline mode.");

						debugUI->SetLabelValue("State","OfflineMode");
						initializeARView(engine);

						ARObject * myCube1 = new ARObject(OpenGLHelper::CreateMultiColorCube(20),Point3f(0,0,0));
						
						//ARObject * myCube2 = new ARObject(OpenGLHelper::CreateMultiColorCube(20),Point3f(50,10,0));
						//augmentedView->AddObject(myCube2);	
						//myCube2->BoundingSphereRadius = 30;						
						//ARObject * myCube3 = new ARObject(OpenGLHelper::CreateMultiColorCube(20),Point3f(-50,-10,0));
						//augmentedView->AddObject(myCube3);	
						//myCube3->BoundingSphereRadius = 30;

						augmentedView->AddObject(myCube1);	
						myCube1->BoundingSphereRadius = 12;

						SetState(ControllerStates::Running);
						delete worldLoader;
						worldLoader = NULL;
					}
				}
				else
					LOGD(LOGTAG_ARCONTROLLER,"QRCode not decoded");
			}
		}
		else if (worldState == WorldStates::WaitingForRealm || worldState == WorldStates::WaitingForResources)
		{		
			if (worldState == WorldStates::WaitingForRealm)
				debugUI->SetLabelValue("State","WaitRealm");
			else
				debugUI->SetLabelValue("State","WaitRsrc");

		}
		//The world is ready and loaded, so do normal AR processing
		else if (worldState == WorldStates::WorldReady)
		{
			debugUI->SetLabelValue("State","LoadCompl");
			initializeARView(engine);
			LOGD(LOGTAG_ARCONTROLLER,"Populating ARView using loaded world");
			worldLoader->PopulateARView(augmentedView);
			SetState(ControllerStates::Running);
			currentQRSize = debugUI->GetParameter("QRSize"); //Should be using value from server
			delete worldLoader;
			worldLoader = NULL;
		}	
		else
		{
			char stateString[100];
			sprintf(stateString,"WrldState=%d",(int)worldState);
			debugUI->SetLabelValue("State",stateString);
			LOGW(LOGTAG_ARCONTROLLER,"Unexpected state");
		}
	}
	else if (controllerState == ControllerStates::Running)
	{
		debugUI->SetLabelValue("State","Run");

		bool doSkip = false;
		if (item->qrCode != NULL && item->qrCode->isValidCode())
		{
			
			if (recheckNext)
			{
				LOGD(LOGTAG_ARCONTROLLER,"Rechecking code. Current value = %s",currentCode.c_str());
				qrDecoder->DecodeQRCode(grayImage,binaryImage,item->qrCode,debugVector);
			
				if (item->qrCode->isDecoded() && item->qrCode->TextValue != currentCode)
				{
					if (worldLoader == NULL)
						worldLoader = new WorldLoader();
					else
					{
						delete worldLoader;
						worldLoader = new WorldLoader();
					}
					LOGD(LOGTAG_ARCONTROLLER,"Changing to code %s",currentCode.c_str());
					currentCode = item->qrCode->TextValue;
					worldLoader->LoadRealm(currentCode);
					SetState(ControllerStates::Loading);
					delete augmentedView;
					augmentedView = NULL;
					recheckNext = false;
					doSkip = true;
				}
				else if (item->qrCode->isDecoded())
					recheckNext = false;
			}
			else
			{
				if (useGuess)
					positionSelector->GetPreviousResult(item);

				LOGV(LOGTAG_QR,"Getting position");
				currentQRSize = debugUI->GetParameter("QRSize"); //Should be using value from server
				item->qrCode->QRCodeDimension = currentQRSize;
				qrLocator->transformPoints(item->qrCode,*(item->rotationMatrix),*(item->translationMatrix),useGuess);
				debugUI->SetTranslation(item->translationMatrix);
				debugUI->SetRotation(item->rotationMatrix);
			}
		}	
		
		certaintyIndicator->EnableOutline(recheckNext);
		if (!doSkip)
		{
			//Evaluate the position	
			float resultCertainty = positionSelector->UpdatePosition(engine,item);
			certaintyIndicator->SetCertainty(resultCertainty);

			if (resultCertainty > 0 && augmentedView != NULL)
			{			
				augmentedView->SetFOV(debugUI->GetParameter("FOV"));
				augmentedView->SetTransformations(item->translationMatrix,item->rotationMatrix,item->gyroRotation);
			}
			if (augmentedView != NULL)
				augmentedView->Update(rgbImage,engine);
		}
	}
	else
	{
		char stateString[100];
		sprintf(stateString,"ContState=%d",(int)controllerState);
		debugUI->SetLabelValue("State",stateString);
		LOGW(LOGTAG_ARCONTROLLER,"Unexpected state: %s",stateString);
	}

	if (debugUI->currentDrawMode == DrawModes::BinaryImage)
	{		
		if (showEntireBinary)
		{		
			ImageProcessor::SimpleThreshold(grayImage,binaryImage);
			cvtColor(*binaryImage, *rgbImage, CV_GRAY2RGBA, 4);
		}
		else
		{
			LOGD(LOGTAG_ARCONTROLLER,"Binary debug draw");
			cvtColor(*binaryImage, *rgbImage, CV_GRAY2RGBA, 4);
			LOGD(LOGTAG_ARCONTROLLER,"Binary debug draw complete");
		}
	}
	
	if (drawingLevel == 1 || drawingLevel == 3)
	{
		if (item->qrCode != NULL)
		{
			item->qrCode->SetDrawingLevel(debugLevel);
			item->qrCode->Draw(rgbImage);
		}

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


	//Do final processing
	Draw(rgbImage);
}

void ARController::Draw(Mat * rgbaImage)
{
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

	//resetButton->Draw(rgbaImage);
	certaintyIndicator->Draw(rgbaImage);
	//Draw FPS label on top of everything else
	fpsLabel->Draw(rgbaImage);
	//Update textured quad 
	quadBackground->SetImage(rgbaImage);
}

void ARController::Render(OpenGL * openGL)
{
	if (!isInitialized)
		return;
	
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
		SET_TIME(&start);
		engine->imageCollector->newFrame();
		engine->imageCollector->getColorCameraImage(*rgbImage);
		SET_TIME(&end);
		LOG_TIME_PRECISE("Color Image Capture", start, end);

		cvtColor(*rgbImage, *grayImage, CV_RGBA2GRAY, 1);
	} 
	else 
	{
		SET_TIME(&start);
		engine->imageCollector->newFrame();
		engine->imageCollector->getGrayCameraImage(*grayImage);
		SET_TIME(&end);
		LOG_TIME_PRECISE("Gray Image Capture", start, end);

		cvtColor(*grayImage, *rgbImage, CV_GRAY2RGBA, 4);
	} 
		
	
}


