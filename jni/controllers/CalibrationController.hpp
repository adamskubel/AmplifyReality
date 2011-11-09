#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"

#include <jni.h>
#include <vector>

#include <opencv2/core/core.hpp>
#include "opencv2/calib3d/calib3d.hpp"

#include "display/opengl/OpenGLRenderer.hpp"
#include "datacollection/ImageCollector.hpp"


using namespace cv;

#ifndef CALIBRATIONCONTROLLER_HPP_
#define CALIBRATIONCONTROLLER_HPP_



struct engine
{
	static const int imageHeight = 480;
	static const int imageWidth = 800;
	static const int screenWidth = 960;
	static const int screenHeight = 540;
	struct android_app* app;
	int animating;
	OpenGLRenderer glRender;
	ImageCollector * imageCollector;
};


class CalibrationController
{
	static const int SampleCount = 10;

	vector<vector<Point3f> > * objectPoints;
	vector<vector<Point2f> > * imagePoints;
	int collectionCount;
	bool calibrationComplete;
	bool isFinding;
	Size_<int> chessBoardSize;
	Mat * rgbImage, *binaryImage, *grayImage;

public:
	CalibrationController();
	~CalibrationController();
	void findCorners(struct engine* engine);
	bool isDone();
	void captureImage();

private:
	vector<Point3f> generateChessboardPoints(int w, int h, float squareSize);
	void drawImageCount(Mat * img);
};

#endif
