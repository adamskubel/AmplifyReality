#include "display/model/AugmentedView.hpp"

AugmentedView::AugmentedView(cv::Mat _cameraMatrix)
{
	cameraMatrix = new Mat();
	_cameraMatrix.copyTo(*cameraMatrix);
	rotation = new Mat();
	position = new Mat();
	objectVector = std::vector<ARObject*>();
	LOGI(LOGTAG_POSITION, "Created AugmentedView");
	canDraw = false;


}

//Delete the camera matrix and the vector of AR objects
AugmentedView::~AugmentedView()
{
	delete cameraMatrix;
	delete rotation;
	delete position;
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
	LOGI(LOGTAG_POSITION, "Added new ARObject to AugmentedView");
}


static void OpenGLSettings()
{	
	glDepthMask(true);
	glClearDepthf(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDisable(GL_CULL_FACE); 

	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glShadeModel (GL_SMOOTH);	
}

static void ResetGLSettings()
{
	glDisable(GL_DEPTH_TEST); 
	glDisable(GL_BLEND);	

}

void AugmentedView::Render(OpenGL * openGL)
{
	if (!canDraw)
		return;

	OpenGLRenderData renderData = openGL->renderData;

	struct timespec start,end;
	SET_TIME(&start);

	SetCameraPosition(renderData);
		
	
	OpenGLSettings();

	LOGV(LOGTAG_OPENGL,"Drawing %d ARObjects",objectVector.size());
	for (int i=0;i<objectVector.size();i++)
	{		
		ARObject * object = objectVector.at(i);

		//LOGV(LOGTAG_OPENGL,"Drawing ARObject at (%f,%f,%f)",object->position.x,object->position.y,object->position.z);

		 
		Mat modelMatrix = Mat::eye(4,4,CV_32F);
		
		OpenGLHelper::translate(modelMatrix,Point3f(object->position.x,object->position.y,object->position.z));
		
		//LOGV(LOGTAG_OPENGL,"With rotation (%f,%f,%f)",object->rotation.x,object->rotation.y,object->rotation.z);

		OpenGLHelper::rotate(modelMatrix,object->rotation.x, Point3f(1.0f, 0.0f, 0.0f));
		OpenGLHelper::rotate(modelMatrix,object->rotation.y, Point3f(0.0f, 1.0f, 0.0f));
		OpenGLHelper::rotate(modelMatrix,object->rotation.z, Point3f(0.0f, 0.0f, 1.0f));

		glUniformMatrix4fv(renderData.modelMatrixLocation, 1, GL_FALSE, modelMatrix.ptr<float>());

		openGL->DrawGLObject(object->glObject);
	}
	

	ResetGLSettings();
		

	SET_TIME(&end);
	LOG_TIME("AugmentedView Render",start,end);
	canDraw = false;
}

void AugmentedView::Update(FrameItem * item)
{
	rotation = item->rotationMatrix;
	position = item->translationMatrix;
	canDraw = true;
}


void AugmentedView::SetCameraPosition(OpenGLRenderData & renderData)
{	
	/*LOGD_Mat(LOGTAG_POSITION,"Camera position",position);
	LOGD_Mat(LOGTAG_POSITION,"Camera rotation",rotation);*/
	
	Mat projection = Mat::ones(4,4,CV_32F);
	OpenGLHelper::gluPerspective(projection,40.0f,1.7f,20.0f, 600.0f);
	
	if (position->size().area() >= 3 && rotation->size().area() >= 3)
	{		
		/*glTranslatef(((float)position->at<double>(0,0)), ((float)position->at<double>(0,1)),-((float)position->at<double>(0,2)));
		
		glRotatef((180.0/PI)*rotation->at<double>(0,0),1,0,0);
		glRotatef((180.0/PI)*rotation->at<double>(0,1),0,1,0);
		glRotatef((180.0/PI)*rotation->at<double>(0,2),0,0,1);*/
/*
		glm::mat4 translateMatrix = glm::translate(projectionMatrix,
			glm::vec3(((float)position->at<double>(0,0)), ((float)position->at<double>(0,1)),-((float)position->at<double>(0,2))));*/

		Mat modelMatrix = Mat::eye(4,4,CV_32F);
		
		OpenGLHelper::translate(modelMatrix,Point3f(((float)position->at<double>(0,0)), ((float)position->at<double>(0,1)),-((float)position->at<double>(0,2))));
		
		//LOGV(LOGTAG_OPENGL,"With rotation (%f,%f,%f)",object->rotation.x,object->rotation.y,object->rotation.z);

		OpenGLHelper::rotate(modelMatrix,rotation->at<double>(0,0), Point3f(1.0f, 0.0f, 0.0f));
		OpenGLHelper::rotate(modelMatrix,rotation->at<double>(0,1), Point3f(0.0f, 1.0f, 0.0f));
		OpenGLHelper::rotate(modelMatrix,rotation->at<double>(0,2), Point3f(0.0f, 0.0f, 1.0f));
/*
		glm::mat4 RotateX = glm::rotate(translateMatrix,(float)((180.0/PI)*rotation->at<double>(0,0)), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 RotateY = glm::rotate(RotateX,(float)((180.0/PI)*rotation->at<double>(0,1)), glm::vec3(0.0f, 1.0f, 0.0f));
		projectionMatrix = glm::rotate(RotateY,(float)((180.0/PI)*rotation->at<double>(0,2)), glm::vec3(0.0f, 0.0f, 1.0f));*/
	}

	glUniformMatrix4fv(renderData.modelMatrixLocation, 1, GL_FALSE, projection.ptr<float>());	

}