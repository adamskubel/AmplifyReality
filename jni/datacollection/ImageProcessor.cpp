#include <datacollection\ImageProcessor.hpp>

#define CODE_SEARCH_RATIO 0.20f
#define SINGLEPATTERN_SEARCH_RATIO 3.0f



void ImageProcessor::SimpleThreshold(Mat * grayImage, Mat * binaryImage)
{
	LOGV(LOGTAG_IMAGEPROCESSING,"Performing Simple global threshold (Otsu)");

	struct timespec start,end;
	SET_TIME(&start);	

	if (binaryImage == NULL)
		binaryImage = new Mat(); 

	threshold(*grayImage, *binaryImage, 100, 255, THRESH_OTSU + THRESH_BINARY);	

	SET_TIME(&end);
	LOG_TIME("Simple Threshold", start, end);
}

/*
This method uses data from previous frames to determine how to binarize the  
camera image from the current frame.
*/
void ImageProcessor::FeedbackBinarization(FrameItem * item)
{ /*
	LOGV(LOGTAG_IMAGEPROCESSING,"Performing feedback-based threshold");
	//If the binary image is NULL, initalize it
	if (item->binaryImage == NULL)
		item->binaryImage = new Mat();
	
	vector<FrameItem *> previousItems = item->getLastFrames();
	if (previousItems.size() > 0)
	{
		//Just consider most recent frame for now
		FrameItem * lastFrame = previousItems.at(0);
					
		if (lastFrame->qrCode != NULL && lastFrame->qrCode->finderPatterns != NULL && (lastFrame->qrCode->validCodeFound ||
			(lastFrame->qrCode->finderPatterns->size() > 0 && lastFrame->qrCode->finderPatterns->size() <= 3)))
		{
			LOGV(LOGTAG_IMAGEPROCESSING,"Appropriate codes found for windowed threshold");
			Point_<int> centroid;
			int distance = 0;
			
			vector<Point2i> pointVector = lastFrame->qrCode->getPatternsAsPoints();
			LOG_Vector(ANDROID_LOG_DEBUG,std::string(LOGTAG_IMAGEPROCESSING),"QR Points",&pointVector);
			GetPointAttributes(pointVector,centroid,distance);	
			//If a complete code was found, then the next code should be nearby
			if (lastFrame->qrCode->validCodeFound)
			{
				distance = distance * 1.0f;			
			}
			//If not, then expand the search based on finder pattern size
			else if (lastFrame->qrCode->finderPatterns->size() == 1)
			{
				distance = lastFrame->qrCode->finderPatterns->at(0)->size;
				distance *= 7.0f;
			}
			//If not, then expand the search based on finder pattern size
			else if (lastFrame->qrCode->finderPatterns->size() == 2)
			{
				distance = distance * 1.5f;			
			}

			Point2i range = Point2i(distance,distance);
			WindowedThreshold(*(item->grayImage),*(item->binaryImage),Rect(centroid-range, centroid+range));
		}		
		//If there are 0 or greater than 4 patterns found, use global
		else
		{
			LOGV(LOGTAG_IMAGEPROCESSING,"Insufficient data found for windowed threshold");
			SimpleThreshold(item);
		}			
	}
	else
	{
		//If no previous frames, use global threshold
		SimpleThreshold(item);
	}		*/
}

void ImageProcessor::WindowedThreshold(Mat & inputImg, Mat & outputImg, Rect window)
{
	struct timespec start,end;
	SET_TIME(&start);	
	
	try
	{	
		int xOffset = window.x, yOffset = window.y;
		int windowWidth = window.width, windowHeight = window.height;
		
		LOGV(LOGTAG_IMAGEPROCESSING,"Performing windowed threshold (input values): xOffset=%d, yOffset=%d, windowWidth=%d, windowHeight=%d",
			xOffset,yOffset,windowWidth,windowHeight);

		//Check that window is within boundaries
		if (xOffset < 0 || xOffset >= inputImg.cols)
		{	xOffset = 0;}
		if (yOffset < 0 || yOffset >= inputImg.rows)
		{	yOffset = 0; }
		if (xOffset + windowWidth < 0 || xOffset + windowWidth > inputImg.cols)
		{	windowWidth = inputImg.cols-xOffset;}
		if (yOffset +  windowHeight < 0 || yOffset + windowHeight > inputImg.rows)
		{	windowHeight = inputImg.rows-yOffset;}
		
		LOGV(LOGTAG_IMAGEPROCESSING,"Performing windowed threshold (bounded values): xOffset=%d, yOffset=%d, windowWidth=%d, windowHeight=%d",
			xOffset,yOffset,windowWidth,windowHeight);

		outputImg = Mat::zeros(inputImg.rows,inputImg.cols,inputImg.type());
			
		Mat tMat = Mat(windowHeight,windowWidth,inputImg.type());
		Mat tMatThresholded = Mat(windowHeight,windowWidth,inputImg.type());
		for (int i=0; i < windowHeight;i++)
		{
			const unsigned char* Mi = inputImg.ptr<unsigned char>(i+yOffset);
			Mi = Mi + xOffset*inputImg.step[1];

			unsigned char * copyTo = tMat.ptr<unsigned char>(i);
			memcpy(tMat.ptr<unsigned char>(i),Mi,windowWidth);
		}		
		threshold(tMat, tMatThresholded, 100, 255, THRESH_OTSU + THRESH_BINARY);
		for (int i=0; i< windowHeight;i++)
		{
			const unsigned char* Mi = tMatThresholded.ptr<unsigned char>(i);
			unsigned char* copyTo = outputImg.ptr<unsigned char>(i+yOffset);

			copyTo = copyTo + xOffset*inputImg.step[1];
			memcpy(copyTo,Mi,windowWidth);
		}
	}
	catch (std::exception &e)
	{
		LOGE("LocalThresholding Exception: %s",e.what());
	}
	SET_TIME(&end);
	LOG_TIME("Windowed Threshold", start, end);
}

//Find the centroid and range
void ImageProcessor::GetPointAttributes(vector<Point2i> points, Point2i & centroid, int & distance)
{
	//CHANGE THESE to maxint/min-int
	Point2i max = Point2i(0,0);
	Point2i min = Point2i(100000,100000);

	for (int i=0;i<points.size();i++)
	{
		centroid.x += points.at(i).x;		
		centroid.y += points.at(i).y;
		
		if (points.at(i).x > max.x)
			max.x = points.at(i).x;
		if (points.at(i).y > max.y)
			max.y = points.at(i).y;

		if (points.at(i).x < min.x)
			min.x = points.at(i).x;
		if (points.at(i).y < min.y)
			min.y = points.at(i).y;		
	}
	Point2i range = Point2i(max.x-min.x, max.y-min.y);
	centroid.x = centroid.x / points.size();
	centroid.y = centroid.y / points.size();
	
	distance = (int)std::sqrt(std::pow(range.x,2)+std::pow(range.y,2));
}





void ImageProcessor::LocalizedThreshold(FrameItem * item)
{
	/*struct timespec start,end;
	SET_TIME(&start);

	int windowWidth = 0, windowHeight =0;
	calculateWindow(*item, &windowWidth,&windowHeight);
	localThresholding(*(item->grayImage),*(item->binaryImage),windowWidth,windowHeight);

	SET_TIME(&end);
	LOG_TIME("Local Threshold", start, end);*/
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

	/*	for (int i=0;i<inputImg.rows;i++)
		{
			const unsigned char* Mi = inputImg.ptr<unsigned char>(i);			
			unsigned char * copyTo = outputImg.ptr<unsigned char>(i);
			memcpy(copyTo,Mi,inputImg.step[0]);
		}
*/
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
		LOGE("LocalThresholding Exception: %s",e.what());
	}
	LOGD("ImageCollector","Local thresholding complete");

}