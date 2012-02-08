#include "FastTracking.hpp"

//1. Determine boundaries
//2. cv::FAST
//3. Concentric segmentation
//4. Find lines/pattern corners
//5. Mark outermost corners
void FastTracking::DoFastTracking(Mat & img, QRCode * code, vector<Drawable*> & debugVector)
{
	if (code == NULL || code->finderPatterns == NULL)
		return;

	LOGD(LOGTAG_QR,"Starting fast tracking. %d patterns found.",code->finderPatterns->size());
	for (int i=0;i<code->finderPatterns->size();i++)
	{
		FinderPattern * fp = code->finderPatterns->at(i);
		
		float size = fp->size;
		size *= 1.2f; //Window a bit bigger.

		Rect window = Rect(fp->pt.x - size/2.0f, fp->pt.y-size/2.0f, size,size);
		debugVector.push_back(new DebugRectangle(window,Colors::Aqua));

		vector<KeyPoint> features;
		
		LOGD(LOGTAG_QR,"Calling FastWindow");
		FastWindow(img,features,window);
		
		
		float innerMost = fp->patternWidths[2]/2.0f;
		float middle = innerMost + (fp->patternWidths[1]+fp->patternWidths[3])/2.0f;
		float outer = middle + (fp->patternWidths[0]+fp->patternWidths[4])/2.0f;
		
		innerMost *= 1.4f;
		middle *= 1.4f;
		outer *= 1.4f;
		
		debugVector.push_back(new DebugCircle(fp->pt,innerMost,Colors::Red));
		debugVector.push_back(new DebugCircle(fp->pt,middle,Colors::Lime));
		debugVector.push_back(new DebugCircle(fp->pt,outer,Colors::Blue));

		vector<KeyPoint> * kpVectorArray = new vector<KeyPoint>[3];

		LOGD(LOGTAG_QR,"Sorting based on concentric region. Bounds = [%f,%f,%f]",innerMost, middle, outer);

		//Sort based on region
		for (int i=0;i<features.size();i++)
		{
			float distance = GetPointDistance(features.at(i).pt,fp->pt);

			if (distance < innerMost)
			{
				kpVectorArray[0].push_back(features.at(i));
				debugVector.push_back(new DebugCircle(features.at(i).pt,3,Colors::Red,true));
			}
			else if (distance < middle)
			{
				kpVectorArray[1].push_back(features.at(i));
				debugVector.push_back(new DebugCircle(features.at(i).pt,3,Colors::Lime,true));
			}
			else 
			{
				kpVectorArray[2].push_back(features.at(i));
				debugVector.push_back(new DebugCircle(features.at(i).pt,3,Colors::Blue,true));
			}
		}

		LOGD(LOGTAG_QR,"Deleting vector array");
		delete[] kpVectorArray;
	}

}

float FastTracking::GetPointDistance(Point2f p1, Point2f p2)
{
	return sqrt(pow(abs(p1.x-p2.x),2) + pow(abs(p1.y-p2.y),2));
}

//void FastTracking::ConcentricSegmentation( vector<KeyPoint> & features, Point2i center, float innerMost, float middle, float outerMost
//	,  vector<KeyPoint> *& kpVectorArray)
//{
//	//Calculate distance
//	
//
//}

void FastTracking::FastWindow(Mat & inputImg, vector<KeyPoint> & features, Rect window)
{
	struct timespec start,end;
	SET_TIME(&start);	
	
	try
	{	
		int xOffset = window.x, yOffset = window.y;
		int windowWidth = window.width, windowHeight = window.height;
		
			//Check that window is within boundaries
		if (xOffset < 0 || xOffset >= inputImg.cols)
		{	xOffset = 0;}
		if (yOffset < 0 || yOffset >= inputImg.rows)
		{	yOffset = 0; }
		if (xOffset + windowWidth < 0 || xOffset + windowWidth > inputImg.cols)
		{	windowWidth = inputImg.cols-xOffset;}
		if (yOffset +  windowHeight < 0 || yOffset + windowHeight > inputImg.rows)
		{	windowHeight = inputImg.rows-yOffset;}
					
		Mat tMat = Mat(windowHeight,windowWidth,inputImg.type());
		for (int i=0; i < windowHeight;i++)
		{
			const unsigned char* Mi = inputImg.ptr<unsigned char>(i+yOffset);
			Mi = Mi + xOffset*inputImg.step[1];

			unsigned char * copyTo = tMat.ptr<unsigned char>(i);
			memcpy(tMat.ptr<unsigned char>(i),Mi,windowWidth);
		}
		cv::FAST(tMat,features,9,true);
		for (int i=0;i<features.size();i++)
		{
			features.at(i).pt.x += xOffset;
			features.at(i).pt.y += yOffset;
		}
	}
	catch (std::exception &e)
	{
		LOGE("FastWindow exception: %s",e.what());
	}
	SET_TIME(&end);
	LOG_TIME("Windowed FAST", start, end);
}