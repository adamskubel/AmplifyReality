#include "display/opengl/QuadBackground.hpp"



QuadBackground::QuadBackground(cv::Size2i size)
{
	//Create the texture object
	calculateTextureSize(size.width,size.height,&textureWidth,&textureHeight);

	LOGI(LOGTAG_OPENGL,"Creating texture, width=%d, height=%d", textureWidth, textureHeight);
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &(textureID));
	LOGD(LOGTAG_OPENGL,"Texture ID is %d", textureID);


	glBindTexture(GL_TEXTURE_2D, textureID);

	//Initialize texture with blank data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	texturedQuad = OpenGLHelper::CreateTexturedQuad(textureWidth,textureHeight,bgSize); 
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

void QuadBackground::Render(OpenGL * openGL)
{
	struct timespec start,end;
	SET_TIME(&start);

	glEnable(GL_TEXTURE_2D);	

	SetMatrices(openGL);
	
	glBindTexture(GL_TEXTURE_2D, textureID);
	
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
	glUniform1i(openGL->renderData.textureLocation,0);
	//Draw object
	openGL->DrawGLObject(texturedQuad);
	
	//Restore settings
	glDisable(GL_TEXTURE_2D);
	
	SET_TIME(&end);
	LOG_TIME("QuadBG Render", start, end);	
}


void QuadBackground::calculateTextureSize(int imageWidth, int imageHeight, int * textureWidth, int * textureHeight)
{
	if (USE_POWER2_TEXTURES)
	{
		int heightLog = ceilf(logbf((float)imageHeight))+1;
		int widthLog = ceilf(logbf((float)imageWidth))+1;

		*textureHeight = (int)pow(2,heightLog);
		*textureWidth = (int)pow(2,widthLog);
	}
	else
	{
		*textureWidth = imageWidth;
		*textureHeight = imageHeight;
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

	//glClearColor(0, 0.2f, 0, 1.0f); 
	//glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


	

	glViewport(0,0,openGL->screenWidth,openGL->screenHeight);

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
	//OpenGLHelper::createOrtho(camera,0.0f,orthoWidth,0.0f,orthoHeight,-10.0f,10.0f);

	//LogMat_Tmp(camera.ptr<float>(0));
	glUniformMatrix4fv(renderData.projectionMatrixLocation,1,GL_TRUE,camera.ptr<float>(0));

	
	//Assume input image is mirrored on Y axis
	//Assume X-Y scale is the same - choose Y
	//Ymax in device coords is orthoHeight
	//Yimg in device coords is imageHeight/textureHeight * bgSize 
	//Scale= -Ymax/Yimg


	float yImg = ((float)imageHeight/(float) textureHeight)* texturedQuad->height;
	float yScale = -(float)orthoHeight/(float)yImg;
	
	float xImg = ((float)imageWidth/(float) textureWidth)* texturedQuad->width;
	float xScale = (float)orthoWidth/(float)xImg;

	float scale = yScale;

	LOGV(LOGTAG_OPENGL,"Scaling background: scale=%f, Ytranslation=%f",scale,-yImg);
	
	Mat modelMatrix = Mat::eye(4,4,CV_32F);
	OpenGLHelper::scale(modelMatrix,Point3f(-scale,scale,1));
	
	//OpenGLHelper::translate(modelMatrix,Point3f(0,-yImg,0));
	//LogMat_Tmp(modelMatrix.ptr<float>(0));
	glUniformMatrix4fv(renderData.modelMatrixLocation,1,GL_TRUE,modelMatrix.ptr<float>(0));
}

QuadBackground::~QuadBackground()
{	
	glDeleteTextures(1, & textureID);
	delete texturedQuad;
	LOGD(LOGTAG_OPENGL,"QuadBackground Deleted Successfully");
}