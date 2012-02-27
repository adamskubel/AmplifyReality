#include "userinterface/uimodel/UIElement.hpp"
#include "userinterface/uimodel/InputScaler.hpp"
#include "userinterface/uimodel/GridLayout.hpp"
#include "userinterface/uimodel/Button.hpp"
#include "userinterface/uimodel/Label.hpp"
#include "userinterface/events/EventArgs.hpp"
#include "model/Drawable.hpp"
#include "Controller.hpp"
#include "model/IDeletable.hpp"
#include "display/opengl/QuadBackground.hpp"
#include <opencv2/core/core.hpp>
#include "CalibrationController.hpp"
#include "ARController.hpp"
#include "userinterface/uimodel/TextBox.hpp"
#include "userinterface/uimodel/TabDisplay.hpp"


#ifndef STARTUP_CONTROLLER_HPP_
#define STARTUP_CONTROLLER_HPP_

using namespace cv;


class StartupController : public Controller
{
public:
	StartupController();
	~StartupController();
	void ProcessFrame(Engine * engine);
	void Initialize(Engine * engine);
	void Teardown(Engine * engine);

	Controller * GetSuccessor(Engine * engine);
	
	void startButtonPress(void * sender, EventArgs args);
	void exitApplication(void * sender, EventArgs args);

	void Render(OpenGL * openGL);

	bool IsExpired();
	void SetExpired();

private:
	bool doCalibration;
	Button * startButton, * calibrateButton;
	Label * statusLabel;
	vector<IDeletable*> deleteVector;
	vector<Drawable*> drawObjects;
	GridLayout * grid;
	Mat * rgbaImage;
	bool isInitialized, isExpired, exitNext;
	QuadBackground * quadBackground;
	TextBox * password, * userName, * hostName;
	int frameCount;
	int dir;

	
};


#endif