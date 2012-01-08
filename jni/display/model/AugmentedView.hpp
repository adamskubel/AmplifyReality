#include "display/model/ARObject.hpp"
#include "display/opengl/GLObject.hpp"
#include "display/opengl/OpenGLRenderable.hpp"
#include "display/opengl/OpenGLHelper.hpp"
#include "model/Updateable.hpp"

#ifndef AUGMENTEDVIEW_HPP_
#define AUGMENTEDVIEW_HPP_

/*
Renders the virtual objects to be overlayed on the camera image. 
Uses input from locationing to determine where to display objects.
*/
class AugmentedView : public Updateable, public OpenGLRenderable
{
public:
	void Update(FrameItem * item);
	void Render(OpenGL * openGL);

	AugmentedView(cv::Mat * cameraMatrix);
	void AddObject(ARObject * arObject);

private:
	cv::Mat * cameraMatrix;
	cv::Mat * rotationMatrix, * translationMatrix;
	std::vector<ARObject *> objectVector;
	void SetCameraPosition();

};

#endif