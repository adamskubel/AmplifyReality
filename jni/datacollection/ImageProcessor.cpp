#include <datacollection\ImageProcessor.hpp>



void ImageProcessor::SimpleThreshold(FrameItem * item)
{
	LOGD(LOGTAG_IMAGEPROCESSING,"Performing Simple global threshold (Otsu)");

	struct timespec start,end;
	SET_TIME(&start);	

	if (item->binaryImage == NULL)
		item->binaryImage = new Mat(); 

	threshold(*(item->grayImage), *(item->binaryImage), 100, 255, THRESH_OTSU + THRESH_BINARY);	

	SET_TIME(&end);
	LOG_TIME("Simple Threshold", start, end);
}


void ImageProcessor::LocalizedThreshold(FrameItem * item)
{
	struct timespec start,end;
	SET_TIME(&start);

	int windowWidth = 0, windowHeight =0;
	calculateWindow(*item, &windowWidth,&windowHeight);
	localThresholding(*(item->grayImage),*(item->binaryImage),windowWidth,windowHeight);

	SET_TIME(&end);
	LOG_TIME("Local Threshold", start, end);
}

void ImageProcessor::calculateWindow(FrameItem item, int * windowWidth, int * windowHeight)
{
	*windowWidth = 100;
	*windowHeight = 100;
}

void ImageProcessor::localThresholding(Mat & inputImg, Mat & outputImg, int windowWidth, int windowHeight)
{	
	try
	{		
		int xCount = floor(inputImg.cols/windowWidth);
		int yCount = floor(inputImg.rows/windowHeight);

		LOGD("ImageCollector","xCount=%d,yCount=%d,rows=%d,cols=%d,step0=%d,step1=%d",xCount,yCount,inputImg.rows,inputImg.cols,inputImg.step[0],inputImg.step[1]);

		outputImg = Mat(inputImg.rows,inputImg.cols,inputImg.type());	

		for (int i=0;i<inputImg.rows;i++)
		{
			const unsigned char* Mi = inputImg.ptr<unsigned char>(i);			
			unsigned char * copyTo = outputImg.ptr<unsigned char>(i);
			memcpy(copyTo,Mi,inputImg.step[0]);
		}

		for (int yOffset=0; yOffset < yCount*windowHeight; yOffset += windowHeight)
		{
			for (int xOffset=0; xOffset < xCount*windowWidth; xOffset += windowWidth)
			{		
				Mat tMat = Mat(windowHeight,windowWidth,inputImg.type());
				Mat tMatThresholded = Mat(windowHeight,windowWidth,inputImg.type());
				for (int i=0; i < windowHeight;i++)
				{
					const unsigned char* Mi = inputImg.ptr<unsigned char>(i+yOffset);
					Mi = Mi + xOffset*inputImg.step[1];
					unsigned char * copyTo = tMat.ptr<unsigned char>(i);

					memcpy(tMat.ptr<unsigned char>(i),Mi,windowWidth);
				}
				LOGD("ImageCollector","Local Threshold: xOffset=%d, yOffset=%d",xOffset,yOffset);
				threshold(tMat, tMatThresholded, 100, 255, THRESH_OTSU + THRESH_BINARY);
				for (int i=0; i< windowHeight;i++)
				{
					const unsigned char* Mi = tMatThresholded.ptr<unsigned char>(i);
					unsigned char* copyTo = outputImg.ptr<unsigned char>(i+yOffset);

					copyTo = copyTo + xOffset*inputImg.step[1];

					memcpy(copyTo,Mi,windowWidth);
				}
			}
		}

	}
	catch (std::exception &e)
	{
		LOGE("ImageCollector","LocalThresholding Exception: %s",e.what());
	}
	LOGD("ImageCollector","Local thresholding complete");

}