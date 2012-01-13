#include "Controller.hpp"

#include "model/FrameItem.hpp"
#include "model/Engine.hpp"

#include "display/model/AugmentedView.hpp"
#include "display/model/ARObject.hpp"
#include "display/opengl/OpenGLRenderable.hpp"

#include "datacollection/ImageCollector.hpp"
#include "datacollection/ImageProcessor.hpp"

#include "positioning/QRLocator.hpp"
#include "positioning/QRFinder.hpp"

#include "userinterface/uimodel/Label.hpp"
#include "userinterface/uimodel/GridLayout.hpp"

#ifndef LocationController_HPP_
#define LocationController_HPP_

using namespace cv;
using namespace std;

class LocationController : public Controller //,public OpenGLRenderable
{
public:
	LocationController();
	LocationController(Mat camera, Mat distortion);
	~LocationController();
	void ProcessFrame(Engine * engine, FrameItem * item);
	void Initialize(Engine * engine);
	//OpenGLRenderable Implementation
	void Render(OpenGL * openGL);


private:
	void drawDebugOverlay(FrameItem * item);
	void getImages(Engine* engine, FrameItem * item);
	void locateCodes(Engine* engine, FrameItem * item);	
	void readGyroData(Engine * engine, FrameItem * item);
	void initializeARView();

	std::vector<Updateable*> updateObjects;
	std::vector<OpenGLRenderable*> renderObjects;

	QRLocator * qrLocator;
	Label * translationVectorLabel, * gyroDataLabel;
	//AugmentedView * augmentedView;

	//Return a rectangle that centered on the centroid of the given points, with a length and width given by borderSize
	Rect createWindow(Point_<int> * points, int borderSize);

	Mat * defaultPosition, * defaultRotation;

};

#endif
