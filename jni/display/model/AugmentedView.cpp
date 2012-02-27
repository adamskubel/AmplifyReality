#include "display/model/AugmentedView.hpp"

#define LOGTAG_AUGMENTEDVIEW "AugmentedView"

AugmentedView::AugmentedView(cv::Mat _cameraMatrix)
{
	cameraMatrix = new Mat();
	_cameraMatrix.copyTo(*cameraMatrix);
	rotation = new Mat();
	position = new Mat();
	objectVector = std::vector<ARObject*>();
	LOGI(LOGTAG_POSITION, "Created AugmentedView");
	canDraw = false;

	testObject = new ARObject(OpenGLHelper::CreateSolidColorCube(40,Colors::OrangeRed));
	AddObject(testObject);
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
}

static void ResetGLSettings()
{
	glDisable(GL_DEPTH_TEST); 
	glDisable(GL_BLEND);	

}

void AugmentedView::HandleTouchInput(void * sender, TouchEventArgs args)
{
	inputPoints.push_back(args.TouchLocations);
	LOGV(LOGTAG_ARINPUT,"Added touch point(%d,%d)",args.TouchLocations.x,args.TouchLocations.y);
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

	Mat invProjection = projection.inv();

	while (!inputPoints.empty())
	{
		
		LOG_Mat(ANDROID_LOG_DEBUG,LOGTAG_ARINPUT,"Projection",&projection);
		LOG_Mat(ANDROID_LOG_DEBUG,LOGTAG_ARINPUT,"Inv. Projection",&invProjection);
		
		float data[] = {inputPoints.back().x,inputPoints.back().y,1.0f,1.0f};
		data[0] = (((data[0])/(openGL->screenWidth/2.0f)) - 1)/projection.at<float>(0,0);
		data[1]= -(((data[1])/(openGL->screenHeight/2.0f)) - 1)/projection.at<float>(0,0);
				
		Mat screenPoint = Mat(4,1,CV_32F,data);
		LOG_Mat(ANDROID_LOG_DEBUG,LOGTAG_ARINPUT,"ScaledScreenPoint",&screenPoint);

		Mat diff = invProjection * screenPoint;
		LOG_Mat(ANDROID_LOG_DEBUG,LOGTAG_ARINPUT,"Diff",&diff);
				
		Mat cameraPos = invProjection.col(3);		
		cameraPos *= 1.0f/ cameraPos.at<float>(3,0);
		LOG_Mat(ANDROID_LOG_DEBUG,LOGTAG_ARINPUT,"CameraPosition",&cameraPos);


		diff *= 1.0f/(diff.at<float>(3,0));
		LOG_Mat(ANDROID_LOG_DEBUG,LOGTAG_ARINPUT,"DiffNorm",&diff);
		
		float scaleZ = cameraPos.at<float>(2,0)/diff.at<float>(2,0);
		
		Mat farPoint = cameraPos - (diff * scaleZ);
		LOG_Mat(ANDROID_LOG_DEBUG,LOGTAG_ARINPUT,"FarPointOrigin",&farPoint);



		if (testObject != NULL)
			testObject->position = Point3f(farPoint.at<float>(0,0),farPoint.at<float>(1,0),farPoint.at<float>(2,0));
		else
			LOGW(LOGTAG_AUGMENTEDVIEW,"Test object is null");


		inputPoints.pop_back();
	}

	LOGV(LOGTAG_OPENGL,"Drawing %d ARObjects",objectVector.size());
	for (int i=0;i<objectVector.size();i++)
	{		
		ARObject * object = objectVector.at(i);

		LOGV(LOGTAG_OPENGL,"Drawing ARObject at (%f,%f,%f)",object->position.x,object->position.y,object->position.z);

		 
		Mat modelMatrix = Mat::eye(4,4,CV_32F);
		
		OpenGLHelper::translate(modelMatrix,Point3f(object->position.x,object->position.y,object->position.z));
		
		LOGV(LOGTAG_OPENGL,"With rotation (%f,%f,%f)",object->rotation.x,object->rotation.y,object->rotation.z);

		OpenGLHelper::rotate(modelMatrix,object->rotation.x, Point3f(1.0f, 0.0f, 0.0f));
		OpenGLHelper::rotate(modelMatrix,object->rotation.y, Point3f(0.0f, 1.0f, 0.0f));
		OpenGLHelper::rotate(modelMatrix,object->rotation.z, Point3f(0.0f, 0.0f, 1.0f));


		OpenGLHelper::scale(modelMatrix,object->scale);

		Mat mt = Mat(modelMatrix.t());
		glUniformMatrix4fv(renderData.modelMatrixLocation, 1, GL_FALSE, mt.ptr<float>(0));

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

//Field of view in degrees
void AugmentedView::SetFOV(float fov)
{
	fieldOfView = fov;
}


void AugmentedView::SetCameraPosition(OpenGLRenderData & renderData)
{	
	LOGD_Mat(LOGTAG_POSITION,"Camera position",position);
	LOGD_Mat(LOGTAG_POSITION,"Camera rotation",rotation);
	
	projection = Mat::eye(4,4,CV_32F);
	OpenGLHelper::gluPerspective(projection,fieldOfView,1.7f,20.0f, 600.0f);
	
	if (position->size().area() >= 3 && rotation->size().area() >= 3)
	{				
		OpenGLHelper::translate(projection,Point3f(((float)position->at<double>(0,0)), ((float)position->at<double>(0,1)),-((float)position->at<double>(0,2))));
		
		//LOGV(LOGTAG_OPENGL,"With rotation (%f,%f,%f)",object->rotation.x,object->rotation.y,object->rotation.z);

		//OpenGLHelper::rotate(projection,(float)rotation->at<double>(0,0), Point3f(1.0f, 0.0f, 0.0f));
		//OpenGLHelper::rotate(projection,(float)rotation->at<double>(0,1), Point3f(0.0f, 1.0f, 0.0f));
		//OpenGLHelper::rotate(projection,(float)rotation->at<double>(0,2), Point3f(0.0f, 0.0f, 1.0f));
	}
	
	Mat pt = Mat(projection.t());
	glUniformMatrix4fv(renderData.projectionMatrixLocation, 1, GL_FALSE, pt.ptr<float>(0));	

}

