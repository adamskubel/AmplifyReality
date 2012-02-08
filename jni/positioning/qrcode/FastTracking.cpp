#include "FastTracking.hpp"


//1. Determine boundaries
//2. cv::FAST
//3. Concentric segmentation
//4. Find lines/pattern corners
//5. Mark outermost corners
//NOPE

//3. Sort points by distance to "center" of pattern
//4. Choose 4 outermost points and test for squareness
//5. Repeat
//6. Choose most square, mark corners
void FastTracking::DoFastTracking(Mat & img, QRCode * code, vector<Drawable*> & debugVector)
{
	if (code == NULL || code->finderPatterns == NULL)
		return;

	LOGD(LOGTAG_QRFAST,"Starting fast tracking. %d patterns found.",code->finderPatterns->size());
	for (int i=0;i<code->finderPatterns->size();i++)
	{
		FinderPattern * fp = code->finderPatterns->at(i);
		
		float size = fp->size;
		size *= 1.2f; //Window a bit bigger.

		Rect window = Rect(fp->pt.x - size/2.0f, fp->pt.y-size/2.0f, size,size);
		debugVector.push_back(new DebugRectangle(window,Colors::Aqua));

		vector<KeyPoint> features;
		
		LOGD(LOGTAG_QRFAST,"Calling FastWindow");
		FastWindow(img,features,window);
		
		map<float,Point2f> pointDistances;	
		//Sort based on distance from center
		for (int i=0;i<features.size();i++)
		{
			float distance = GetPointDistance(features.at(i).pt,fp->pt);
			pointDistances.insert(pair<float,Point2f>(distance,features.at(i).pt));
		}

		//sort(pointDistances.begin(),pointDistances.end());

		float minSquareSideLength = size * 0.5f;
		float maxSquareSideLength = size;
		float maxPointDistance = size * 1.4f;

		float minArea = pow(minSquareSideLength,2);
		float maxArea = pow(maxSquareSideLength,2);

		float minCosine = 0.3f;
		
		//Search boundaries
		map<float,Point2f>::iterator searchBegin = pointDistances.upper_bound(maxPointDistance);
		map<float,Point2f>::iterator searchEnd = pointDistances.lower_bound(minSquareSideLength);

		vector<Point2f> testPoints;

		//Put the most distant point on the vector
		//testPoints.push_back((*searchBegin).second);
		
		bool found = false;
		map<float,Point2f>::iterator it;
		for (it = searchBegin;it != searchEnd && !found; it--)
		{		
			testPoints.push_back((*it).second);
			LOGD(LOGTAG_QRFAST,"Starting with point (%f,%f)",testPoints.back().x,testPoints.back().y);
			//Look for four points of appropriate distance
			map<float,Point2f>::iterator innerIterator;
			for (innerIterator = it;innerIterator != searchEnd; innerIterator--)
			{
				float distance = GetPointDistance(testPoints.back(),(*innerIterator).second);
				if (distance > minSquareSideLength && distance < maxSquareSideLength)
				{
					testPoints.push_back((*innerIterator).second);		
					LOGD(LOGTAG_QRFAST,"Added point (%f,%f)",testPoints.back().x,testPoints.back().y);
					if (testPoints.size() == 4)
					{							
						float area = getArea(testPoints);
						if (area < maxArea && area > minArea && getCosine(testPoints) < minCosine)
						{
							debugVector.push_back(new DebugPoly(testPoints,Colors::Gold));
							found = true;
							break;
						}else
						{
							LOGD(LOGTAG_QRFAST,"Square check failed, cos = %f, area = %f (min=%f,max=%f)",getCosine(testPoints),area,minArea,maxArea);			
							testPoints.pop_back();
						}
					}
				}
			}
			LOGD(LOGTAG_QRFAST,"No squares found, trying next point");
			testPoints.clear();
		}


	}

}

float FastTracking::getPolyArea(Point2f a, Point2f b, Point2f c)
{
	float vec1[] = {b.x - a.x, b.y - a.y, 0};
	float vec2[] = {b.x - c.x, b.y - c.y, 0};
	Mat m1(1,3,CV_32F,vec1);
	Mat m2(1,3,CV_32F,vec2);
	Mat crossProduct = m1.cross(m2);
	float result = fabs(crossProduct.dot(crossProduct));
	return sqrt(result)/2.0f;
}

float FastTracking::getArea(vector<Point2f> & testPoints)
{
	if (testPoints.size() != 4)
	{
		LOGE("Vector is wrong size.");
		throw exception();
	}

	float maxArea = 0;
	for (int i=0;i<4;i++)
	{
		float area = getPolyArea(testPoints[i%4],testPoints[(i+1)%4],testPoints[(i+2)%4]);			
		maxArea = MAX(maxArea,area);
	}

	return maxArea;
}

static double angle( Point pt1, Point pt2, Point pt0 )
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

float FastTracking::getCosine(vector<Point2f> & testPoints)
{
	if (testPoints.size() != 4)
	{
		LOGE("Vector is wrong size.");
		throw exception();
	}

	float maxCosine = 0;
	for (int i=0;i<4;i++)
	{
		float cosine = fabs(angle(testPoints[i%4],testPoints[(i+1)%4],testPoints[(i+2)%4]));
		maxCosine = MAX(maxCosine, cosine);
	}

	return maxCosine;
}




float FastTracking::GetPointDistance(Point2f p1, Point2f p2)
{
	return sqrt(pow(abs(p1.x-p2.x),2) + pow(abs(p1.y-p2.y),2));
}


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


/*

void FastTracking::DoSquareTracking(Mat & img, QRCode * code, vector<Drawable*> & debugVector)
{
	if (code == NULL || code->finderPatterns == NULL)
		return;

	LOGD(LOGTAG_QRFAST,"Starting fast tracking. %d patterns found.",code->finderPatterns->size());
	for (int i=0;i<code->finderPatterns->size();i++)
	{
		FinderPattern * fp = code->finderPatterns->at(i);
		
		float size = fp->size;
		size *= 1.2f; //Window a bit bigger.

		Rect window = Rect(fp->pt.x - size/2.0f, fp->pt.y-size/2.0f, size,size);
		debugVector.push_back(new DebugRectangle(window,Colors::Aqua));

		struct timespec start,end;
		SET_TIME(&start);	 
		vector<vector<Point> > squares;

		int xOffset = window.x, yOffset = window.y;
		int windowWidth = window.width, windowHeight = window.height;
		
			//Check that window is within boundaries
		if (xOffset < 0 || xOffset >= img.cols)
		{	xOffset = 0;}
		if (yOffset < 0 || yOffset >= img.rows)
		{	yOffset = 0; }
		if (xOffset + windowWidth < 0 || xOffset + windowWidth > img.cols)
		{	windowWidth = img.cols-xOffset;}
		if (yOffset +  windowHeight < 0 || yOffset + windowHeight > img.rows)
		{	windowHeight = img.rows-yOffset;}

		window = Rect(xOffset,yOffset,windowWidth,windowHeight);


		Mat roiMat = Mat(img,window);
		findSquares(roiMat,squares);

		for( size_t i = 0; i < squares.size(); i++ )
		{
			debugVector.push_back(new DebugPoly(squares[i],Colors::Red,2));	
		}

		SET_TIME(&end);
		LOG_TIME("Square tracking", start, end);

	}


	
}


//******** OPEN CV SQUARE DETECTION DEMO ***********



// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
static double angle( Point pt1, Point pt2, Point pt0 )
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
void FastTracking::findSquares( const Mat& image, vector<vector<Point> >& squares )
{
	int thresh = 50, N = 2;
	squares.clear();

	//Mat gray;

	// down-scale and upscale the image to filter out the noise

	vector<vector<Point> > contours;

	Mat outputMat = Mat(image);

	// try several threshold levels
	for( int l = 0; l < N; l++ )
	{
		//// SLOW: use Canny instead of zero threshold level.
		//// Canny helps to catch squares with gradient shading
		//if( l == 0 )
		//{
		//	// apply Canny. Take the upper threshold from slider
		//	// and set the lower to 0 (which forces edges merging)
		//	Canny(image, gray, 0, thresh, 5);
		//	// dilate canny output to remove potential
		//	// holes between edge segments
		//	dilate(gray, gray, Mat(), Point(-1,-1));
		//}
		//else
		//{
		//	// apply threshold if l!=0:
		//	//     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
		//	gray = image >= (l+1)*255/N;
		//}

		// find contours and store them all as a list
		findContours(outputMat, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

		vector<Point> approx;

		// test each contour
		for( size_t i = 0; i < contours.size(); i++ )
		{
			// approximate contour with accuracy proportional
			// to the contour perimeter
			approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);

			// square contours should have 4 vertices after approximation
			// relatively large area (to filter out noisy contours)
			// and be convex.
			// Note: absolute value of an area is used because
			// area may be positive or negative - in accordance with the
			// contour orientation
			if( approx.size() == 4 &&
				fabs(contourArea(Mat(approx))) > 1000 &&
				isContourConvex(Mat(approx)) )
			{
				double maxCosine = 0;

				for( int j = 2; j < 5; j++ )
				{
					// find the maximum cosine of the angle between joint edges
					double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
					maxCosine = MAX(maxCosine, cosine);
				}

				// if cosines of all angles are small
				// (all angles are ~90 degree) then write quandrange
				// vertices to resultant sequence
				if( maxCosine < 0.3 )
					squares.push_back(approx);
			}


		}
	}
}

*/