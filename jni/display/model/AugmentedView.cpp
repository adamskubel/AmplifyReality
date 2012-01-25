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

static void setLights()
{
	 // Enable lighting
    glEnable(GL_LIGHTING);

    // Turn the first light on
    glEnable(GL_LIGHT0);
    
    // Define the ambient component of the first light
    const GLfloat light0Ambient[] = {0.1, 0.1, 0.1, 1.0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0Ambient);
    
    // Define the diffuse component of the first light
    const GLfloat light0Diffuse[] = {0.7, 0.7, 0.7, 1.0};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Diffuse);
    
    // Define the specular component and shininess of the first light
    const GLfloat light0Specular[] = {0.7, 0.7, 0.7, 1.0};
    const GLfloat light0Shininess = 0.4;
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0Specular);
    
    
    // Define the position of the first light
    const GLfloat light0Position[] = {0.0, 10.0, 10.0, 0.0}; 
    glLightfv(GL_LIGHT0, GL_POSITION, light0Position); 
    
    // Define a direction vector for the light, this one points right down the Z axis
    const GLfloat light0Direction[] = {0.0, 0.0, -1.0};
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light0Direction);

    // Define a cutoff angle. This defines a 90° field of vision, since the cutoff
    // is number of degrees to each side of an imaginary line drawn from the light's
    // position along the vector supplied in GL_SPOT_DIRECTION above
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 45.0);

		//Lighting
	
//	GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
//	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
//	GLfloat mat_shininess[] = { 50.0 };
//	glClearColor (0.0, 0.0, 0.0, 0.0);
//	
////	glEnable(GL_COLOR_MATERIAL);
//	glShadeModel (GL_SMOOTH);
//
//	
//	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_diffuse);
//	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
//	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
//	
//	
//	GLfloat white[] = {1,1,1,1};
//	GLfloat black[] = {0.0f,0.0f,0.0f,1.0f};
//
//	GLfloat light_position[] = { 0, 1, 0, 0.0 };
//	glLightfv(GL_LIGHT0, GL_POSITION, light_position);	
//	glLightfv(GL_LIGHT0, GL_AMBIENT, black );
//	glLightfv(GL_LIGHT0, GL_DIFFUSE, white );
//
//	//GLfloat light_position2[] = { 1, 0, 1, 0.0};
//	//glLightfv(GL_LIGHT1, GL_POSITION, light_position2);
//	//glLightfv(GL_LIGHT1, GL_AMBIENT, black);
//	//glLightfv(GL_LIGHT1, GL_DIFFUSE, white );
//
//	glEnable(GL_LIGHTING);
//	glEnable(GL_LIGHT0);
//	//glEnable(GL_LIGHT1);
	//End lighting
}

static void OpenGLSettings()
{
	glEnableClientState(GL_COLOR_ARRAY);	

	glDisable(GL_LIGHTING);
	//setLights();
	
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
	glDisable(GL_LIGHTING);	
	glDisable(GL_DEPTH_TEST); 
	glDisable(GL_BLEND);	

}

void AugmentedView::Render(OpenGL * openGL)
{
	if (!canDraw)
		return;


	struct timespec start,end;
	SET_TIME(&start);

	SetCameraPosition();
		
	glMatrixMode(GL_MODELVIEW);
	
	OpenGLSettings();

	LOGV(LOGTAG_OPENGL,"Drawing %d ARObjects",objectVector.size());
	for (int i=0;i<objectVector.size();i++)
	{		
		glPushMatrix();
		ARObject * object = objectVector.at(i);

		LOGV(LOGTAG_OPENGL,"Drawing ARObject at (%f,%f,%f)",object->position.x,object->position.y,object->position.z);
		glTranslatef(object->position.x,object->position.y,object->position.z);
		
		LOGV(LOGTAG_OPENGL,"With rotation (%f,%f,%f)",object->rotation.x,object->rotation.y,object->rotation.z);
		glRotatef(object->rotation.x,1,0,0);
		glRotatef(object->rotation.y,0,1,0);
		glRotatef(object->rotation.z,0,0,1);
	
		openGL->DrawGLObject(object->glObject);
		glPopMatrix();
	}

	ResetGLSettings();
		
	//Restore matrix stack
	glMatrixMode(GL_PROJECTION);	
	glPopMatrix();

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


void AugmentedView::SetCameraPosition()
{	
	/*LOGD_Mat(LOGTAG_POSITION,"Camera position",position);
	LOGD_Mat(LOGTAG_POSITION,"Camera rotation",rotation);*/
	
	glMatrixMode(GL_PROJECTION);	
	glPushMatrix();

	OpenGLHelper::gluPerspective(40.0f,1.7f,20.0f, 600.0f);
	

	if (position->size().area() >= 3 && rotation->size().area() >= 3)
	{		
		glTranslatef(((float)position->at<double>(0,0)), ((float)position->at<double>(0,1)),-((float)position->at<double>(0,2)));
		
		glRotatef((180.0/PI)*rotation->at<double>(0,0),1,0,0);
		glRotatef((180.0/PI)*rotation->at<double>(0,1),0,1,0);
		glRotatef((180.0/PI)*rotation->at<double>(0,2),0,0,1);
	}

	

}