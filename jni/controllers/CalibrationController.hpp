#include "Controller.hpp"

#include <jni.h>
#include <vector>

#include <opencv2/core/core.hpp>
#include "opencv2/calib3d/calib3d.hpp"

#include "datacollection/ImageCollector.hpp"
#include "datacollection/ImageProcessor.hpp"

#include "userinterface/uimodel/Button.hpp"
#include "userinterface/uimodel/GridLayout.hpp"
#include "userinterface/events/EventDelegates.hpp"
#include "userinterface/events/EventArgs.hpp"





#ifndef CALIBRATIONCONTROLLER_HPP_
#define CALIBRATIONCONTROLLER_HPP_

using namespace cv;

class CalibrationController : public Controller
{
public:
	CalibrationController();
	~CalibrationController();
	void Initialize(Engine * engine);
	void ProcessFrame(Engine * engine, FrameItem * frame);
	bool isExpired();
	bool wasSuccessful();
	void captureImage();
	void getCameraMatrices(Mat& camera, Mat& distortion);
	void HandleButtonClick(void * sender, EventArgs args);

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

	vector<Point3f> generateChessboardPoints(Size_<int> boardDimensions, float squareSize);

	vector<Updateable *> updateObjects;
};

#endif
