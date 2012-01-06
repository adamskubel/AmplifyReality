#include "CalibrationController.hpp"

CalibrationController::CalibrationController()
{
	objectPoints = new vector<vector<Point3f> >();
	imagePoints = new vector<vector<Point2f> >();
	collectionCount = 0;
	calibrationComplete = false;
	chessBoardSize = Size_<int>(7, 7);
	LOGI(LOGTAG_CALIBRATION,"Calibration Controller Initialized");
}

CalibrationController::~CalibrationController()
{
	delete objectPoints;
	delete imagePoints;
}

void CalibrationController::captureImage()
{
	isFinding = true;
}

bool CalibrationController::isExpired()
{
	return calibrationComplete;
}

void CalibrationController::getCameraMatrices(Mat &camera, Mat& distortion)
{
	camera = cameraMatrix->clone();
	distortion = distortionMatrix->clone();
}

vector<Point3f> CalibrationController::generateChessboardPoints(int w, int h, float squareSize)
{
	vector<Point3f> points;

	for (int x = 0; x < w; x++)
	{
		for (int y = 0; y < h; y++)
		{
			Point3f point(squareSize * ((float) x), squareSize * ((float) y), 0);
			points.push_back(point);
		}
	}
	return points;
}

void CalibrationController::ProcessFrame(Engine* engine, FrameItem * item)
{
	LOGV(LOGTAG_CALIBRATION,"Begin ProcessFrame");
	struct timespec start, end;
	
	engine->imageCollector->newFrame();	
	engine->imageCollector->getGrayCameraImage(*(item->grayImage));
	ImageProcessor::SimpleThreshold(item);	
	cvtColor(*(item->grayImage), *(item->rgbImage), CV_GRAY2RGBA, 4);

	if (isFinding)
	{
		vector<Point2f> corners;
		LOGD(LOGTAG_CALIBRATION,"Finding corners");
		bool wasFound = findChessboardCorners(*(item->grayImage), chessBoardSize, corners, CALIB_CB_FAST_CHECK);

		LOGD(LOGTAG_CALIBRATION,"Drawing corners");
		drawChessboardCorners(*(item->rgbImage), chessBoardSize, corners, wasFound);

		if (wasFound)
		{
			cornerSubPix(*(item->grayImage), corners, Size(10, 10), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
			objectPoints->push_back(generateChessboardPoints(7, 7, 25.0f));
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

			calibrateCamera(*objectPoints, *imagePoints, (item->grayImage)->size(), *cameraMatrix, *distortionMatrix, rotations, translations, 0);
			SET_TIME(&end);
			LOG_TIME("Camera Matrix Generation", start, end);
			LOGI_Mat(LOGTAG_CALIBRATION,"Camera Matrix",cameraMatrix);

			calibrationComplete = true;
		}
	}

	drawImageCount((item->rgbImage));
	LOGV(LOGTAG_CALIBRATION,"End ProcessFrame");
}

void CalibrationController::drawImageCount(Mat * img)
{
	char myString[100];
	sprintf(myString, "%d/%d", collectionCount, SampleCount);
	int fontFace = FONT_HERSHEY_SCRIPT_SIMPLEX;
	double fontScale = 2;
	int thickness = 3;
	int baseline = 0;
	Size textSize = getTextSize(myString, fontFace, fontScale, thickness, &baseline);
	baseline += thickness;

	Point textOrg((img->cols - textSize.width) / 2, (img->rows - textSize.height) / 2);
	putText(*img, myString, textOrg, fontFace, fontScale, Scalar::all(255), thickness, 8);

}


//			for (int i = 0; i < objectPoints->size(); i++)
//			{
//				vector<Point3f> vec = objectPoints->at(i);
//				for (int j = 0; j < vec.size(); j++)
//				{
//					Point3f point = vec.at(j);
//					LOGI("Point[%f,%f,%f]", point.x, point.y, point.z);
//				}
//			}
//
//			for (int i = 0; i < imagePoints->size(); i++)
//			{
//				vector<Point2f> vec = imagePoints->at(i);
//				for (int j = 0; j < vec.size(); j++)
//				{
//					Point2f point = vec.at(j);
//					LOGI("Point[%f,%f]", point.x, point.y);
//				}
//			}
