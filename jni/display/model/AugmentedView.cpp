#include "display/model/AugmentedView.hpp"

AugmentedView::AugmentedView(cv::Mat * _cameraMatrix)
{
	cameraMatrix = _cameraMatrix;
	
}


void AugmentedView::AddObject(ARObject * arObject)
{
	objectVector.push_back(arObject);
}

void AugmentedView::Render(OpenGL * openGL)
{
	for (int i=0;i<objectVector.size();i++)
	{
		ARObject * object = objectVector.at(i);

		glTranslatef(object->position.x,object->position.y,object->position.z);

		glRotatef(object->rotation.x,1,0,0);
		glRotatef(object->rotation.y,0,1,0);
		glRotatef(object->rotation.z,0,0,1);
	
		openGL->DrawGLObject(object);
	}
}

void AugmentedView::Update(FrameItem * item)
{
	rotationMatrix = item->rotationMatrix;
	translationMatrix = item->translationMatrix;
}

void AugmentedView::SetCameraPosition()
{
	glMatrixMode(GL_PROJECTION);
	
	glLoadIdentity();	

	OpenGLHelper::gluPerspective(PI/2.0f,1.6f,-10.0f,10.0f);

	glTranslatef(translationMatrix->at<float>(0,0), translationMatrix->at<float>(0,1),translationMatrix->at<float>(0,2));

	glRotatef(rotationMatrix->at<float>(0,0),1,0,0);
	glRotatef(rotationMatrix->at<float>(0,1),0,1,0);
	glRotatef(rotationMatrix->at<float>(0,2),0,0,1);

	
}