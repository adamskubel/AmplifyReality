#include "display/model/AugmentedView.hpp"

#define LOGTAG_AUGMENTEDVIEW "AugmentedView"

AugmentedView::AugmentedView(UIElementCollection * window, cv::Mat _cameraMatrix)
{
	cameraMatrix = new Mat();
	_cameraMatrix.copyTo(*cameraMatrix);
	rotation = new Mat();
	position = new Mat();
	objectVector = std::vector<ARObject*>();
	LOGI(LOGTAG_POSITION, "Created AugmentedView");
	canDraw = false;

	Scalar selectColor = Colors::DodgerBlue;
	selectColor[3] = 80;
	selectionIndicator = new ARObject(OpenGLHelper::CreateSolidColorCube(1,selectColor));
	selectedObject = NULL;
	SET_TIME(&lastSelectionTime);
	testObject = new ARObject(OpenGLHelper::CreateSolidColorCube(10,Colors::OrangeRed));

	tabs = new TabDisplay(true);
	tabs->DoLayout(Rect(0,0,_cameraMatrix.cols,_cameraMatrix.rows));
	window->AddChild(tabs);

	GridLayout * myGrid = new GridLayout(cv::Size2i(5,4));
	tabs->AddTab("AR",myGrid);

	Button * cancelSelection = new Button("Cancel");
	cancelSelection->AddClickDelegate(ClickEventDelegate::from_method<AugmentedView,&AugmentedView::ButtonPressed>(this));
	myGrid->AddChild(cancelSelection,Point2i(4,3));		

}

//Delete the camera matrix and the vector of AR objects
AugmentedView::~AugmentedView()
{
	delete tabs;
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
	if (args.InputType == ARInput::Press)
	{
		inputPoints.push_back(args.TouchLocations);
		LOGV(LOGTAG_ARINPUT,"Added touch point(%d,%d)",args.TouchLocations.x,args.TouchLocations.y);
	}
}

void AugmentedView::ButtonPressed(void * sender, EventArgs args)
{
	if (selectedObject != NULL)
	{
		delete selectedObject;
		selectedObject = NULL;
	}
}


static void getPickingRay(Point2i point, OpenGL * openGL, Mat projection, Point3f & near, Point3f & far, ARObject * testObject, float fieldOfView)
{

	Mat invProjection = projection.inv();

	LOG_Mat(ANDROID_LOG_VERBOSE,LOGTAG_ARINPUT,"Projection",&projection);
	//LOG_Mat(ANDROID_LOG_VERBOSE,LOGTAG_ARINPUT,"Inv. Projection",&invProjection);
	
	Mat tmpProjection = Mat::eye(4,4,CV_32F);
	OpenGLHelper::gluPerspective(tmpProjection,fieldOfView,1.7f,20.0f, 600.0f);
	

	float data[] = {point.x,point.y,1.0f,1.0f};
	data[0] = (((data[0])/(openGL->screenWidth/2.0f)) - 1)/tmpProjection.at<float>(0,0);
	data[1]= -(((data[1])/(openGL->screenHeight/2.0f)) - 1)/tmpProjection.at<float>(0,0);

	Mat screenPoint = Mat(4,1,CV_32F,data);
	Mat cameraPos; screenPoint.copyTo(cameraPos);
	//LOG_Mat(ANDROID_LOG_VERBOSE,LOGTAG_ARINPUT,"ScaledScreenPoint",&screenPoint);

	Mat diff = invProjection * screenPoint;
	//LOG_Mat(ANDROID_LOG_VERBOSE,LOGTAG_ARINPUT,"Diff",&diff);

	//Mat cameraPos = invProjection.col(3);		
	//cameraPos *= 1.0f/ cameraPos.at<float>(3,0);
	//LOG_Mat(ANDROID_LOG_VERBOSE,LOGTAG_ARINPUT,"CameraPosition",&cameraPos);\
	
	cameraPos.at<float>(2,0) = 0.0f;
	cameraPos = invProjection * cameraPos;
	cameraPos *= 1.0f/ cameraPos.at<float>(3,0);
	//LOG_Mat(ANDROID_LOG_VERBOSE,LOGTAG_ARINPUT,"CameraPosition",&cameraPos);


	diff *= 1.0f/(diff.at<float>(3,0));
	//LOG_Mat(ANDROID_LOG_VERBOSE,LOGTAG_ARINPUT,"DiffNorm",&diff);

	float scaleZ = 600.0f/diff.at<float>(2,0);	
	Mat farMatrix = cameraPos - (diff * scaleZ);	
	//LOG_Mat(ANDROID_LOG_VERBOSE,LOGTAG_ARINPUT,"FarPoint",&farMatrix);

	near = Point3f(cameraPos.at<float>(0,0),cameraPos.at<float>(1,0),cameraPos.at<float>(2,0));
	far = Point3f(farMatrix.at<float>(0,0),farMatrix.at<float>(1,0),farMatrix.at<float>(2,0));

	if (testObject != NULL)
	{
		Mat originIntersection =cameraPos - (cameraPos.at<float>(2.0)/diff.at<float>(2,0))*diff;
		testObject->position = Point3f(originIntersection.at<float>(0,0),originIntersection.at<float>(1,0),originIntersection.at<float>(2,0));
		//LOG_Mat(ANDROID_LOG_VERBOSE,LOGTAG_ARINPUT,"FarPointOrigin",&originIntersection);
	}

}

static Point3f getCameraPosition(Mat projection)
{
	Mat invProjection = projection.inv();

	float data[] = {0,0,0,1.0f};
	Mat cameraPos = Mat(4,1,CV_32F,data);
	cameraPos = invProjection * cameraPos;
	cameraPos *= 1.0f/ cameraPos.at<float>(3,0);
	
	return Point3f(cameraPos.at<float>(0,0),cameraPos.at<float>(1,0),cameraPos.at<float>(2,0));
	//LOG_Mat(ANDROID_LOG_VERBOSE,LOGTAG_ARINPUT,"CameraPosition",&cameraPos);

}


void AugmentedView::Render(OpenGL * openGL)
{
	if (!canDraw)
		return;

	OpenGLRenderData renderData = openGL->renderData;

	struct timespec start,end;
	SET_TIME(&start);

	SetCameraPosition(renderData);
	
	vector<Point2i> currentPoints = inputPoints;
	inputPoints.clear();
	
	OpenGLSettings();
	
	SelectedObject * newSelection = NULL;
	while (!currentPoints.empty())
	{
		Point3f p1,p2;

		getPickingRay(currentPoints.back(),openGL,projection,p1,p2,testObject,fieldOfView);

		vector<ARObject*> hitObjects;		
		Point3f p12 = p2-p1;
		//LOGD(LOGTAG_ARINPUT,"P12=(%f,%f,%f)",p12.x,p12.y,p12.z);

		float p12Mag = pow(p12.x,2) + pow(p12.y,2) + pow(p12.z,2);
		for (int i=0;i<objectVector.size();i++)
		{
			Point3f p0 = objectVector[i]->position;
			//Mat position = Mat(1,3,CV_32F);

			Point3f cross = p12.cross((p1-p0));
			float distance = pow(cross.x,2) + pow(cross.y,2) + pow(cross.z,2);
			distance /= p12Mag;
		
			distance = sqrt(distance);

			if (distance < objectVector[i]->BoundingSphereRadius)
			{
				hitObjects.push_back(objectVector[i]);
			}
			else
			{
				LOGV(LOGTAG_ARINPUT,"Did not collide with object %s at position %f,%f,%f with sphere of radius %f (distance=%f)",
					objectVector[i]->objectID.c_str(),p0.x,p0.y,p0.z,objectVector[i]->BoundingSphereRadius,distance);
			}
		}

		if (hitObjects.size() > 0)
		{
			if (hitObjects.size() > 1)
			{
				std::sort(hitObjects.begin(),hitObjects.end(),ARObjectDistanceSort_Ascending(p1));
			}
			struct timespec now;
			SET_TIME(&now);
			newSelection = new SelectedObject(hitObjects.front(),now);
			currentPoints.clear();
		}
		else
		{
			currentPoints.pop_back();
		}
	}

	struct timespec now;
	SET_TIME(&now);
	double timediff = calc_time_double(lastSelectionTime,now);

	//Only allowed to change selections every 1.0 seconds
	if (newSelection != NULL && timediff > 800000.0)
	{
		//Unselect object if selected twice
		if (selectedObject != NULL && selectedObject->arObject == newSelection->arObject)
		{		
			Point3f cameraPosition = getCameraPosition(projection);
			Point3f objPosition =  selectedObject->objectPositionDelta + cameraPosition;
			selectedObject->arObject->position = objPosition; //Offset by camera position
			LOGD(LOGTAG_ARINPUT,"Updated Object Position(%f,%f,%f)",selectedObject->arObject->position.x,selectedObject->arObject->position.y,selectedObject->arObject->position.z);

			//Send command to server notifying that object has new position
			updateObjectMap.insert(pair<string,ARObjectMessage*>(selectedObject->arObject->objectID,new ARObjectMessage(selectedObject->arObject)));

			delete selectedObject;
			selectedObject = NULL;
		}
		else
		{
			delete selectedObject;
			selectedObject = newSelection;
						
			Point3f cameraPosition = getCameraPosition(projection);			
			Point3f cameraOffset = selectedObject->arObject->position - cameraPosition;
			//Initial offset between object and camera
			selectedObject->objectPositionDelta = cameraOffset;

			LOGD(LOGTAG_ARINPUT,"CameraPositionOffset(%f,%f,%f)",cameraOffset.x,cameraOffset.y,cameraOffset.z);
		}
		lastSelectionTime = now;
	}else if (newSelection != NULL)
	{
		LOGD(LOGTAG_ARINPUT,"Time spacing too short for unselect. Diff=%lf",timediff);
	}

	objectVector.push_back(testObject);	

	LOGV(LOGTAG_OPENGL,"Drawing %d ARObjects",objectVector.size());
	for (int i=0;i<objectVector.size();i++)
	{		
		ARObject * object = objectVector.at(i);

		//LOGV(LOGTAG_OPENGL,"Drawing ARObject at (%f,%f,%f)",object->position.x,object->position.y,object->position.z);
		//LOGV(LOGTAG_OPENGL,"With rotation (%f,%f,%f)",object->rotation.x,object->rotation.y,object->rotation.z);
		 
		Mat modelMatrix = Mat::eye(4,4,CV_32F);
		
		if (selectedObject != NULL && selectedObject->arObject == object)
		{			
			Point3f cameraPosition = getCameraPosition(projection);			
			selectedObject->arObject->position = selectedObject->objectPositionDelta + cameraPosition; 
			//OpenGLHelper::translate(modelMatrix,objectPosition);
		}
		/*else
		{*/
			OpenGLHelper::translate(modelMatrix,Point3f(object->position.x,object->position.y,object->position.z));
			OpenGLHelper::rotate(modelMatrix,object->rotation.x, Point3f(1.0f, 0.0f, 0.0f));
			OpenGLHelper::rotate(modelMatrix,object->rotation.y, Point3f(0.0f, 1.0f, 0.0f));
			OpenGLHelper::rotate(modelMatrix,object->rotation.z, Point3f(0.0f, 0.0f, 1.0f));
		//}
				
		//Use seperate scale for selection indicator
		if (selectedObject != NULL && selectedObject->arObject == object)
		{
			Mat tmpModelMatrix; modelMatrix.copyTo(tmpModelMatrix);
			float selectorSize = object->BoundingSphereRadius*2.25f;
			OpenGLHelper::scale(tmpModelMatrix,Point3f(selectorSize,selectorSize,selectorSize));

			Mat mt = Mat(tmpModelMatrix.t());
			glUniformMatrix4fv(renderData.modelMatrixLocation, 1, GL_FALSE, mt.ptr<float>(0));
			openGL->DrawGLObject(selectionIndicator->glObject);
		}

		OpenGLHelper::scale(modelMatrix,object->scale);
		Mat mt = Mat(modelMatrix.t());
		glUniformMatrix4fv(renderData.modelMatrixLocation, 1, GL_FALSE, mt.ptr<float>(0));
		
		openGL->DrawGLObject(object->glObject);
	}	
	//Get rid of test object
	objectVector.pop_back();
	ResetGLSettings();		

	SET_TIME(&end);
	LOG_TIME("AugmentedView Render",start,end);
	canDraw = false;
}

void AugmentedView::SetTransformations(Mat * translationMatrix, Mat * rotationMatrix)
{
	
	rotation = rotationMatrix;
	position = translationMatrix;
}

void AugmentedView::Update(Mat * rgbaImage, Engine * engine)
{
	if (!updateObjectMap.empty())
	{
		if (engine->communicator != NULL && engine->communicator->IsConnected())
		{
			for (map<string,ARObjectMessage*>::iterator it = updateObjectMap.begin();it != updateObjectMap.end();it++)
			{
				LOGD(LOGTAG_ARINPUT,"Sending update for object %s",(*it).first.c_str());
				engine->communicator->SendMessage((*it).second);
			}			
		}
		updateObjectMap.clear();		
	}

	tabs->Draw(rgbaImage);

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

		OpenGLHelper::rotate(projection,(float)rotation->at<double>(0,0), Point3f(1.0f, 0.0f, 0.0f));
		OpenGLHelper::rotate(projection,(float)rotation->at<double>(0,1), Point3f(0.0f, 1.0f, 0.0f));
		OpenGLHelper::rotate(projection,(float)rotation->at<double>(0,2), Point3f(0.0f, 0.0f, 1.0f));
	}
	
	Mat pt = Mat(projection.t());
	glUniformMatrix4fv(renderData.projectionMatrixLocation, 1, GL_FALSE, pt.ptr<float>(0));	

}

