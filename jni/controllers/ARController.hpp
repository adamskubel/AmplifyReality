#include "Controller.hpp"

#include "model/FrameItem.hpp"
#include "model/Engine.hpp"

#include "display/model/AugmentedView.hpp"
#include "display/model/ARObject.hpp"
#include "display/opengl/OpenGLRenderable.hpp"
#include "display/objloader/objLoader.h"

#include "model/Drawable.hpp"

#include "datacollection/ImageCollector.hpp"
#include "datacollection/ImageProcessor.hpp"

#include "positioning/qrcode/QRLocator.hpp"
#include "positioning/qrcode/QRFinder.hpp"
#include "positioning/PositionSelector.hpp"

#include "userinterface/uimodel/Label.hpp"
#include "userinterface/uimodel/CertaintyIndicator.hpp"
#include "userinterface/uimodel/GridLayout.hpp"
#include "userinterface/uimodel/InputScaler.hpp"
#include "userinterface/events/EventDelegates.hpp"

#include "positioning/ARConfigurator.hpp"

#include "display/opengl/QuadBackground.hpp"

#ifndef ARController_HPP_
#define ARController_HPP_

using namespace cv;
using namespace std;


#include "userinterface/uimodel/DataDisplay.hpp"
#include "userinterface/uimodel/PageDisplay.hpp"

class ARControllerDebugUI : public PageDisplay
{
public:
	ARControllerDebugUI(Engine * engine, Point2i position);
	void SetTranslation(Mat * data);
	void SetRotation(Mat * data);
	void SetPositionCertainty(float certainty);

private:
	DataDisplay * translationLabel;
	DataDisplay * rotationLabel;
	CertaintyIndicator * certaintyIndicator;
	GridLayout * myGrid;


};


class ARController : public Controller //,public OpenGLRenderable
{
public:
	ARController();
	ARController(Mat camera, Mat distortion);
	~ARController();
	void ProcessFrame(Engine * engine);
	void Initialize(Engine * engine);
	void Teardown(Engine * engine);

	void Render(OpenGL * openGL);
	void HandleTouchInput(void* sender, TouchEventArgs args);
	void IncreaseQRScore(void * sender, EventArgs args);
	void DecreaseQRScore(void * sender, EventArgs args);

private:
	//Constants
	static const int numItems = 6; //Number of previous frames to store
	//int minQRScore;

	void drawDebugOverlay(FrameItem * item);
	void getImages(Engine* engine, FrameItem * item);
	void locateCodes(Engine* engine, FrameItem * item);	
	void readGyroData(Engine * engine, FrameItem * item);
	void initializeARView();

	std::vector<Drawable*> drawObjects;
	std::vector<OpenGLRenderable*> renderObjects;
	std::vector<IDeletable*> deletableObjects;

	QRLocator * qrLocator;
	AugmentedView * augmentedView;

	ARConfigurator * config;
	//UI objects
	ARControllerDebugUI * debugUI;

	//Return a rectangle that centered on the centroid of the given points, with a length and width given by borderSize
	Rect createWindow(Point_<int> * points, int borderSize);
	
	QuadBackground * quadBackground;
	Mat * defaultPosition, * defaultRotation;

	cv::Mat *rgbImage, *binaryImage, *grayImage;
		
	FrameItem ** items;	
	int currentFrameItem;
	bool isExpired, isInitialized;

	PositionSelector * positionSelector;

	Configuration::DrawMode drawMode;

};




#endif
