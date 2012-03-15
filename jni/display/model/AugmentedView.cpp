#include "display/model/AugmentedView.hpp"

#define LOGTAG_AUGMENTEDVIEW "AugmentedView"

AugmentedView::AugmentedView(UIElementCollection * window, Engine * engine, cv::Mat _cameraMatrix)
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
	window->AddChild(tabs);

	createNext =false;

	GridLayout * myGrid = new GridLayout(cv::Size2i(5,4));

	cancelSelection = new Button("Cancel");
	cancelSelection->AddClickDelegate(ClickEventDelegate::from_method<AugmentedView,&AugmentedView::ButtonPressed>(this));
	myGrid->AddChild(cancelSelection,Point2i(4,3));		
	cancelSelection->SetVisible(false);
	cancelSelection->Name = "Cancel";
	
	releaseSelection = new Button("Release");
	releaseSelection->AddClickDelegate(ClickEventDelegate::from_method<AugmentedView,&AugmentedView::ButtonPressed>(this));
	myGrid->AddChild(releaseSelection,Point2i(4,2));	
	releaseSelection->SetVisible(false);
	releaseSelection->Name = "Release";

	Button * createCube = new Button("Create");
	createCube->AddClickDelegate(ClickEventDelegate::from_method<AugmentedView,&AugmentedView::ButtonPressed>(this));
	myGrid->AddChild(createCube,Point2i(4,1));	
	createCube->Name = "Create";
	createCube->FillColor = Colors::LightGreen;

	Button * deleteObject = new Button("Delete");
	deleteObject->AddClickDelegate(ClickEventDelegate::from_method<AugmentedView,&AugmentedView::ButtonPressed>(this));
	myGrid->AddChild(deleteObject,Point2i(4,0));	
	deleteObject->Name = "Delete";
	deleteObject->FillColor = Colors::Orange;

	tabs->AddTab("AR",myGrid);
	LOGD(LOGTAG_ARINPUT,"Laying out tabs %d,%d",engine->imageWidth,engine->imageHeight);
	tabs->DoLayout(Rect(0,0,engine->imageWidth,engine->imageHeight));
	
	tabs->SetTab(0);
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

void AugmentedView::UnselectObject()
{
	if (selectedObject != NULL)
	{
		cancelSelection->SetVisible(false);
		releaseSelection->SetVisible(false);
		delete selectedObject;
		selectedObject = NULL;
	}
}

void AugmentedView::ButtonPressed(void * sender, EventArgs args)
{
	Button * button = (Button*)sender;
	if (button->Name.compare("Cancel") == 0)
	{
		UnselectObject();
	}
	else if (button->Name.compare("Create") == 0)
	{
		UnselectObject();
		createNext = true;
	}
	else if (button->Name.compare("Release") == 0)
	{
		unselectNext = true;
	}
	else if (button->Name.compare("Delete") == 0)
	{
		if (selectedObject != NULL)
		{
			selectedObject->arObject->BoundingSphereRadius = 0.1f;
			selectedObject->arObject->scale *= 0.001f;
		}
	}
}

void AugmentedView::HandleButtonPress(void * sender, PhysicalButtonEventArgs args)
{
	if (args.ButtonCode == AKEYCODE_VOLUME_DOWN)
	{
		closeNext = true;
	}
	else if (args.ButtonCode == AKEYCODE_VOLUME_UP)
	{
		farNext = true;
	}
	
	if (selectedObject != NULL)
	{
		if (farNext)
		{
			selectedObject->objectPositionDelta += Point3f(0,0,5);
			farNext = false;
		}
		else if (closeNext)
		{
			selectedObject->objectPositionDelta -= Point3f(0,0,5);
			closeNext = false;
		}
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


static Point3f getObject3DCoord(Point3f objectPosition, Mat projection)
{
	Mat invProjection = projection.inv();

	float data[] = {objectPosition.x,objectPosition.y,objectPosition.z,1.0f};
	Mat cameraPos = Mat(4,1,CV_32F,data);
	cameraPos = invProjection * cameraPos;
	cameraPos *= 1.0f/ cameraPos.at<float>(3,0);
	
	LOG_Mat(ANDROID_LOG_VERBOSE,LOGTAG_ARINPUT,"3D->Screen",&cameraPos);
	return Point3f(cameraPos.at<float>(0,0),cameraPos.at<float>(1,0),cameraPos.at<float>(2,0));

}

static Point3f getObjectScreenCoord(Point3f objectPosition, Mat projection)
{
	float data[] = {objectPosition.x,objectPosition.y,objectPosition.z,1.0f};
	Mat cameraPos = Mat(4,1,CV_32F,data);
	cameraPos = projection * cameraPos;
	cameraPos *= 1.0f/ cameraPos.at<float>(3,0);
	
	LOG_Mat(ANDROID_LOG_VERBOSE,LOGTAG_ARINPUT,"Screen->3D",&cameraPos);
	return Point3f(cameraPos.at<float>(0,0),cameraPos.at<float>(1,0),cameraPos.at<float>(2,0));
}

Point3f AugmentedView::getCameraRotation()
{
	/*Mat invTranslation = Mat::eye(4,4,CV_32F);
	OpenGLHelper::translate(invTranslation,Point3f(((float)position->at<double>(0,0)), ((float)position->at<double>(0,1)),-((float)position->at<double>(0,2))));
	*/
	
	Mat cameraProj = Mat::eye(4,4,CV_32F);
	OpenGLHelper::gluPerspective(cameraProj,fieldOfView,1.7f,20.0f, 600.0f);
	Mat invCamProj = cameraProj.inv();

	//Projection = camera * gyro * translation * rotation
	Mat rotationOnly = projection * invCamProj;
	Mat rotationVector = Mat(1,3,CV_32F);
	rotationOnly *= (1.0f / rotationOnly.at<float>(3,3));
	Mat smallRotation =  rotationOnly(Range(0,3),Range(0,3));
	try
	{
		LOGD_Mat(LOGTAG_POSITION,"smallRotation",&smallRotation);
		cv::Rodrigues(smallRotation,rotationVector);
		LOGD_Mat(LOGTAG_POSITION,"RotationVector",&rotationVector);
	}
	catch (exception & e)
	{
		LOGW(LOGTAG_POSITION,"Error rodriguing: %s",e.what());
		return Point3f(0,0,0);
	}
	return Point3f(-(rotationVector.at<float>(0,0)), -(rotationVector.at<float>(0,1)),-(rotationVector.at<float>(0,2)));
}

void AugmentedView::UpdateObjectPosition(Mat & _projection, SelectedObject * selectedObject)
{
	//Point3f cameraPosition = getCameraPosition(projection);
	//Point3f objPosition =  selectedObject->objectPositionDelta + cameraPosition;
	//selectedObject->arObject->position =  objPosition; //Offset by camera position
	selectedObject->arObject->position =  getObject3DCoord(selectedObject->objectPositionDelta,projection);
	LOGD(LOGTAG_ARINPUT,"Updated Object Position(%f,%f,%f)",selectedObject->arObject->position.x,selectedObject->arObject->position.y,selectedObject->arObject->position.z);

	//Send command to server notifying that object has new position
	updateObjectMap.insert(pair<string,ARObjectMessage*>(selectedObject->arObject->objectID,new ARObjectMessage(selectedObject->arObject)));

	UnselectObject();
}

SelectedObject * AugmentedView::SelectObjects(OpenGL * openGL)
{	
	vector<Point2i> currentPoints = inputPoints;
	inputPoints.clear();
	SelectedObject * found = NULL;

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
			cursorShowTime.tv_sec = 0; //hide cursor if selection successful
			found = new SelectedObject(hitObjects.front(),now);
			break;
		}
		else
		{
			currentPoints.pop_back();
		}


		if (found == NULL)
			SET_TIME(&cursorShowTime);
	}

	return found;
}
bool showCursor;
void AugmentedView::Render(OpenGL * openGL)
{
	if (!canDraw)
		return;

	OpenGLRenderData renderData = openGL->renderData;

	struct timespec start,end;
	SET_TIME(&start);

	SetCameraPosition(renderData);	
	OpenGLSettings();
	

	SelectedObject * newSelection = SelectObjects(openGL);
	

	struct timespec now;
	SET_TIME(&now);
	double timediff = calc_time_double(lastSelectionTime,now);	
	
	if (unselectNext)
	{
		if (selectedObject != NULL)
			UpdateObjectPosition(projection,selectedObject);
		unselectNext = false;

	}
	//Only allowed to change selections every 1.0 seconds
	else if (newSelection != NULL && timediff > 800000.0)
	{
		//Unselect object if selected twice
		if (selectedObject != NULL && selectedObject->arObject == newSelection->arObject)
		{		
			UpdateObjectPosition(projection,selectedObject);
		}
		else
		{
			delete selectedObject;
			selectedObject = newSelection;
			//			
			//Point3f cameraPosition = getCameraPosition(projection);			
			//Point3f cameraOffset = selectedObject->arObject->position - cameraPosition;
			////Initial offset between object and camera
			selectedObject->objectPositionDelta = getObjectScreenCoord(selectedObject->arObject->position,projection);//scameraOffset;
			projection.copyTo(selectedObject->originalProjectionMat);
			cancelSelection->SetVisible(true);
			releaseSelection->SetVisible(true);

			//LOGD(LOGTAG_ARINPUT,"CameraPositionOffset(%f,%f,%f)",cameraOffset.x,cameraOffset.y,cameraOffset.z);
		}
		lastSelectionTime = now;
	}else if (newSelection != NULL)
	{
		LOGD(LOGTAG_ARINPUT,"Time spacing too short for unselect. Diff=%lf",timediff);
	}
	


	SET_TIME(&now);
	bool popVector = false;
	timediff = calc_time_double(cursorShowTime,now);
	if (timediff < 5000000.0)
	{
		objectVector.push_back(testObject);	
		popVector = true;
	}
	LOGV(LOGTAG_OPENGL,"Drawing %d ARObjects",objectVector.size());
	for (int i=0;i<objectVector.size();i++)
	{		
		ARObject * object = objectVector.at(i);

		//LOGV(LOGTAG_OPENGL,"Drawing ARObject at (%f,%f,%f)",object->position.x,object->position.y,object->position.z);
		//LOGV(LOGTAG_OPENGL,"With rotation (%f,%f,%f)",object->rotation.x,object->rotation.y,object->rotation.z);
		 
		Mat modelMatrix = Mat::eye(4,4,CV_32F);

		if (selectedObject != NULL && selectedObject->arObject == object)
		{			
			//Point3f cameraPosition = getCameraPosition(projection);			
			selectedObject->arObject->position =  getObject3DCoord(selectedObject->objectPositionDelta,projection);//selectedObject->objectPositionDelta + cameraPosition; 
		/*	Point3f cameraRotation = getCameraRotation();			
			selectedObject->arObject->rotation = selectedObject->objectRotationDelta + cameraRotation; */
		}

		OpenGLHelper::translate(modelMatrix,Point3f(object->position.x,object->position.y,object->position.z));
		OpenGLHelper::rotate(modelMatrix,object->rotation.x, Point3f(1.0f, 0.0f, 0.0f));
		OpenGLHelper::rotate(modelMatrix,object->rotation.y, Point3f(0.0f, 1.0f, 0.0f));
		OpenGLHelper::rotate(modelMatrix,object->rotation.z, Point3f(0.0f, 0.0f, 1.0f));


		Mat tmpModelMatrix;
		//Use seperate scale for selection indicator
		if (selectedObject != NULL && selectedObject->arObject == object)
		{
			modelMatrix.copyTo(tmpModelMatrix);
		}

		

		OpenGLHelper::scale(modelMatrix,object->scale);
		Mat mt = Mat(modelMatrix.t());
		glUniformMatrix4fv(renderData.modelMatrixLocation, 1, GL_FALSE, mt.ptr<float>(0));
		
		openGL->DrawGLObject(object->glObject);

		if (selectedObject != NULL && selectedObject->arObject == object)
		{
			float selectorSize = object->BoundingSphereRadius*2.25f;
			OpenGLHelper::scale(tmpModelMatrix,Point3f(selectorSize,selectorSize,selectorSize));

			Mat mt = Mat(tmpModelMatrix.t());
			glUniformMatrix4fv(renderData.modelMatrixLocation, 1, GL_FALSE, mt.ptr<float>(0));
			openGL->DrawGLObject(selectionIndicator->glObject);
		}
	}	
	//Get rid of test object
	if (popVector)
		objectVector.pop_back();
	ResetGLSettings();		

	SET_TIME(&end);
	LOG_TIME("AugmentedView Render",start,end);
	canDraw = false;
}

void AugmentedView::SetTransformations(Mat * translationMatrix, Mat * rotationMatrix, Mat gyroData)
{
	gyroData.convertTo(gyroRotation,CV_32FC1);	
	rotation = rotationMatrix;
	position = translationMatrix;
}

void AugmentedView::Update(Mat * rgbaImage, Engine * engine)
{	
	tabs->Draw(rgbaImage);

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

	
	if (createNext)
	{
		Point3f cameraPosition = getCameraPosition(projection);
		char objectName[100];
		sprintf(objectName,"user_obj_%d",objectVector.size()+1);
		GLObject * glObject = OpenGLHelper::CreateMultiColorCube(35);
		Point3f newLocation = Point3f(0,0,0);
		if (testObject != NULL)
			newLocation = testObject->position;

		ARObject * newObject = new ARObject(glObject, newLocation);
		newObject->BoundingSphereRadius = 20;
		newObject->objectID = objectName;
		if (engine->communicator->IsConnected())
		{
			engine->communicator->SendMessage(new ARObjectMessage(newObject,true));
		}
		createNext = false;
		objectVector.push_back(newObject);
	}

	canDraw = true;
}

//Field of view in degrees
void AugmentedView::SetFOV(float fov)
{
	fieldOfView = fov;
}


void AugmentedView::SetCameraPosition(OpenGLRenderData & renderData)
{	
	/*LOGD_Mat(LOGTAG_POSITION,"Camera position",position);
	LOGD_Mat(LOGTAG_POSITION,"Camera rotation",rotation);*/
	
	projection = Mat::eye(4,4,CV_32F);
	OpenGLHelper::gluPerspective(projection,fieldOfView,1.7f,20.0f, 600.0f);
	
	if (position->size().area() >= 3 && rotation->size().area() >= 3)
	{			
		try
		{
			projection =  projection * gyroRotation;
			LOGD_Mat(LOGTAG_SENSOR,"GyroData",&gyroRotation);
			LOGD_Mat(LOGTAG_SENSOR,"Projection",&projection);
		}
		catch (exception & e )
		{
			LOGW(LOGTAG_SENSOR,"Error transforming matrix T1=%d,T2=%d: %s",projection.type(),gyroRotation.type(),e.what());
		}


		OpenGLHelper::translate(projection,Point3f(((float)position->at<double>(0,0)), ((float)position->at<double>(0,1)),-((float)position->at<double>(0,2))));
		
		
		//LOGV(LOGTAG_OPENGL,"With rotation (%f,%f,%f)",object->rotation.x,object->rotation.y,object->rotation.z);

		OpenGLHelper::rotate(projection,-(float)rotation->at<double>(0,0), Point3f(1.0f, 0.0f, 0.0f));
		OpenGLHelper::rotate(projection,-(float)rotation->at<double>(0,1), Point3f(0.0f, 1.0f, 0.0f));
		OpenGLHelper::rotate(projection,(float)rotation->at<double>(0,2), Point3f(0.0f, 0.0f, 1.0f));
		
		
	}
	
	Mat pt = Mat(projection.t());
	glUniformMatrix4fv(renderData.projectionMatrixLocation, 1, GL_FALSE, pt.ptr<float>(0));	

}

