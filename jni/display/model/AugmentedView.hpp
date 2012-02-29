#include "display/model/ARObject.hpp"
#include "display/opengl/GLObject.hpp"
#include "display/opengl/OpenGLRenderable.hpp"
#include "display/opengl/OpenGLHelper.hpp"
#include "model/Updateable.hpp"
#include "model/network/RealmDefinition.hpp"
#include "userinterface/events/EventArgs.hpp"
#include "userinterface/events/EventDelegates.hpp"

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
	Mat startPosition;
	Mat startRotation;
	Point3f originalPosition;
	Point3f originalRotation;
};

/*
Renders the virtual objects to be overlayed on the camera image. 
Uses input from locationing to determine where to display objects.
*/
class AugmentedView 
{
public:
	void Update(Engine * engine, FrameItem * item);
	void Render(OpenGL * openGL);

	AugmentedView(cv::Mat cameraMatrix);
	~AugmentedView();
	void AddObject(ARObject * arObject);

	void HandleTouchInput(void * sender, TouchEventArgs args);
	void SetFOV(float fov);
	
private:
	cv::Mat * cameraMatrix;
	cv::Mat * rotation, * position;
	std::vector<ARObject *> objectVector;
	void SetCameraPosition(OpenGLRenderData & renderData);
	bool canDraw;
	vector<Point2i> inputPoints;
	Mat projection;
	float fieldOfView;
	ARObject * selectionIndicator, * testObject;
	SelectedObject * selectedObject;

	struct timespec lastSelectionTime;

	map<std::string,ARObjectMessage*> updateObjectMap;
	

};

#endif