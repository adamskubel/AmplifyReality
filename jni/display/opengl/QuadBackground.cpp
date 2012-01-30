#include "display/opengl/QuadBackground.hpp"


ColorGLObject * testCube;

QuadBackground::QuadBackground(cv::Size2i size)
{
	//Create the texture object
	calculateTextureSize(size.width,size.height,textureWidth,textureHeight);

	LOGI(LOGTAG_OPENGL,"Creating texture, width=%d, height=%d", textureWidth, textureHeight);
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &(textureID));
	LOGD(LOGTAG_OPENGL,"Texture ID is %d", textureID);


	glBindTexture(GL_TEXTURE_2D, textureID);

	//Initialize texture with blank data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	texturedQuad = OpenGLHelper::CreateTexturedQuad(textureWidth,textureHeight,bgSize); 
	testCube = OpenGLHelper::CreateSolidColorCube(5,Colors::OliveDrab);
}

void QuadBackground::SetImage(cv::Mat * image) 
{			
	//Store the size of the image for rendering step
	imageWidth = image->cols;
	imageHeight = image->rows;
	//Store pointer to image data
	if (DRAW_BACKGROUND_BORDER)
		cv::rectangle(*image,Rect(0,0,imageWidth,imageHeight),cv::Scalar(0,255,0,255),3,CV_AA);
	imagePixels = image->ptr<uint32_t>(0);	
}


static void OpenGLSettings()
{	
	/*glDepthMask(true);
	glClearDepthf(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDisable(GL_CULL_FACE); 

	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/

	glDisable(GL_DEPTH_TEST); 
	glDisable(GL_BLEND);
}

static void ResetGLSettings()
{
	glDisable(GL_DEPTH_TEST); 
	glDisable(GL_BLEND);	

}

void QuadBackground::Render(OpenGL * openGL)
{
	struct timespec start,end;
	SET_TIME(&start);
	OpenGLSettings();
	
	//Draw object
	SetMatrices(openGL);

	//glEnable(GL_TEXTURE_2D);	
	glActiveTexture(GL_TEXTURE0);	
	glBindTexture(GL_TEXTURE_2D, textureID);
	glUniform1i(openGL->renderData.textureLocation,0);
	
	
	//Debugging - Draw a solid color to texture
	if (ENABLE_TEXTURE_COLOR)
	{
		u_int32_t * pxData = new u_int32_t[textureWidth*textureHeight];
		for (int i=0;i<(textureWidth*textureHeight);i++)
		{
			pxData[i] = 123912048;
		}
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, pxData);
		delete[] pxData;
	}
	
	//Update the texture 
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,imageWidth,imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, imagePixels);
		
	openGL->DrawGLObject(testCube);
	openGL->DrawGLObject(texturedQuad);
	
	ResetGLSettings();
	SET_TIME(&end);
	LOG_TIME("QuadBG Render", start, end);	
}


void QuadBackground::calculateTextureSize(int imageWidth, int imageHeight, int & _textureWidth, int & _textureHeight)
{
	if (USE_POWER2_TEXTURES)
	{
		int heightLog = ceilf(logbf((float)imageHeight))+1;
		int widthLog = ceilf(logbf((float)imageWidth))+1;

		_textureHeight = (int)pow(2,heightLog);
		_textureWidth = (int)pow(2,widthLog);
	}
	else
	{
		_textureWidth = imageWidth;
		_textureHeight = imageHeight;
	}
}

static void LogMat_Tmp(float * matArray)
{
	for (int i=0;i<16;i++)
	{
		LOGD(LOGTAG_OPENGL,"Mat[%d]=%f",i,matArray[i]);
	}
}


void QuadBackground::SetMatrices(OpenGL * openGL)
{	
	LOGV(LOGTAG_OPENGL,"Prepare BG, width=%d, height=%d",imageWidth,imageHeight);

	OpenGLRenderData renderData = openGL->renderData;
	
	float aspectRatio = ((float)openGL->screenWidth)/openGL->screenHeight;

	GLfloat orthoHeight;
	GLfloat orthoWidth;
	
	if (aspectRatio > 1)
	{
		orthoWidth = aspectRatio * bgSize;
		orthoHeight = bgSize;
	}
	else
	{
		orthoWidth = bgSize;
		orthoHeight = ((float)bgSize) / aspectRatio;
	}


	//Define projection matrix
	Mat camera = Mat::eye(4,4,CV_32F);
	//LOGV(LOGTAG_OPENGL,"Creating ortho: width=%f,height=%f",orthoWidth,orthoHeight);
	OpenGLHelper::createOrtho(camera,0.0f,orthoWidth,0.0f,orthoHeight,-10.0f,10.0f);

	Mat camT = Mat(camera.t());
	glUniformMatrix4fv(renderData.projectionMatrixLocation,1,GL_FALSE,camT.ptr<float>(0));

	
	LOGV(LOGTAG_OPENGL,"ImageHeight=%d,TextureHeight=%d,QuadHeight=%f",imageHeight,textureHeight,texturedQuad->height);	
	float imageHeightOnTexture = ((float)imageHeight/(float) textureHeight)* texturedQuad->height;
	float yScale = -(float)orthoHeight/(float)imageHeightOnTexture;	
	float yTranslation = (imageHeightOnTexture-texturedQuad->height);
	LOGV(LOGTAG_OPENGL,"Scaling background: scale=%f, Ytranslation=%f",yScale,yTranslation);	
	Mat modelMatrix = Mat::eye(4,4,CV_32F);

	OpenGLHelper::scale(modelMatrix,Point3f(-yScale,-yScale,1));		
	OpenGLHelper::translate(modelMatrix,Point3f(0,yTranslation,0));

	Mat mt = Mat(modelMatrix.t());
	glUniformMatrix4fv(renderData.modelMatrixLocation,1,GL_FALSE,mt.ptr<float>(0));
}

QuadBackground::~QuadBackground()
{	
	glDeleteTextures(1, & textureID);
	delete texturedQuad;
	LOGD(LOGTAG_OPENGL,"QuadBackground Deleted Successfully");
}