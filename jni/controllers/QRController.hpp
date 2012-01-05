#include "Controller.hpp"

#include "model/FrameItem.hpp"
#include "model/Engine.hpp"

#include "datacollection/ImageCollector.hpp"
#include "datacollection/ImageProcessor.hpp"

#include "positioning/QRLocator.hpp"
#include "positioning/QRFinder.hpp"

#ifndef QRCONTROLLER_HPP_
#define QRCONTROLLER_HPP_

using namespace cv;
using namespace std;

class QRController : public Controller
{
public:
	QRController();
	void ProcessFrame(Engine * engine, FrameItem * item);

private:
	void drawDebugOverlay(FrameItem * item);
	void getImages(Engine* engine, FrameItem * item);
	void locateCodes(Engine* engine, FrameItem * item);
	
	QRLocator * qrLocator;

};

#endif
