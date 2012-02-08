#include "StartupController.hpp"

StartupController::StartupController()
{
	isInitialized = false;
	isExpired = false;
	doCalibration = false;
}

StartupController::~StartupController()
{
	delete rgbaImage;

	if (!isInitialized)
		return;

	deleteVector.clear();
}

void StartupController::ProcessFrame(Engine * engine)
{
	if (!isInitialized)
		return;

	//Draw background
	rectangle(*rgbaImage,Point2i(0,0),Point2i(rgbaImage->cols,rgbaImage->rows),Colors::CornflowerBlue,-1);

	//Draw UI objects
	for (int i=0;i<drawObjects.size();i++)
	{
		drawObjects.at(i)->Draw(rgbaImage);
	}
	quadBackground->SetImage(rgbaImage);

}

void StartupController::Render(OpenGL * openGL)
{	
	//Only thing to render in this controller is textured quad
	quadBackground->Render(openGL);
}

void StartupController::Initialize(Engine * engine)
{
	if (isInitialized)
		return;
	
	GridLayout * grid = new GridLayout(engine->ScreenSize(),Size2i(4,3));

	engine->inputHandler->SetRootUIElement(grid);

	Button * startButton = new Button("Start",Colors::MidnightBlue);
	startButton->AddClickDelegate(ClickEventDelegate::from_method<StartupController,&StartupController::startButtonPress>(this));
	startButton->Name = "Start";
	
	Button * calibrateButton = new Button("Calibrate",Colors::MediumSeaGreen);
	calibrateButton->Name = "Calibrate";
	calibrateButton->AddClickDelegate(ClickEventDelegate::from_method<StartupController,&StartupController::startButtonPress>(this));

	Label * label = new Label("Status: Ready",Point2i(0,0),Colors::Black,Colors::White);

	grid->AddChild(label,Point2i(0,0),Size(4,1));
	grid->AddChild(startButton,Point2i(1,1),Size(2,1));
	grid->AddChild(calibrateButton,Point2i(1,2),Size(2,1));
		
	drawObjects.push_back(grid);

	if (!engine->imageCollector->IsReady())
	{
		startButton->SetEnabled(false);
		calibrateButton->SetEnabled(false);
		startButton->FillColor = Colors::Red;
		label->SetText("Error! Unable to access camera. Try restarting your phone.");
	}
	
	quadBackground = new QuadBackground(engine->ScreenSize());

	deleteVector.push_back(quadBackground);
	deleteVector.push_back(grid);

	LOGI(LOGTAG_MAIN,"StartupController Initialized");

	rgbaImage = new Mat(engine->ScreenSize(), CV_8UC4);

	isInitialized = true;

}
void StartupController::Teardown(Engine * engine)
{
	engine->inputHandler->SetRootUIElement(NULL);
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