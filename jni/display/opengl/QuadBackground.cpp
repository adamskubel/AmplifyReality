#include "display/opengl/QuadBackground.hpp"



QuadBackground::QuadBackground(int _imageWidth, int _imageHeight)
{
	//Create the texture object
	calculateTextureSize(_imageWidth,_imageHeight,&textureWidth,&textureHeight);

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
	struct timespec start,end;
	SET_TIME(&start);
		
	//Store the size of the image for rendering step
	imageWidth = image->cols;
	imageHeight = image->rows;
	//Store pointer to image data
	if (DRAW_BACKGROUND_BORDER)
		cv::rectangle(*image,Rect(0,0,imageWidth,imageHeight),cv::Scalar(0,255,0,255),3,CV_AA);
	imagePixels = image->ptr<uint32_t>(0);
	
	SET_TIME(&end);
	LOG_TIME("QuadBG Update", start, end);	
}

void QuadBackground::Render(OpenGL * openGL)
{
	struct timespec start,end;
	SET_TIME(&start);

	//Background Settings
	/*glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);*/
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	//glDisable(GL_LIGHTING);	
	glEnable(GL_TEXTURE_2D);	
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);	
	glDisableClientState(GL_COLOR_ARRAY);

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
		
	//Draw object
	openGL->DrawGLObject(texturedQuad);

	//Restore Matrix stacks
	glMatrixMode(GL_PROJECTION); glPopMatrix();
	glMatrixMode(GL_MODELVIEW);	glPopMatrix();

	//Restore settings
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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



void QuadBackground::SetMatrices(OpenGL * openGL)
{	
	LOGV(LOGTAG_OPENGL,"Prepare BG, width=%d, height=%d",imageWidth,imageHeight);

	glClearColorx(0, 0, 0, 0); 
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	

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
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glOrthof(0,orthoWidth,0,orthoHeight,-10,10);
	
	//Define model matrix
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	//Assume input image is mirrored on Y axis
	//Assume X-Y scale is the same - choose Y
	//Ymax in device coords is orthoHeight
	//Yimg in device coords is imageHeight/textureHeight * bgSize 
	//Scale= -Ymax/Yimg

	float yImg = ((float)imageHeight/(float) textureHeight)* texturedQuad->height;
	float yScale = -(float)orthoHeight/(float)yImg;
	
	float xImg = ((float)imageWidth/(float) textureWidth)* texturedQuad->width;
	float xScale = (float)orthoWidth/(float)xImg;

	LOGV(LOGTAG_OPENGL,"Scaling background: xScale=%f,yScale=%f",xScale,yScale);
	
	glScalef(-yScale,yScale,1);
	
	glTranslatef(0,-yImg,0);

}

QuadBackground::~QuadBackground()
{	
	glDeleteTextures(1, & textureID);
	delete texturedQuad;
	LOGD(LOGTAG_OPENGL,"QuadBackground Deleted Successfully");
}