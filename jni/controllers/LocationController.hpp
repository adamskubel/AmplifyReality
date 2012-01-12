#include "Controller.hpp"

#include "model/FrameItem.hpp"
#include "model/Engine.hpp"

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

class LocationController : public Controller
{
public:
	LocationController();
	LocationController(Mat camera, Mat distortion);
	~LocationController();
	void ProcessFrame(Engine * engine, FrameItem * item);
	void Initialize(Engine * engine);

private:
	void drawDebugOverlay(FrameItem * item);
	void getImages(Engine* engine, FrameItem * item);
	void locateCodes(Engine* engine, FrameItem * item);	
	void readGyroData(Engine * engine, FrameItem * item);
	std::vector<Updateable*> updateObjects;
	QRLocator * qrLocator;
	Label * translationVectorLabel, * gyroDataLabel;

	//Return a rectangle that centered on the centroid of the given points, with a length and width given by borderSize
	Rect createWindow(Point_<int> * points, int borderSize);

};

#endif
