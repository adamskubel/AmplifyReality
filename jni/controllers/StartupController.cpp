#include "StartupController.hpp"

StartupController::StartupController()
{
	isInitialized = false;
	exitNext = false;
	isExpired = false;
	doCalibration = false;
	connectNext = false;
	frameCount = 0;
	dir = 1;
	changeSize = Size2i(0,0);
}

StartupController::~StartupController()
{
	LOGD(LOGTAG_MAIN,"Cleaning up StartupController");
	delete rgbaImage;

	if (!isInitialized)
		return;
	
	while (!deleteVector.empty())
	{
		delete deleteVector.back();
		deleteVector.pop_back();
	}
}

void StartupController::ProcessFrame(Engine * engine)
{
	if (!isInitialized)
		return;
	
	if (!engine->imageCollector->IsReady())
	{
		startButton->SetEnabled(false);
		calibrateButton->SetEnabled(false);
		startButton->SetFillColor(Colors::Red);
		statusLabel->SetText("Error! Unable to access camera. Try restarting your phone.");
	}
	else if (!engine->communicator->IsConnected())
	{		
		startButton->SetEnabled(true);
		calibrateButton->SetEnabled(true);
		startButton->SetFillColor(Colors::LightSlateGray);
		statusLabel->SetText("Offline mode only");
	}
	else
	{		
		startButton->SetEnabled(true);
		calibrateButton->SetEnabled(true);
		startButton->SetFillColor(Colors::LimeGreen);
		statusLabel->SetText("Ready!");
	}
	
	if (changeSize.width != 0 && changeSize.height != 0)
	{
		LOGI(LOGTAG_MAIN, "Changing resolution to [%d,%d]",changeSize.width,changeSize.height);
		engine->imageHeight = changeSize.height;
		engine->imageWidth = changeSize.width;
		engine->imageCollector->SetResolution(changeSize);				
		changeSize.width = 0;
		changeSize.height = 0;
	}


	switch(engine->communicator->GetState())
	{
	case CommunicatorStates::Ready:
		networkStatusLabel->SetText("Ready to connect");		
		break;
	case CommunicatorStates::ConnectingNext:
		networkStatusLabel->SetText("Connecting....");
		break;
	case CommunicatorStates::AuthFailed:
		networkStatusLabel->SetText("Authentication failed!");
		break;
	case CommunicatorStates::ConnectFailed:
		networkStatusLabel->SetText("Connection failed!");
		break;
	case CommunicatorStates::Starting:
		networkStatusLabel->SetText("Starting up...");
		break;
	case CommunicatorStates::Connected:
		networkStatusLabel->SetText("Connected!");
		break;
	case CommunicatorStates::InvalidHost:
		networkStatusLabel->SetText("Invalid connection string");
		break;
	}
	
	if (connectNext && engine->communicator->CanConnect())
	{
		connectNext = false;
		std::string hostNameText =hostName->GetText();
		LOGD(LOGTAG_NETWORKING,"Hostname text = %s",hostNameText.c_str());
		
		engine->preferenceManager->SetPreference("connect.hostname",hostNameText);
		engine->preferenceManager->SetPreference("connect.username",userName->GetText());
		engine->preferenceManager->SetPreference("connect.password",password->GetText());

		engine->communicator->ConnectTo(hostNameText,userName->GetText(),password->GetText());
		networkStatusLabel->SetText("Connecting..");
	}


	Scalar bgColor = Colors::MidnightBlue;	
	if (frameCount > 50) frameCount = 50;	

	if (frameCount == 50)	
		dir = -1;	
	else if (frameCount < 0)
		dir = 1;
	
	frameCount += dir;

	LOGV(LOGTAG_MAIN,"Framecount=%d",frameCount);
	bgColor[2] = bgColor[2] * (100.0f - frameCount)/100.0f;

	//Draw background
	rectangle(*rgbaImage,Point2i(0,0),Point2i(rgbaImage->cols,rgbaImage->rows),bgColor,-1);


	//Draw UI objects
	for (int i=0;i<drawObjects.size();i++)
	{
		drawObjects.at(i)->Draw(rgbaImage);
	}
	quadBackground->SetImage(rgbaImage);

	if (exitNext)
	{
		LOGI(LOGTAG_MAIN,"Exiting from startup controller!");
		engine->ExitApplication();
	}

}

void StartupController::Render(OpenGL * openGL)
{	
	//Only thing to render in this controller is textured quad
	quadBackground->Render(openGL);
}


void StartupController::exitApplication(void * sender, EventArgs args)
{
	exitNext = true;
}

void StartupController::Initialize(Engine * engine)
{
	if (isInitialized)
		return;

	engine->imageCollector->SetAutograb(false);
	TabDisplay * tabs = new TabDisplay(false, Size2i(200,90));
	engine->inputHandler->SetRootUIElement(tabs);
	drawObjects.push_back(tabs);
		
	tabs->DoLayout(Rect(0,0,engine->ScreenSize().width,engine->ScreenSize().height));

	//Main page
	GridLayout * grid = new GridLayout(engine->ScreenSize(),Size2i(4,3));
	startButton = new Button("Start",Colors::LimeGreen);
	startButton->AddClickDelegate(ClickEventDelegate::from_method<StartupController,&StartupController::startButtonPress>(this));
	startButton->Name = "Start";

	Button * exitButton = new Button("Exit",Colors::Red);
	exitButton->AddClickDelegate(ClickEventDelegate::from_method<StartupController,&StartupController::exitApplication>(this));
			
	statusLabel = new Label("Ready!",Point2i(0,0),Colors::Black,Colors::White);
	
	grid->AddChild(exitButton,Point2i(3,2));
	grid->AddChild(statusLabel,Point2i(0,0),Size(4,1));
	grid->AddChild(startButton,Point2i(1,1),Size(2,1));
	tabs->AddTab("Begin",grid);

	//Misc. Configuration
	GridLayout * settingsGrid = new GridLayout(engine->ScreenSize(),Size2i(4,3));

	calibrateButton = new Button("Calibrate",Colors::MediumSeaGreen);
	calibrateButton->Name = "Calibrate";
	calibrateButton->AddClickDelegate(ClickEventDelegate::from_method<StartupController,&StartupController::startButtonPress>(this));		
	settingsGrid->AddChild(calibrateButton,Point2i(0,2),Size(1,1));
	tabs->AddTab("Settings",settingsGrid);

	vector<Size2i> resolutions = engine->imageCollector->GetResolutions();
	if (resolutions.size() > 0)
	{	
		settingsGrid->AddChild(CreateResolutionButtons(resolutions),Point2i(0,0),Size(4,1));
	}



	//Network Configuration
	GridLayout * networkConfigGrid = new GridLayout(engine->ScreenSize(),Size2i(4,5));
	tabs->AddTab("Network",networkConfigGrid);
	Label * hostName_label = new Label("Connection string (host[:port])",Point2i(0,0),Colors::Black,Colors::White);
	hostName = new TextBox(engine->ScreenSize(),engine->preferenceManager->GetPreference("connect.hostname",DEFAULT_HOST));
	engine->inputHandler->AddTextListener(hostName);
	networkStatusLabel = new Label("[x]",Point2i(0,0),Colors::Black, Colors::White);
	networkConfigGrid->AddChild(networkStatusLabel,Point2i(0,0),Size(4,1));


	Button * connectButton = new Button("Connect",Colors::Green);
	connectButton->AddClickDelegate(ClickEventDelegate::from_method<StartupController,&StartupController::ConnectButtonPressed>(this));
	networkConfigGrid->AddChild(connectButton,Point2i(1,4),Size(2,1));

	networkConfigGrid->AddChild(hostName_label,Point2i(0,1),Size(2,1));
	networkConfigGrid->AddChild(hostName,Point2i(2,1),Size(2,1));
	
	Label * userName_label = new Label("Username",Point2i(0,0),Colors::Black,Colors::White);
	userName = new TextBox(engine->ScreenSize(), engine->preferenceManager->GetPreference("connect.username",DEFAULT_USER));
	engine->inputHandler->AddTextListener(userName);

	networkConfigGrid->AddChild(userName_label,Point2i(0,2),Size(2,1));
	networkConfigGrid->AddChild(userName,Point2i(2,2),Size(2,1));
	
	Label * password_Label = new Label("Password",Point2i(0,0),Colors::Black,Colors::White);
	password = new TextBox(engine->ScreenSize(),engine->preferenceManager->GetPreference("connect.password",DEFAULT_PASS));
	password->obfuscateMode = true;
	password->SetText(DEFAULT_PASS);
	engine->inputHandler->AddTextListener(password);

	networkConfigGrid->AddChild(password_Label,Point2i(0,3),Size(2,1));
	networkConfigGrid->AddChild(password,Point2i(2,3),Size(2,1));
		
	tabs->DoLayout(Rect(0,0,engine->ScreenSize().width,engine->ScreenSize().height));

	tabs->SetTab(0);


	

	quadBackground = new QuadBackground(engine->ScreenSize());

	deleteVector.push_back(quadBackground);
	deleteVector.push_back(grid);

	LOGI(LOGTAG_MAIN,"StartupController Initialized");

	rgbaImage = new Mat(engine->ScreenSize(), CV_8UC4);

	isInitialized = true;	
	frameCount = 0;

}

GridLayout * StartupController::CreateResolutionButtons(vector<Size2i> & resolutions)
{
	Size2i gridSize = Size2i(resolutions.size(),1);
	GridLayout * resolutionGrid = new GridLayout(gridSize);

	for (int i=0;i<resolutions.size();i++)
	{
		char labelString[100];
		sprintf(labelString,"%d x %d",resolutions[i].width,resolutions[i].height);
		Button * newButton = new Button(labelString);
		newButton->Tag = new Size2i(resolutions[i].width,resolutions[i].height);
		resolutionGrid->AddChild(newButton,Point2i(i,0));
		newButton->AddClickDelegate(ClickEventDelegate::from_method<StartupController,&StartupController::ResolutionButtonPressed>(this));	
	}
	return resolutionGrid;
}

void StartupController::Teardown(Engine * engine)
{
	engine->inputHandler->SetRootUIElement(NULL);
	engine->inputHandler->RemoveTextListener(hostName);
	engine->inputHandler->RemoveTextListener(userName);
	engine->inputHandler->RemoveTextListener(password);
}	

void StartupController::ResolutionButtonPressed(void * sender, EventArgs args)
{
	Button * pressed = (Button*)sender;	
	changeSize = *((Size2i*)pressed->Tag);
}

void StartupController::ConnectButtonPressed(void * sender, EventArgs args)
{
	connectNext = true;
}


void StartupController::startButtonPress(void * sender, EventArgs args)
{
	Button * pressed = (Button*)sender;

	if (pressed->Name.compare("Calibrate") == 0)
	{
		doCalibration = true;
		LOGI(LOGTAG_MAIN,"Starting with calibration");
		SetExpired();
	}
	else 
	{		
		LOGI(LOGTAG_MAIN,"Starting without calibration");
		SetExpired();
	}
}

bool StartupController::IsExpired()
{
	return isExpired;
}

void StartupController::SetExpired()
{
	isExpired = true;
}


Controller * StartupController::GetSuccessor(Engine * engine)
{
	LOGI(LOGTAG_MAIN,"StartupController expired");
		
	engine->imageCollector->SetAutograb(true);
	Teardown(engine);
	if (doCalibration)
	{
		return new CalibrationController();
	}
	else
	{
		return new ARController(Mat(),Mat());
	}
}