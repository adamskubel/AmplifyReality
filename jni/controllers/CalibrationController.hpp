#include "Controller.hpp"

#include <jni.h>
#include <vector>

#include <opencv2/core/core.hpp>
#include "opencv2/calib3d/calib3d.hpp"

#include "display/opengl/OpenGLRenderer.hpp"
#include "datacollection/ImageCollector.hpp"
#include "datacollection/ImageProcessor.hpp"




#ifndef CALIBRATIONCONTROLLER_HPP_
#define CALIBRATIONCONTROLLER_HPP_

using namespace cv;

class CalibrationController : public Controller
{
public:
	CalibrationController();
	~CalibrationController();
	void ProcessFrame(Engine * engine, FrameItem * frame);
	bool isExpired();
	void captureImage();
	void getCameraMatrices(Mat& camera, Mat& distortion);

private:
	static const int SampleCount = NUM_CALIBRATION_SAMPLES;

	vector<vector<Point3f> > * objectPoints;
	vector<vector<Point2f> > * imagePoints;
	int collectionCount;
	bool calibrationComplete;
	bool isFinding;
	Size_<int> chessBoardSize;
	Mat * rgbImage, *binaryImage, *grayImage, *distortionMatrix, * cameraMatrix;

	vector<Point3f> generateChessboardPoints(int w, int h, float squareSize);
	void drawImageCount(Mat * img);
};

#endif
