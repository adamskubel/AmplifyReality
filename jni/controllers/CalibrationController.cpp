#include "CalibrationController.hpp"

CalibrationController::CalibrationController()
{
	objectPoints = new vector<vector<Point3f> >();
	imagePoints = new vector<vector<Point2f> >();
	collectionCount = 0;
	calibrationComplete = false;
	chessBoardSize = Size_<int>(7, 7);

	rgbImage = new Mat(1, 1, CV_8UC4);

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

bool CalibrationController::isDone()
{
	return calibrationComplete;
}

void CalibrationController::getDistortionMatrix(Mat &mat)
{
	mat = distortionMatrix->clone();
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

void CalibrationController::findCorners(struct engine* engine)
{
	LOGD("Find corners Start");
	struct timespec start, end;

	engine->imageCollector->newFrame();
	engine->imageCollector->getImage(&grayImage, ImageCollector::GRAY);

	SET_TIME(&start);
	engine->imageCollector->getImage(&binaryImage, ImageCollector::OTSU);
	SET_TIME(&end);
	LOG_TIME("Binary get", start, end);

	SET_TIME(&start)
	cvtColor(*grayImage, *rgbImage, CV_GRAY2RGBA, 4);
	SET_TIME(&end);
	LOG_TIME("Gray->RGBA", start, end);

	if (isFinding)
	{
		vector<Point2f> corners;
		LOGD("Finding corners");
		bool wasFound = findChessboardCorners(*grayImage, chessBoardSize, corners, CALIB_CB_FAST_CHECK);

		LOGD("Drawing corners");
		drawChessboardCorners(*rgbImage, chessBoardSize, corners, wasFound);

		if (wasFound)
		{
			cornerSubPix(*grayImage, corners, Size(10, 10), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
			objectPoints->push_back(generateChessboardPoints(7, 7, 25.0f));
			imagePoints->push_back(corners);
			collectionCount++;
			isFinding = false;
		}

		if (collectionCount >= SampleCount)
		{
			LOGD("Calculate camera matrix");
			SET_TIME(&start);

			Mat cameraMatrix;
			cameraMatrix = Mat::zeros(3, 3, CV_64F);
			distortionMatrix = new Mat(3,3,CV_64F);
			vector<Mat> rotations, translations;

			calibrateCamera(*objectPoints, *imagePoints, grayImage->size(), cameraMatrix, *distortionMatrix, rotations, translations, 0);
			SET_TIME(&end);
			LOG_TIME("Camera Matrix Generation", start, end);
			LOGI("Camera matrix: [%lf,%lf,%lf;%lf,%lf,%lf;%lf,%lf,%lf]", cameraMatrix.at<double>(0,0), cameraMatrix.at<double>(0,1), cameraMatrix.at<double>(0,2), cameraMatrix.at<double>(1,0), cameraMatrix.at<double>(1,1), cameraMatrix.at<double>(1,2), cameraMatrix.at<double>(2,0), cameraMatrix.at<double>(2,1), cameraMatrix.at<double>(2,2));

			double fovx,fovy,aspectRatio,focalLength;
			Point2d principalPoint;
			double apX = 4.13,apY = 2.30;

			calibrationMatrixValues(cameraMatrix,grayImage->size(),apX,apY,fovx,fovy,focalLength,principalPoint,aspectRatio);

			LOGI("FOVx = %lf, FOVy = %lf, AR = %lf, FocalLength=%lf, PrincPoint=(%lf,%lf)",fovx,fovy,aspectRatio,focalLength,principalPoint.x,principalPoint.y);

			calibrationComplete = true;
		}
	}

	drawImageCount(rgbImage);

	SET_TIME(&start);
	engine->glRender.render(engine->screenWidth, engine->screenHeight, engine->imageWidth, engine->imageHeight, rgbImage->ptr<uint32_t>(0));
	SET_TIME(&end);
	LOG_TIME("OpenGL Drawing", start, end);
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
