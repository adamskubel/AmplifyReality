#include "CalibrationController.hpp"

CalibrationController::CalibrationController()
{
	objectPoints = new vector<vector<Point3f> >();
	imagePoints = new vector<vector<Point2f> >();
	collectionCount = 0;
	chessBoardSize = Size_<int>(7, 7);

	drawObjects = vector<Drawable*>();

	isInitialized = false;

	rgbImage = new Mat();
	binaryImage = new Mat();
	grayImage = new Mat();
	missCount = 0;
	state = CalibrationControllerStates::Running;

	LOGI(LOGTAG_CALIBRATION,"Calibration Controller created");
}

CalibrationController::~CalibrationController()
{
	LOGI(LOGTAG_CALIBRATION,"Cleaning up calibration controller");
	delete objectPoints;
	delete imagePoints;
	delete grayImage, binaryImage, rgbImage;


	if (!isInitialized)
		return;

	while (!drawObjects.empty())
	{
		delete drawObjects.back();
		drawObjects.pop_back();
	}
	delete inputScaler;
	delete quadBackground;
	LOGI(LOGTAG_CALIBRATION, "CalibrationController deleted successfully");
}

void CalibrationController::Initialize(Engine * engine)
{	
	if (isInitialized)
		return;
	LOGI(LOGTAG_CALIBRATION,"Initializing calibration controller");

	layout = new GridLayout(Size2i(engine->imageWidth,engine->imageHeight),Size_<int>(6,4));

	//Create image capture button
	captureButton = new Button(std::string("Capture"),cv::Rect(300,300,150,160),Colors::Turquoise);
	captureButton->AddClickDelegate(ClickEventDelegate::from_method<CalibrationController,&CalibrationController::HandleButtonClick>(this));	
	captureButton->Name = std::string("CaptureButton");
	layout->AddChild(captureButton,Point2i(4,3),Size_<int>(2,1));

	cameraMatDisplay = new DataDisplay("%6.2lf",Colors::Black,Colors::White);
	distortionMatDisplay = new DataDisplay("%3.2lf",Colors::Black,Colors::White);

	fovLabel = new Label("",Point2i(0,0),Colors::Orange,Colors::DarkRed);
	fovLabel->FontScale = 0.7f;
	principalLabel = new Label("",Point2i(0,0),Colors::Orange,Colors::DarkRed);
	principalLabel->FontScale = 0.7f;
	
	layout->AddChild(fovLabel,Point2i(0,0), Size2i(4,1));
	layout->AddChild(principalLabel,Point2i(0,1), Size2i(4,1));	
	layout->AddChild(cameraMatDisplay,Point2i(0,2), Size2i(2,1));	
	layout->AddChild(distortionMatDisplay,Point2i(0,3),Size2i(3,1));

	//Create exit button
	calculateButton = new Button(std::string("Calculate"),Colors::Green);
	calculateButton->AddClickDelegate(ClickEventDelegate::from_method<CalibrationController,&CalibrationController::HandleButtonClick>(this));
	calculateButton->Name = std::string("CalculateButton");
	layout->AddChild(calculateButton,Point2i(4,0),Size_<int>(2,1));

	//Size adjuster
	sizeSpinner = new NumberSpinner("SquareSize(mm)",10,0.5,"%3.0f");
	layout->AddChild(sizeSpinner,Point2i(4,1),Size2i(2,2));

	//Set grid layout as the root UI element
	inputScaler = new InputScaler(engine->ImageSize(),engine->ScreenSize(),layout);

	engine->inputHandler->SetRootUIElement(inputScaler);
	drawObjects.push_back(layout);

	//Create background quad
	quadBackground = new QuadBackground(Size2i(engine->ImageSize()));
	layout->DoLayout(Rect(0,0,engine->imageWidth,engine->imageHeight));
	isInitialized = true;
	LOGI(LOGTAG_CALIBRATION,"Initialization complete");
}

void CalibrationController::Teardown(Engine * engine)
{
	if (!isInitialized)
		return;

	engine->inputHandler->SetRootUIElement(NULL);
	LOGI(LOGTAG_CALIBRATION,"Teardown complete");
}

static void setCaptureButton(Button * captureButton, int collectionCount)
{
	//Update label
	char myString[100];
	sprintf(myString, "Capture (%d)", collectionCount);
	captureButton->SetText(std::string(myString));
	captureButton->SetFillColor(Colors::Turquoise);
}

void CalibrationController::HandleButtonClick(void * sender, EventArgs args)
{
	Button * button = (Button*)sender;

	if (button == NULL)
	{
		LOGE("Button is null!");
		return;
	}

	if (button->Name.compare("CaptureButton") == 0)
	{
		if (state == CalibrationControllerStates::Calculated)
		{
			LOGI(LOGTAG_CALIBRATION,"Redoing calculation.");
			collectionCount = 0;
			missCount = 0;
			imagePoints->clear();
			objectPoints->clear();
			state = CalibrationControllerStates::Running;

			cameraMatDisplay->SetVisible(false);
			distortionMatDisplay->SetVisible(false);

			calculateButton->SetText("Calculate");
			setCaptureButton(captureButton,collectionCount);
		}
		else if (state == CalibrationControllerStates::Running)
		{
			missCount = 0;
			state = CalibrationControllerStates::Finding;
			
			captureButton->SetFillColor(Colors::OrangeRed);
			captureButton->SetText("Cancel search");
		}
		else if (state == CalibrationControllerStates::Finding)
		{
			//Abort capture attempt
			state = CalibrationControllerStates::Running;			
			setCaptureButton(captureButton,collectionCount);
		}
		else
		{
			LOGW(LOGTAG_CALIBRATION,"Unexpected state: %d",state);
		}
	}
	else if (button->Name.compare("CalculateButton") == 0)
	{
		if (state == CalibrationControllerStates::Calculated)
		{
			LOGI(LOGTAG_CALIBRATION,"Calibration accepted. Exiting.");
			state = CalibrationControllerStates::Complete;
		}
		else if (state == CalibrationControllerStates::Running)
		{			
			state = CalibrationControllerStates::Calculating1;
		}
		
	}
}

bool CalibrationController::IsExpired()
{
	return state == CalibrationControllerStates::Exiting || state == CalibrationControllerStates::Complete;
}

void CalibrationController::SetExpired()
{
	state = CalibrationControllerStates::Exiting;
}


void CalibrationController::getCameraMatrices(Mat &camera, Mat& distortion)
{
	if (cameraMatrix != NULL && distortionMatrix != NULL)
	{
		camera = cameraMatrix->clone();
		distortion = distortionMatrix->clone();
	}
	else
	{
		LOGE("CalibrationController: Attempted to clone camera matrices, but matrices were NULL");
	}
}

Controller * CalibrationController::GetSuccessor(Engine * engine)
{
	LOGI(LOGTAG_CALIBRATION,"Calibration controller expired");
	//If the camera matrices were created correctly, then create a QR controller with them
	if (state == CalibrationControllerStates::Complete)
	{				
		LOGD(LOGTAG_CALIBRATION,"Calibration was completed successfully");
		Mat camera,distortion;
		getCameraMatrices(camera,distortion);
		Teardown(engine);
		return new ARController(camera,distortion, fovX);	
	}
	//Otherwise, create the controller using the predefined matrix
	else
	{
		LOGD(LOGTAG_CALIBRATION,"Calibration was not completed");
		Teardown(engine);
		return new ARController();
	}
}

vector<Point3f> CalibrationController::generateChessboardPoints(Size_<int> boardSize, float squareSize)
{
	vector<Point3f> points;

	for (int x = 0; x < boardSize.width; x++)
	{
		for (int y = 0; y < boardSize.height; y++)
		{
			Point3f point(squareSize * ((float) x), squareSize * ((float) y), 0);
			points.push_back(point);
		}
	}
	return points;
}

void CalibrationController::ProcessFrame(Engine* engine)
{
	if (!isInitialized)
		return;

	struct timespec start, end;
	
	engine->imageCollector->newFrame();	
	engine->imageCollector->getGrayCameraImage(*grayImage);
	ImageProcessor::SimpleThreshold(grayImage, binaryImage);	
	cvtColor(*grayImage, *rgbImage, CV_GRAY2RGBA, 4);

	if (state == CalibrationControllerStates::Finding)
	{
		

		vector<Point2f> corners;
		bool wasFound = findChessboardCorners(*grayImage, chessBoardSize, corners, CALIB_CB_FAST_CHECK);

		drawChessboardCorners(*rgbImage, chessBoardSize, corners, wasFound);

		if (wasFound)
		{
			cornerSubPix(*grayImage, corners, Size(10, 10), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
			objectPoints->push_back(generateChessboardPoints(chessBoardSize, sizeSpinner->GetValue()));
			imagePoints->push_back(corners);
			collectionCount++;
			state = CalibrationControllerStates::Running;
			setCaptureButton(captureButton,collectionCount);
			
		}
		else if (missCount++ > 5)
		{
			state = CalibrationControllerStates::Running;			
			setCaptureButton(captureButton,collectionCount);
		}
		
	}
	else if (state == CalibrationControllerStates::Calculating1)
	{
		calculateButton->SetText("Calculating...");
		calculateButton->SetFillColor(Colors::Gold);
		state = CalibrationControllerStates::Calculating2;
	}
	else if (state == CalibrationControllerStates::Calculating2)
	{
		CalculateMatrices();
	}		

	for (int i=0;i<drawObjects.size();i++)
	{
		drawObjects.at(i)->Draw(rgbImage);
	}

	quadBackground->SetImage(rgbImage);
}

void CalibrationController::CalculateMatrices()
{
	if (collectionCount < MIN_CALIBRATION_SAMPLES)
	{
		LOGI(LOGTAG_CALIBRATION,"Insufficient samples, exiting");
		state = CalibrationControllerStates::Exiting;
		return;
	}

	struct timespec start, end;
	LOGD(LOGTAG_CALIBRATION,"Calculating camera matrix");
	SET_TIME(&start);

	cameraMatrix = new Mat(3, 3, CV_64F);
	distortionMatrix = new Mat(1,5,CV_64F);

	vector<Mat> rotations, translations;

	calibrateCamera(*objectPoints, *imagePoints, grayImage->size(), *cameraMatrix, *distortionMatrix, rotations, translations, 0);
	SET_TIME(&end);
	LOG_TIME("Camera Matrix Generation", start, end);
	LOG_Mat(ANDROID_LOG_INFO,LOGTAG_CALIBRATION,"Camera Matrix",cameraMatrix);

	double fovx,fovy,focalLength,aspectRatio;
	Point2d principalPoint;
	calibrationMatrixValues(*cameraMatrix,grayImage->size(),1,1,fovx,fovy,focalLength,principalPoint,aspectRatio);
		
	LOGI(LOGTAG_CALIBRATION,"Calibration Complete");

	char labelStr[500];
	sprintf(labelStr,"FocalLength=%6.3lf - Principal[%4.1lf,%4.1lf]",fovx,fovy,focalLength,principalPoint.x,principalPoint.y,aspectRatio);
	fovLabel->SetText(labelStr);
	
	sprintf(labelStr,"FOV=[%5.2lf,%5.2lf] - AR=%3.2lf",fovx,fovy,focalLength,principalPoint.x,principalPoint.y,aspectRatio);
	principalLabel->SetText(labelStr);

	fovX = fovx;
	
	state = CalibrationControllerStates::Calculated;

	cameraMatDisplay->SetVisible(true);
	distortionMatDisplay->SetVisible(true);
	cameraMatDisplay->SetData(cameraMatrix);
	distortionMatDisplay->SetData(distortionMatrix);

	calculateButton->SetText("Accept");
	calculateButton->SetFillColor(Colors::DarkCyan);

	captureButton->SetText("Redo");
	captureButton->SetFillColor(Colors::Red);

}


void CalibrationController::Render(OpenGL * openGL)
{	
	if (!isInitialized)
		return;
	quadBackground->Render(openGL);
}