#include "Controller.hpp"

#include "model/FrameItem.hpp"
#include "model/Engine.hpp"
#include "model/WorldLoader.hpp"

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

#include "display/opengl/QuadBackground.hpp"

#include "datastructures/CircularList.hpp"

#include "ARControllerDebugUI.hpp"

#include "positioning/fast/FastQRFinder.hpp"

#ifndef ARController_HPP_
#define ARController_HPP_

using namespace cv;
using namespace std;


namespace ControllerStates
{
	enum ControllerState
	{
		Loading,
		Running
	};
}


class ARController : public Controller, private Drawable //,public OpenGLRenderable
{
public:
	ARController(Mat camera = Mat(), Mat distortion = Mat());
	~ARController();
	void ProcessFrame(Engine * engine);
	void Initialize(Engine * engine);
	void Teardown(Engine * engine);
	
	void Render(OpenGL * openGL);

private:

	void drawDebugOverlay(FrameItem * item);
	void getImages(Engine* engine);
	void locateCodes(Engine* engine, FrameItem * item);	
	void readGyroData(Engine * engine, FrameItem * item);
	void initializeARView();
	void initializeUI(Engine * engine);
	
	void SetState(ControllerStates::ControllerState state);

	void Draw(Mat * rgbaImage);

	std::vector<Drawable*> drawObjects;
	std::vector<OpenGLRenderable*> renderObjects;
	std::vector<IDeletable*> deletableObjects;

	QRLocator * qrLocator;
	QRFinder * qrFinder;
	AugmentedView * augmentedView;
	WorldLoader * worldLoader;
	FastQRFinder * fastQRFinder;

	//UI objects
	ARControllerDebugUI * debugUI;

	//Return a rectangle that centered on the centroid of the given points, with a length and width given by borderSize
	Rect createWindow(Point_<int> * points, int borderSize);
	
	QuadBackground * quadBackground;

	cv::Mat *rgbImage, *binaryImage, *grayImage;
		
	bool isExpired, isInitialized;

	ControllerStates::ControllerState controllerState;

	PositionSelector * positionSelector;

	CircularList<FrameItem*> * frameList;

	int frameCount;
	float fpsAverage;
	struct timespec lastFrameTime;


};




#endif
