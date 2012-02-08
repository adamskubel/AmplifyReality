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

	layout = new GridLayout(Size2i(engine->imageWidth,engine->imageHeight),Size_<int>(5,4));

	//Create image capture button
	captureButton = new Button(std::string("Capture"),cv::Rect(300,300,150,160),Scalar(12,62,141,255));
	captureButton->AddClickDelegate(ClickEventDelegate::from_method<CalibrationController,&CalibrationController::HandleButtonClick>(this));	
	captureButton->Name = std::string("CaptureButton");
	layout->AddChild(captureButton,Point2i(3,3),Size_<int>(2,1));

	cameraMatDisplay = new DataDisplay("%6.2lf",Colors::Black,Colors::White);
	distortionMatDisplay = new DataDisplay("%3.2lf",Colors::Black,Colors::White);

	layout->AddChild(cameraMatDisplay,Point2i(0,0), Size2i(3,2));	
	layout->AddChild(distortionMatDisplay,Point2i(0,3),Size2i(3,1));

	//Create exit button
	calculateButton = new Button(std::string("Calculate"),Colors::Green);
	calculateButton->AddClickDelegate(ClickEventDelegate::from_method<CalibrationController,&CalibrationController::HandleButtonClick>(this));
	calculateButton->Name = std::string("CalculateButton");
	layout->AddChild(calculateButton,Point2i(3,0),Size_<int>(2,1));

	//Size adjuster
	sizeSpinner = new NumberSpinner("SquareSize(mm)",10,0.5,"%3.0f");
	layout->AddChild(sizeSpinner,Point2i(3,1),Size2i(2,2));

	//Set grid layout as the root UI element
	inputScaler = new InputScaler(engine->ImageSize(),engine->ScreenSize(),layout);

	engine->inputHandler->SetRootUIElement(inputScaler);
	drawObjects.push_back(layout);

	//Create background quad
	quadBackground = new QuadBackground(Size2i(engine->ImageSize()));

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
			imagePoints->clear();
			objectPoints->clear();
			state = CalibrationControllerStates::Running;

			cameraMatDisplay->SetVisible(false);
			distortionMatDisplay->SetVisible(false);

			calculateButton->label = "Calculate";
			captureButton->label = "Capture";
		}
		else if (state == CalibrationControllerStates::Running)
		{
			LOGI(LOGTAG_CALIBRATION,"Capturing by button");
			state = CalibrationControllerStates::Finding;
		}
		else if (state == CalibrationControllerStates::Finding)
		{
			//Abort capture attempt
			state = CalibrationControllerStates::Running;
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
		return new ARController(camera,distortion);	
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

	LOGV(LOGTAG_CALIBRATION,"Begin ProcessFrame");
	struct timespec start, end;
	
	engine->imageCollector->newFrame();	
	engine->imageCollector->getGrayCameraImage(*grayImage);
	ImageProcessor::SimpleThreshold(grayImage, binaryImage);	
	cvtColor(*grayImage, *rgbImage, CV_GRAY2RGBA, 4);

	if (state == CalibrationControllerStates::Finding)
	{
		vector<Point2f> corners;
		LOGD(LOGTAG_CALIBRATION,"Finding corners");
		bool wasFound = findChessboardCorners(*grayImage, chessBoardSize, corners, CALIB_CB_FAST_CHECK);

		LOGD(LOGTAG_CALIBRATION,"Drawing corners");
		drawChessboardCorners(*rgbImage, chessBoardSize, corners, wasFound);

		if (wasFound)
		{
			cornerSubPix(*grayImage, corners, Size(10, 10), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
			objectPoints->push_back(generateChessboardPoints(chessBoardSize, sizeSpinner->GetValue()));
			imagePoints->push_back(corners);
			collectionCount++;
			state = CalibrationControllerStates::Running;

			//Update label
			char myString[100];
			sprintf(myString, "Capture (%d)", collectionCount);
			captureButton->label = std::string(myString);
		}

		
	}
	else if (state == CalibrationControllerStates::Calculating1)
	{
		calculateButton->label = "Calculating...";
		calculateButton->FillColor = Colors::Gold;
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
	
	LOGV(LOGTAG_CALIBRATION,"End ProcessFrame");
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
	LOGI(LOGTAG_CALIBRATION,"Camera physical parameters: fovx=%lf,fovy=%lf,focalLength=%lf,PrincipalPoint=(%lf,%lf),aspectRatio=%lf",fovx,fovy,focalLength,principalPoint.x,principalPoint.y,aspectRatio);
		
	LOGI(LOGTAG_CALIBRATION,"Calibration Complete");

	
	state = CalibrationControllerStates::Calculated;

	cameraMatDisplay->SetVisible(true);
	distortionMatDisplay->SetVisible(true);
	cameraMatDisplay->SetData(cameraMatrix);
	distortionMatDisplay->SetData(distortionMatrix);

	calculateButton->label = "Accept";
	calculateButton->FillColor = Colors::MediumSeaGreen;
	captureButton->label = "Redo";

}


void CalibrationController::Render(OpenGL * openGL)
{	
	if (!isInitialized)
		return;
	quadBackground->Render(openGL);
}