#include "Controller.hpp"

#include <jni.h>
#include <vector>

#include <opencv2/core/core.hpp>
#include "opencv2/calib3d/calib3d.hpp"

#include "display/opengl/QuadBackground.hpp"

#include "datacollection/ImageCollector.hpp"
#include "datacollection/ImageProcessor.hpp"

#include "userinterface/uimodel/Button.hpp"
#include "userinterface/uimodel/GridLayout.hpp"
#include "userinterface/uimodel/InputScaler.hpp"
#include "userinterface/events/EventDelegates.hpp"
#include "userinterface/events/EventArgs.hpp"
#include "model/Drawable.hpp"




#ifndef CALIBRATIONCONTROLLER_HPP_
#define CALIBRATIONCONTROLLER_HPP_

using namespace cv;

class CalibrationController : public Controller
{
public:
	CalibrationController();
	~CalibrationController();
	void Initialize(Engine * engine);
	void Teardown(Engine * engine);
	void ProcessFrame(Engine * engine);
	bool IsExpired();
	bool SetExpired();
	bool wasSuccessful();
	void captureImage();
	void getCameraMatrices(Mat& camera, Mat& distortion);
	void HandleButtonClick(void * sender, EventArgs args);
	void Render(OpenGL * openGL);

private:
	static const int SampleCount = NUM_CALIBRATION_SAMPLES;

	vector<vector<Point3f> > * objectPoints;
	vector<vector<Point2f> > * imagePoints;
	int collectionCount;
	bool calibrationComplete, exitRequested;
	bool isFinding;
	Size_<int> chessBoardSize;
	Mat *distortionMatrix, * cameraMatrix;
	Button * myCaptureButton;
	QuadBackground * quadBackground;
	vector<Point3f> generateChessboardPoints(Size_<int> boardDimensions, float squareSize);

	cv::Mat *rgbImage, *binaryImage, *grayImage;

	vector<Drawable *> drawObjects;

	GridLayout * layout;
	InputScaler * inputScaler;
};

#endif
