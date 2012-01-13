#include "display/model/AugmentedView.hpp"

AugmentedView::AugmentedView(cv::Mat _cameraMatrix)
{
	cameraMatrix = new Mat();
	_cameraMatrix.copyTo(*cameraMatrix);

	LOGI(LOGTAG_POSITION, "Created AugmentedView");
}

//Delete the camera matrix and the vector of AR objects
AugmentedView::~AugmentedView()
{
	delete cameraMatrix;
	while (!objectVector.empty())
	{
		delete objectVector.back();
		objectVector.pop_back();
	}
	LOGI(LOGTAG_POSITION, "AugmentedView deleted successfully");
}

void AugmentedView::AddObject(ARObject * arObject)
{
	objectVector.push_back(arObject);
	LOGD(LOGTAG_POSITION, "Added new ARObject to AugmentedView");
}

void AugmentedView::Render(OpenGL * openGL)
{
	struct timespec start,end;
	SET_TIME(&start);

	SetCameraPosition();
	
	glMatrixMode(GL_MODELVIEW);

	
	glEnableClientState(GL_COLOR_ARRAY);

	glEnable(GL_DEPTH_TEST); 
	glEnable(GL_CULL_FACE); 
	glCullFace(GL_BACK);	
	glDisable(GL_LIGHTING);
	
	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_SRC_COLOR);	
	
	for (int i=0;i<objectVector.size();i++)
	{		
		glPushMatrix();
		ARObject * object = objectVector.at(i);

		//LOGV(LOGTAG_OPENGL,"Drawing ARObject at (%f,%f,%f)",object->position.x,object->position.y,object->position.z);
		glTranslatef(object->position.x,object->position.y,object->position.z);

		glRotatef(object->rotation.x,1,0,0);
		glRotatef(object->rotation.y,0,1,0);
		glRotatef(object->rotation.z,0,0,1);
	
		openGL->DrawGLObject(object->glObject);
		glPopMatrix();
	}


	//glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST); 
	glDisable(GL_CULL_FACE); 
	glDisable(GL_BLEND);	

	//Restore matrix stack
	glMatrixMode(GL_PROJECTION);	
	glPopMatrix();

	SET_TIME(&end);
	LOG_TIME("AugmentedView Render",start,end);
}

void AugmentedView::Update(FrameItem * item)
{
	rotation = item->rotationMatrix;
	position = item->translationMatrix;
}


void AugmentedView::SetCameraPosition()
{	
	/*LOGD_Mat(LOGTAG_POSITION,"Camera position",position);
	LOGD_Mat(LOGTAG_POSITION,"Camera rotation",rotation);*/
	
	glMatrixMode(GL_PROJECTION);	
	glPushMatrix();

	OpenGLHelper::gluPerspective(40.0f,1.7f,20.0f, 600.0f);

	if (position->size().area() >= 3)
	{
		/*LOGD(LOGTAG_OPENGL,"Translating camera (%f,%f,%f)",
			-((float)position->at<double>(0,0)), 
			-((float)position->at<double>(0,1)),
			-((float)position->at<double>(2,0)));*/

		glTranslatef(((float)position->at<double>(0,0)), ((float)position->at<double>(0,1)),-((float)position->at<double>(0,2)));
	}


	/*glRotatef(rotation->at<float>(0,0),1,0,0);
	glRotatef(rotation->at<float>(0,1),0,1,0);
	glRotatef(rotation->at<float>(0,2),0,0,1);*/

}

