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
#include "userinterface/uimodel/DataDisplay.hpp"

#include "model/Drawable.hpp"
#include "ARController.hpp"



#ifndef CALIBRATIONCONTROLLER_HPP_
#define CALIBRATIONCONTROLLER_HPP_

using namespace cv;

namespace CalibrationControllerStates
{
	enum CalibrationControllerState
	{
		Running,
		Calculating1,
		Calculating2,
		Calculated,
		Finding,
		Exiting,
		Complete
	};
}

class CalibrationController : public Controller
{
public:
	CalibrationController();
	~CalibrationController();
	void Initialize(Engine * engine);
	void Teardown(Engine * engine);
	void ProcessFrame(Engine * engine);
	bool IsExpired();
	void SetExpired();
	void captureImage();
	void getCameraMatrices(Mat& camera, Mat& distortion);
	void HandleButtonClick(void * sender, EventArgs args);
	void Render(OpenGL * openGL);
	Controller * GetSuccessor(Engine * engine);
	void CalculateMatrices();

private:
	CalibrationControllerStates::CalibrationControllerState state;

	vector<vector<Point3f> > * objectPoints;
	vector<vector<Point2f> > * imagePoints;
	int collectionCount, missCount;
	bool isInitialized;
	Size_<int> chessBoardSize;
	Mat *distortionMatrix, * cameraMatrix;
	double fovX;
	Button * captureButton, * calculateButton;
	QuadBackground * quadBackground;
	vector<Point3f> generateChessboardPoints(Size_<int> boardDimensions, float squareSize);
	NumberSpinner * sizeSpinner;
	DataDisplay * cameraMatDisplay, * distortionMatDisplay;
	Label * fovLabel, * principalLabel;

	cv::Mat *rgbImage, *binaryImage, *grayImage;

	vector<Drawable *> drawObjects;

	GridLayout * layout;
	InputScaler * inputScaler;
};

#endif
