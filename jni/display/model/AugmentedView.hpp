#include "display/model/ARObject.hpp"
#include "display/opengl/GLObject.hpp"
#include "display/opengl/OpenGLRenderable.hpp"
#include "display/opengl/OpenGLHelper.hpp"
#include "model/Updateable.hpp"
#include "model/network/RealmDefinition.hpp"
#include "userinterface/events/EventArgs.hpp"
#include "userinterface/events/EventDelegates.hpp"
#include "userinterface/uimodel/UIElementCollection.hpp"
#include "userinterface/uimodel/GridLayout.hpp"
#include "userinterface/uimodel/Button.hpp"
#include "userinterface/uimodel/TabDisplay.hpp"
#include <opencv2/calib3d/calib3d.hpp>

#ifndef AUGMENTEDVIEW_HPP_
#define AUGMENTEDVIEW_HPP_


struct SelectedObject
{
	SelectedObject(ARObject * _arObject, struct timespec _selectTime)
	{
		arObject = _arObject;
		selectTime = _selectTime;
	}

	ARObject * arObject;
	struct timespec selectTime;
	Point3f objectPositionDelta;
	Point3f objectRotationDelta;
	
	Point3f originalPosition;
	Point3f originalRotation;
	Mat originalProjectionMat;
};

/*
Renders the virtual objects to be overlayed on the camera image. 
Uses input from locationing to determine where to display objects.
*/
class AugmentedView 
{
public:
	AugmentedView(UIElementCollection * window, Engine * engine, cv::Mat cameraMatrix);
	~AugmentedView();

	void SetTransformations(Mat * position, Mat * rotation, Mat gyroData);
	void Update(Mat * rgbaImage, Engine * engine);
	void Render(OpenGL * openGL);

	void AddObject(ARObject * arObject);

	void HandleTouchInput(void * sender, TouchEventArgs args);
	void HandleButtonPress(void*,PhysicalButtonEventArgs);
	void SetFOV(float fov);

	void ButtonPressed(void * sender, EventArgs args);
	
private:
	cv::Mat * cameraMatrix;
	cv::Mat * rotation, * position;
	cv::Mat gyroRotation;
	std::vector<ARObject *> objectVector;
	bool canDraw, createNext;
	vector<Point2i> inputPoints;
	Mat projection;
	float fieldOfView;
	ARObject * selectionIndicator, * testObject;
	SelectedObject * selectedObject;
	TabDisplay * tabs;
	Button *cancelSelection, *releaseSelection;

	struct timespec lastSelectionTime, cursorShowTime;
	bool unselectNext, closeNext,farNext;
	map<std::string,ARObjectMessage*> updateObjectMap;
	

	//Members
	void UnselectObject();
	void SetCameraPosition(OpenGLRenderData & renderData);
	void UpdateObjectPosition(Mat & projection, SelectedObject * selectedObject);
	SelectedObject * SelectObjects(OpenGL * openGL);
	Point3f getCameraRotation();
};

#endif