#include "positioning/QRFinder.hpp"
#include "LogDefinitions.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "android_native_app_glue.h"
#include "display/NativeWindowRenderer.hpp"
#include "datacollection/ImageCollector.hpp"
#include "display/opengl/OpenGLRenderer.hpp"
#include "controllers/CalibrationController.hpp"
#include "positioning/QRLocator.hpp"

using namespace std;
using namespace cv;

enum DrawMode
{
	Color = 0, Gray = 1, Binary = 2
};

enum ActionMode
{
	QRTrack = 0, Calibrate = 1
};

int imageHeight = 480, imageWidth = 800;
int screenWidth = 960, screenHeight = 540;

DrawMode currentDrawMode = Gray;
ActionMode currentActionMode = QRTrack;

Mat * rgbImage, *binaryImage, *grayImage, *cameraMatrix;
struct timespec lastFrame;

void * currentController;

QRLocator * qrLocator;

//struct engine
//{
//	static const int imageHeight = 480;
//	static const int imageWidth = 800;
//	static const int screenWidth = 960;
//	static const int screenHeight = 540;
//	struct android_app* app;
//	int animating;
//	OpenGLRenderer glRender;
//	ImageCollector * imageCollector;
//};

void process_frame(struct engine* engine)
{
	LOGD("Frame Start");
	struct timespec start, end;

	engine->imageCollector->newFrame();
	if (currentDrawMode == Gray || currentDrawMode == Binary)
	{
		engine->imageCollector->getImage(&grayImage, ImageCollector::GRAY);
	} else
	{
		SET_TIME(&start);
		engine->imageCollector->getImage(&rgbImage, ImageCollector::RGBA);
		SET_TIME(&end);
		LOG_TIME("RGBA get", start, end);
	}

	SET_TIME(&start);
	engine->imageCollector->getImage(&binaryImage, ImageCollector::OTSU);
	SET_TIME(&end);
	LOG_TIME("Binary get", start, end);

	vector<Point_<int>*> v;
	vector<Point3i> vDebug;

	SET_TIME(&start);
	QRFinder::locateQRCode(*binaryImage, v, vDebug);
	SET_TIME(&end);
	LOG_TIME("QR Search", start, end);

	if (currentDrawMode == Gray)
	{
		SET_TIME(&start)
		cvtColor(*grayImage, *rgbImage, CV_GRAY2RGBA, 4);
		SET_TIME(&end);
		LOG_TIME("Gray->RGBA", start, end);
	} else if (currentDrawMode == Binary)
	{
		SET_TIME(&start)
		cvtColor(*binaryImage, *rgbImage, CV_GRAY2RGBA, 4);
		SET_TIME(&end);
		LOG_TIME("Binary->RGBA", start, end);
	}

	if (qrLocator != NULL && v.size() > 0)
	{
		LOGD("Unprojecting points");
		Mat rotation = Mat();
		Mat translation = Mat();
		qrLocator->transformPoints(v,10,rotation,translation);
	}

	for (size_t i = 0; i < v.size(); i++)
	{
		fillConvexPoly(*rgbImage, v[i], 4, Scalar(0, 0, 255, 255));
	}
	while (!v.empty())
	{
		delete v.back();
		v.pop_back();
	}
	for (size_t i = 0; i < vDebug.size(); i++)
	{
		circle(*rgbImage, Point2i((vDebug)[i].x, (vDebug)[i].y), (vDebug)[i].z, Scalar(255, 0, 0, 255), 1);
	}

	SET_TIME(&start);
	engine->glRender.render(screenWidth, screenHeight, imageWidth, imageHeight, rgbImage->ptr<uint32_t>(0));
	SET_TIME(&end);
	LOG_TIME("OpenGL Drawing", start, end);
	//NativeWindowRenderer::drawToBuffer(buffer, rgbImage);
}

void setActionMode(ActionMode newMode)
{
	if (currentActionMode == newMode)
		return;

	if (newMode == Calibrate)
	{
		currentController = new CalibrationController();
	} else if (currentActionMode == Calibrate)
	{
		CalibrationController* calibrationController = (CalibrationController*) (currentController);
		delete calibrationController;
	}

	currentActionMode = newMode;
}

void drawFrame(struct engine* engine)
{
	if (engine->app->window == NULL)
	{
		return;
	}

	switch (currentActionMode)
	{
	case QRTrack:
		process_frame(engine);
		break;
	case Calibrate:
		CalibrationController* calibrationController = (CalibrationController*) (currentController);
		calibrationController->findCorners(engine);
		if (calibrationController->isDone())
		{
			Mat distortionMatrix;
			double camData[9] = {4.31, 0, 0,0, 4.31, 0,0,0 ,1};
			Mat cameraMatrix = Mat(3,3,CV_64F,camData);
			calibrationController->getDistortionMatrix(distortionMatrix);
			LOGI("Creating QRLocator");
			qrLocator = new QRLocator(cameraMatrix,distortionMatrix);

			setActionMode(QRTrack);
		}
		break;
	}

	char myTimeString[100];
	struct timespec now;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
	sprintf(myTimeString, "Frame took %ld ms", calc_time(lastFrame,now));
	LOGT(myTimeString);

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &lastFrame);
}

void terminateDisplay(struct engine* engine)
{
	engine->animating = 0;
	engine->glRender.teardownOpenGL();
	engine->imageCollector->teardown();
	LOGI("Teardown successful. Goodbye!");
}

static void engineHandleCommand(struct android_app* app, int32_t cmd)
{
	struct engine* engine = (struct engine*) app->userData;
	switch (cmd)
	{
	case APP_CMD_INIT_WINDOW:
		if (engine->app->window != NULL)
		{
			LOGD("Importing OpenGL");
			engine->glRender.initOpenGL(engine->app->window, imageWidth, imageHeight);
			LOGD("Import complete");
			engine->animating = 1;
		}
		break;
	case APP_CMD_TERM_WINDOW:
		terminateDisplay(engine);
		break;
	case APP_CMD_LOST_FOCUS:
		engine->animating = 0;
		break;
	}
}

static int32_t engineHandleInput(struct android_app* app, AInputEvent* event)
{
	struct engine* engine = (struct engine*) app->userData;
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
	{
		if (((double) (AMotionEvent_getEventTime(event) / (1000000000LL))) > 0.5 && AMotionEvent_getAction(event) == AMOTION_EVENT_ACTION_UP)
		{
			switch (currentActionMode)
			{
			case QRTrack:
				currentDrawMode = (DrawMode) ((currentDrawMode + 1) % 3);
				break;
			case Calibrate:
				CalibrationController* calibrationController = (CalibrationController*) (currentController);
				calibrationController->captureImage();
				break;
			}

			return 1;
		}
	} else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY)
	{
		if (((double) (AKeyEvent_getEventTime(event) / (1000000000LL))) > 0.5 && AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_UP)
		{
			LOGI("Key event: action=%d keyCode=%d metaState=0x%x", AKeyEvent_getAction(event), AKeyEvent_getKeyCode(event), AKeyEvent_getMetaState(event));
			setActionMode((ActionMode) ((currentActionMode + 1) % 2));
		}
	}
	return 0;
}

void initializeEngine(struct engine* engine)
{
	//Mat myuv(height + height / 2, width, CV_8UC1);
	engine->imageCollector = new ImageCollector(imageWidth, imageHeight);
	rgbImage = new Mat(imageHeight, imageWidth, CV_8UC4);

}

void android_main(struct android_app* state)
{
	struct engine engine;

	app_dummy();

	memset(&engine, 0, sizeof(engine));

	state->userData = &engine;
	state->onAppCmd = engineHandleCommand;
	state->onInputEvent = engineHandleInput;

	engine.app = state;
	engine.animating = 0;

	initializeEngine(&engine);

	while (1)
	{

		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source* source;

		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		while ((ident = ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events, (void**) &source)) >= 0)
		{

			// Process this event.
			if (source != NULL)
			{
				source->process(state, source);
			}

			// Check if we are exiting.
			if (state->destroyRequested != 0)
			{
				LOGI("Engine thread destroy requested!");
				terminateDisplay(&engine);
				return;
			}
		}

		if (engine.animating)
		{
			drawFrame(&engine);
		}
	}

	engine.glRender.teardownOpenGL();

}

