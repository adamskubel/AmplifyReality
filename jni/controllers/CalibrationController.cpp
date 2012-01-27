#include "CalibrationController.hpp"

CalibrationController::CalibrationController()
{
	objectPoints = new vector<vector<Point3f> >();
	imagePoints = new vector<vector<Point2f> >();
	collectionCount = 0;
	chessBoardSize = Size_<int>(7, 7);

	drawObjects = vector<Drawable*>();

	exitRequested = false;
	isFinding = false;
	calibrationComplete = false;
	isInitialized = false;

	rgbImage = new Mat();
	binaryImage = new Mat();
	grayImage = new Mat();

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
	myCaptureButton = new Button(std::string("Capture"),cv::Rect(300,300,150,160),Scalar(12,62,141,255));
	myCaptureButton->AddClickDelegate(ClickEventDelegate::from_method<CalibrationController,&CalibrationController::HandleButtonClick>(this));	
	myCaptureButton->Name = std::string("CaptureButton");
	layout->AddChild(myCaptureButton,Point2i(3,3),Size_<int>(2,1));

	//Create exit button
	Button * exitButton = new Button(std::string("Exit"),cv::Rect(300,300,150,160),Scalar(255,0,0,255));
	exitButton->AddClickDelegate(ClickEventDelegate::from_method<CalibrationController,&CalibrationController::HandleButtonClick>(this));
	exitButton->Name = std::string("ExitButton");
	layout->AddChild(exitButton,Point2i(4,0),Size_<int>(1,1));

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
		LOGI(LOGTAG_CALIBRATION,"Capturing by button");
		isFinding = true;
	}
	else if (button->Name.compare("ExitButton") == 0)
	{
		LOGI(LOGTAG_CALIBRATION,"Exiting by button");
		exitRequested = true;
	}
}

bool CalibrationController::IsExpired()
{
	return calibrationComplete || exitRequested;
}

void CalibrationController::SetExpired()
{
	exitRequested = true;
}

bool CalibrationController::wasSuccessful()
{
	return calibrationComplete;
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

	if (isFinding)
	{
		vector<Point2f> corners;
		LOGD(LOGTAG_CALIBRATION,"Finding corners");
		bool wasFound = findChessboardCorners(*grayImage, chessBoardSize, corners, CALIB_CB_FAST_CHECK);

		LOGD(LOGTAG_CALIBRATION,"Drawing corners");
		drawChessboardCorners(*rgbImage, chessBoardSize, corners, wasFound);

		if (wasFound)
		{
			cornerSubPix(*grayImage, corners, Size(10, 10), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
			objectPoints->push_back(generateChessboardPoints(chessBoardSize, 11.5f));
			imagePoints->push_back(corners);
			collectionCount++;
			isFinding = false;
		}

		if (collectionCount >= SampleCount)
		{
			LOGD(LOGTAG_CALIBRATION,"Calculate camera matrix");
			SET_TIME(&start);

			cameraMatrix = new Mat(3, 3, CV_64F);
			distortionMatrix = new Mat(3,3,CV_64F);
			vector<Mat> rotations, translations;

			calibrateCamera(*objectPoints, *imagePoints, grayImage->size(), *cameraMatrix, *distortionMatrix, rotations, translations, 0);
			SET_TIME(&end);
			LOG_TIME("Camera Matrix Generation", start, end);
			LOG_Mat(ANDROID_LOG_INFO,LOGTAG_CALIBRATION,"Camera Matrix",cameraMatrix);

			double fovx,fovy,focalLength,aspectRatio;
			Point2d principalPoint;
			calibrationMatrixValues(*cameraMatrix,grayImage->size(),1,1,fovx,fovy,focalLength,principalPoint,aspectRatio);
			LOGI(LOGTAG_CALIBRATION,"Camera physical parameters: fovx=%lf,fovy=%lf,focalLength=%lf,PrincipalPoint=(%lf,%lf),aspectRatio=%lf",fovx,fovy,focalLength,principalPoint.x,principalPoint.y,aspectRatio);

			calibrationComplete = true;
			LOGI(LOGTAG_CALIBRATION,"Calibration Complete");
		}
	}
	
	char myString[100];
	sprintf(myString, "Capture (%d/%d)", collectionCount, SampleCount);
	myCaptureButton->label = std::string(myString);

	for (int i=0;i<drawObjects.size();i++)
	{
		drawObjects.at(i)->Draw(rgbImage);
	}

	quadBackground->SetImage(rgbImage);
	
	LOGV(LOGTAG_CALIBRATION,"End ProcessFrame");
}


void CalibrationController::Render(OpenGL * openGL)
{	
	if (!isInitialized)
		return;
	quadBackground->Render(openGL);
}