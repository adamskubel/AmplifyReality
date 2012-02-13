#include "FastQRFinder.hpp"

//class Point2iComparator
//{ 
//   public:
//      bool operator()(const Point2i pt1, const Point2i pt2)
//	  {
//		  return (pt1.x < pt2.x) && (pt1.y < pt2.y);
//	  }
//};

FastQRFinder::FastQRFinder(ARControllerDebugUI * debugUI)
{
	config = debugUI;
	srand(time(NULL));
}

static bool getPointAtXLimit(Point2i & result, map<int,map<int,Point2i>*> & xKeyMap, int xLimit, Point2i approachDirection, int yRangeLower, int yRangeUpper=0)
{
	map<int,map<int,Point2i>*>::iterator xIterator;

	xIterator = xKeyMap.lower_bound(xLimit);
	if (xIterator == xKeyMap.end())
		return false;

	if (approachDirection.x < 0)
	{
		//If approaching X from negative side, decrement as we want the value that is right before the limit
		xIterator--;
		if (xIterator == xKeyMap.end())
			return false;
	}
	else
	{
		;//If positive side, we just take the value right after the limit
	}
	
	map<int,Point2i>::iterator yIterator = (*xIterator).second->lower_bound(yRangeUpper);

	if (yIterator == (*xIterator).second->end())
		return false;

	if (approachDirection.y < 0)
	{
		//If approaching Y from negative side, decrement as we want the value that is right before the limit
		yIterator--;
		if (yIterator == (*xIterator).second->end())
			return false;
	}
	else
	{
		;//If positive side, we just take the value right after the limit
	}

	result = (*yIterator).second;
	return true;

}

static Point2i SearchRegion(map<int,map<int,Point2i>*> & pointMapX, map<int,map<int,Point2i>*> & pointMapY, int limit, Point2i yPoint, bool xMode)
{
	if (xMode)
	{
	}
	else
	{
		Point2i result;
		getPointAtXLimit(result,pointMapY,limit,Point2i(1,-1),yPoint.x)
	}
}

static Point2i FindClosestPoints(map<int,map<int,Point2i>*> & pointMapX, map<int,map<int,Point2i>*> & pointMapY, Point2i searchPoint)
{
	//map<int,map<int,Point2i>*>::iterator xIterator

	Point2i startX;
	bool result = getPointAtXLimit(startX,pointMapX,searchPoint.x,Point2i(1,1),searchPoint.y);

	Point2i startY;
	result = result && getPointAtXLimit(startY,pointMapY,searchPoint.y,Point2i(1,1),searchPoint.x);

	if (startX == startY) //I wish!
		return startX;

	int nextY = startY.y + (startX.y - startY.y)/2;
	SearchRegion(pointMapX,pointMapY,nextY,false);


	//result = result && getPointAtXLimit(startY,pointMapY,searchPoint.y,Point2i(1,1),searchPoint.x);



}

QRCode * FastQRFinder::FindQRCodes(Mat & img, Mat & binaryImage, vector<Drawable*> & debugVector)
{
	//Steps:
	//1. Divide the keypoints into regions. Regions are formed by a grid of set size. 
	//2. Score each region.
	//3. Select regions with the highest score
	//4. If more than one, combine
	//5. Score points in region

	float cellDimension = 20;
	Size2i cellSize(cellDimension,cellDimension);


	vector<KeyPoint> keypoints;
	//Do FAST
	
	struct timespec start,end;
	LOGD(LOGTAG_QRFAST,"Performing FAST detection");
	SET_TIME(&start)
	cv::FAST(img,keypoints,20,true);
	SET_TIME(&end)
	LOG_TIME("FAST",start,end);

	LOGD(LOGTAG_QRFAST,"FAST complete. %d points found.",keypoints.size());

	/*
	vector<KeyPoint>::iterator pointIterator = keypoints.begin();
	for (; pointIterator != keypoints.end(); pointIterator++)
	{
		float size = (fabs(((*pointIterator).response / 30.0f)) * 5.0f) + 5.0f;
		debugVector.push_back(new DebugCircle((*pointIterator).pt,size,((*pointIterator).response < 0.0f) ? Colors::Gold : Colors::Red));
	}*/

	//LOGD(LOGTAG_QRFAST,"Calling SURF");

	//vector<float>
	//cv::SURF()(img,Mat(),keypoints,descriptorVector,true);




	//map<int,multimap<int,Point2i>*> regionMap;
	map<int,map<int,Point2i>*> regionMap;
	map<int,map<int,Point2i>*> insideCorners;
	map<int,map<int,Point2i>*> outsideCorners;

	LOGD(LOGTAG_QRFAST,"Regioning points.");

	SET_TIME(&start);
	for (int i=0;i<keypoints.size();i++)
	{
		Point2i point = keypoints.at(i).pt;
		if (keypoints.at(i).angle > 180)
		{
			map<int,map<int,Point2i>* >::iterator it = insideCorners.find(point.x);
			if (it == insideCorners.end())
			{
				map<int,Point2i> * newMap = new map<int,Point2i>();
				insideCorners.insert(pair<int,map<int,Point2i>*>(point.x,newMap));
				newMap->insert(pair<int,Point2i>(point.x,point));
			}
			else
			{
				(*it).second->insert(pair<int,Point2i>(point.y,point));		
			}
		}
		else
		{
			map<int,map<int,Point2i>* >::iterator it = outsideCorners.find(point.x);
			if (it == outsideCorners.end())
			{
				map<int,Point2i> * newMap = new map<int,Point2i>();
				outsideCorners.insert(pair<int,map<int,Point2i>*>(point.x,newMap));
				newMap->insert(pair<int,Point2i>(point.x,point));
			}
			else
			{
				(*it).second->insert(pair<int,Point2i>(point.y,point));		
			}
		}
				
	/*			
		if (keypoints.at(i).angle > 180)
		{
			debugVector.push_back(new DebugCircle(point,5,Colors::Gold));
			interiorCount++;
		}
		else 
		{
			debugVector.push_back(new DebugCircle(point,5,Colors::Red));
			exteriorCount++;
		}*/
		
	}
	
	LOGD(LOGTAG_QRFAST,"InsideCorners=%d,OutsideCorners=%d",insideCorners.size(),outsideCorners.size());
	map<int,map<int,Point2i>*>::iterator xIterator;
	for (xIterator = insideCorners.begin();xIterator != insideCorners.end(); xIterator++)
	{
		for (map<int,Point2i>::iterator yIterator = (*xIterator).second->begin(); yIterator != (*xIterator).second->end(); yIterator++)
		{
			Point2i pt = (*yIterator).second;
			
			map<int,map<int,Point2i>*>::iterator xIterator_Out = outsideCorners.upper_bound(pt.x);
			map<int,Point2i>::iterator yIterator_Out = (*xIterator_Out).second->upper_bound(pt.y);
			float quadUUDistance = GetPointDistance((*yIterator_Out).second,pt);

			xIterator_Out = outsideCorners.lower_bound(pt.x);
			xIterator_Out--;
			yIterator_Out = (*xIterator_Out).second->upper_bound(pt.y);
			float quadLUDistance = GetPointDistance((*yIterator_Out).second,pt);

			
			xIterator_Out = outsideCorners.lower_bound(pt.x);
			xIterator_Out--;
			yIterator_Out = (*xIterator_Out).second->lower_bound(pt.y);
			yIterator_Out--;
			float quadLLDistance = GetPointDistance((*yIterator_Out).second,pt);
						
			xIterator_Out = outsideCorners.upper_bound(pt.x);
			yIterator_Out = (*xIterator_Out).second->lower_bound(pt.y);
			yIterator_Out--;
			float quadULDistance = GetPointDistance((*yIterator_Out).second,pt);

			if (std::abs(quadUUDistance - quadLLDistance) < 5)
			{
				
				debugVector.push_back(new DebugCircle(pt,quadUUDistance,Colors::Lime));
			}
			else if (std::abs(quadLUDistance - quadULDistance) < 5)
			{
				debugVector.push_back(new DebugCircle(pt,quadLUDistance,Colors::Lime));
			}

		}
	}


	SET_TIME(&end);
	LOG_TIME("RegioningPoints",start,end);

//#define FASTQR_EDGE_TESTING true
#ifdef FASTQR_EDGE_TESTING
	//Declaring iterators

		map<int,multimap<int,Point2i>*>::iterator xIterator;
		pair<multimap<int,Point2i>::iterator,multimap<int,Point2i>::iterator> yIteratorRange;
		multimap<int,Point2i>::iterator yIterator;
		
		for (xIterator = regionMap.begin(); xIterator != regionMap.end(); xIterator++)
		{
			multimap<int,Point2i> * yPointMap = (*xIterator).second;
			yIterator = yPointMap->begin();
			for (;yIterator != yPointMap->end(); yIterator++)
			{			
				Point2i pt0 = (*yIterator).second;
				Point2i pt1 = (*++yIterator).second;



				/*float edgeSize = getBestEdgeSize(3,binaryImage,Point2i(round(pt.x),round(pt.y)),4);
				LOGD(LOGTAG_QRFAST,"Edgesize = %f",edgeSize);
				debugVector.push_back(new DebugCircle(Point2i(round(pt.x),round(pt.y)),(edgeSize > 5) ? edgeSize : 5,Colors::Lime));	*/
			}
		}
#endif

	//vector<vector<Point> > contours;
	//Mat binImage = Mat(binaryImage);
	//cv::findContours(binImage,contours,CV_RETR_LIST,CV_CHAIN_APPROX_SIMPLE);
	//LOGD(LOGTAG_QRFAST,"Found %d contours",contours.size());
	//for (int j=0;j<contours.size();j++)
	//{
	//	debugVector.push_back(new DebugPoly(contours[j],Colors::Lime,2));
	//}


//#define FAST_QR_DEBUGGING true
#ifdef FAST_QR_DEBUGGING
		LOGD(LOGTAG_QRFAST,"Regioning complete. Drawing points by region.");
		Size2i maxDim = Size2i((int)round(img.cols/cellSize.width),(int)round(img.rows/cellSize.height));
		Scalar colorArray[5] = {Colors::Red,Colors::Blue,Colors::Green,Colors::Gold,Colors::Lime};
		SET_TIME(&start)
			int count =0;
		for (int x=0;x<=maxDim.width;x++)
		{
			for (int y=0;y<=maxDim.height;y++)
			{			
				Scalar regionColor = colorArray[(count++)%2];
				map<int,multimap<int,Point2i>*>::iterator it = regionMap.find(x);
				if (it != regionMap.end())
				{
					pair<multimap<int,Point2i>::iterator,multimap<int,Point2i>::iterator> innerItPair = (*it).second->equal_range(y);
					for (multimap<int,Point2i>::iterator innerIt = innerItPair.first; innerIt != innerItPair.second; innerIt++)
					{
						//LOGD(LOGTAG_QRFAST,"Region(%d,%d) has %d points",x,y,kpvec.size());
						Point2i pt = (*innerIt).second;
						debugVector.push_back(new DebugCircle(Point2i(round(pt.x),round(pt.y)),4,regionColor));					
					}
				}
			}
		}
		SET_TIME(&end)
		LOG_TIME("DrawingPoints",start,end);
#endif
//#define FAST_QR_DEBUGGING2 true
#ifdef FAST_QR_DEBUGGING2
		LOGD(LOGTAG_QRFAST,"Regioning complete. Drawing points by region.");
		Size2i maxDim = Size2i((int)round(img.cols/cellSize.width),(int)round(img.rows/cellSize.height));
		Scalar colorArray[5] = {Colors::Red,Colors::Blue,Colors::Green,Colors::Gold,Colors::Lime};
		SET_TIME(&start);
		int count =0;

		Scalar regionColor = colorArray[(count++)%2];
		map<int,map<int,Point2i>*>::iterator it = regionMap.begin();
		for (it = regionMap.begin();it != regionMap.end(); it++)
		{
			for (map<int,Point2i>::iterator yIterator = (*it).second->begin(); yIterator != (*it).second->end(); yIterator++)
			{
				//LOGD(LOGTAG_QRFAST,"Region(%d,%d) has %d points",x,y,kpvec.size());
				Point2i pt = (*yIterator).second;
				debugVector.push_back(new DebugCircle(pt,4,Scalar(100,));					
			}
		}

		SET_TIME(&end);
		LOG_TIME("DrawingPoints",start,end);
#endif



		//Random 4-point contour search
		//Select point at random
		//Choose next point based on Rule_Dist
		//Choose next point based on Rule_Dist and Rule_Angle
		//Choose next point based on Rule_Dist and Rule_Angle and Rule_Angle_Distribution
		//Evaluate contour using Rule_Evaluate

		//Size2i maxDim = Size2i((int)round(img.cols/cellSize.width),(int)round(img.rows/cellSize.height));




#ifdef USE_RANDOM_SEARCH
		int regionDistance = 1;
		//Get nearby points
		for (int attempts = 0;attempts < 20; attempts++)
		{
			Point2i randPoint;
			Point2i randPointKey;

			//Choose a random starting point
			LOGD(LOGTAG_QRFAST,"Finding random point");
			GetRandomPoint(regionMap, randPoint, randPointKey);
			//Init contour of desired type
			IDetectableContour * square = new DetectableSquare();
			
			//Add the starting point
			square->AddNextPoint(randPoint);
			
			//Look for the next point
			FindPoint(randPointKey,square,regionMap);

			if (square->IsComplete())
			{
				LOGD(LOGTAG_QRFAST,"Square found, drawing");
				debugVector.push_back(square);
				break;
			}
		}
#endif

//#define USE_ITERATIVE_SEARCH
#ifdef USE_ITERATIVE_SEARCH
		//IDetectableContour * square;
		//Size2i maxDim = Size2i((int)round(img.cols/cellSize.width),(int)round(img.rows/cellSize.height));	
		/*for (int x=0;x<=maxDim.width;x++)
		{
		for (int y=0;y<=maxDim.height;y++)
		{	*/		

		vector<vector<Point2i> > contourPoints;
		for (int attempts = 0;attempts < 20; attempts++)
		{
			Point2f randPoint;
			Point2i randPointKey;

			//Choose a random starting point
			LOGD(LOGTAG_QRFAST,"Finding random point");
			GetRandomPoint(regionMap, randPoint, randPointKey);
		//	map<float, set<float> > foundPoints;
			FindContours(binaryImage,randPoint,contourPoints, regionMap);//,foundPoints);

			//map<int,map<int,Point2i>*>::iterator it = regionMap.begin();//regionMap.find(x);
			//if (it != regionMap.end())
			//{
			//	map<int,Point2i>::iterator yIterator = (*it).second->begin();//find(y);
			//	if (yIterator != (*it).second->end())
			//	{
			//	;
			//	}

			//	/*	pair<multimap<int,Point2i>::iterator,multimap<int,Point2i>::iterator> innerItPair = (*it).second->equal_range(y);
			//	for (multimap<int,Point2i>::iterator innerIt = innerItPair.first; innerIt != innerItPair.second; innerIt++)
			//	{
			//	square = new DetectableSquare();
			//	square->AddNextPoint((*innerIt).second);
			//	FindPoint(Point2i(x,y),square,regionMap);
			//	if (square->IsComplete())
			//	{
			//	LOGD(LOGTAG_QRFAST,"Square found, drawing");
			//	debugVector.push_back(square);
			//	break;
			//	}	
			//	}*/
			//	/*	if ((*it).second->find(y) != (*it).second->end())
			//	{					

			//	}*/
			//}
		}
		LOGD(LOGTAG_QRFAST,"Found %d contours",contourPoints.size());
		for (int j=0;j<contourPoints.size();j++)
		{
			debugVector.push_back(new DebugPoly(contourPoints[j],Colors::Lime,2));
		}
		//	}
		//}
#endif

		
		//Cleanup map
		LOGV(LOGTAG_QRFAST,"Cleaning up maps");
		for (map<int,map<int,Point2i>*>::iterator deleteIt = regionMap.begin(); deleteIt != regionMap.end();deleteIt++)
		{
			delete (*deleteIt).second;
		}

		for (map<int,map<int,Point2i>*>::iterator deleteIt = outsideCorners.begin(); deleteIt != outsideCorners.end();deleteIt++)
		{
			delete (*deleteIt).second;
		}

		for (map<int,map<int,Point2i>*>::iterator deleteIt = insideCorners.begin(); deleteIt != insideCorners.end();deleteIt++)
		{
			delete (*deleteIt).second;
		}
		

		
		return NULL;
}


static bool IsEdgeBetweenPoints(Point2f pt0, Point2f pt1, Mat & img, bool & isBlack)
{	
	LOGD(LOGTAG_QRFAST,"Checking edge between (%f,%f) and (%f,%f)",pt0.x,pt0.y,pt1.x,pt1.y);
	float xDif = pt0.x - pt1.x;
	float yDif = pt0.y - pt1.y;

	float xStep = 1,yStep = 1;


	if (abs(xDif) >= abs(yDif))
	{
		yStep = yDif/xDif;
	}
	else
	{
		xStep = yDif/xDif;
	}

	int totalPx =0;
	int numChanges = 0;
	int blackCount =0 ;

	unsigned char lastPx = 0;
	for (float x=pt0.x, y = pt0.y; x < pt1.x && y < pt1.y && x < img.cols && y < img.rows; x+= xStep, y += yStep)
	{
		unsigned char px = img.at<unsigned char>((int)(round(x)),(int)round(y));
		totalPx++;

		if (px == 0)
			blackCount++;		

		if (lastPx != px && totalPx > 0)
			numChanges++;		
		lastPx = px;
	}

	if (totalPx / blackCount < 2)
		isBlack = true;
	else
		isBlack = false;
	
	return numChanges < 4;
}

static bool IsEdgeBetween(Point2i pt0, Point2i pt1, Mat & img)
{
	float spacing = 2;
	float xDif = pt0.x - pt1.x;
	float yDif = pt0.y - pt1.y;
	
	
	if (abs(xDif) < 0.01)
	{
		bool color1 = 0, color2 = 0;
		bool edge1 = IsEdgeBetweenPoints(Point2f(pt0.x  + spacing,pt0.y),Point2f(pt1.x + spacing,pt1.y), img, color1);
		bool edge2 = edge1 && IsEdgeBetweenPoints(Point2f(pt0.x - spacing,pt0.y),Point2f(pt1.x - spacing,pt1.y), img, color2);
		
	
		if (edge1 && edge2 && (color1 != color2))
			return true;

	}
	else if (abs(yDif) < 0.01)
	{		
		bool color1 = 0, color2 = 0;
		bool edge1 = IsEdgeBetweenPoints(Point2f(pt0.x,pt0.y  + spacing),Point2f(pt1.x,pt1.y  + spacing), img, color1);
		bool edge2 = edge1 && IsEdgeBetweenPoints(Point2f(pt0.x,pt0.y - spacing),Point2f(pt1.x,pt1.y - spacing), img, color2);
		
		if (edge1 && edge2 && (color1 != color2))
			return true;
	}
	else
	{		
		float invSlope = -xDif/yDif;
		float a1 = invSlope;

		float a2 = sqrt(pow(spacing,2)/(1 + pow(invSlope,2)));
		a1 = invSlope * a2;

		Point2i offset ((int)round(a1),(int)round(a2));
		bool color1 = 0, color2 = 0;
		bool edge1 = IsEdgeBetweenPoints(pt0 + offset,pt1 + offset, img,color1);
		bool edge2 = edge1 && IsEdgeBetweenPoints(pt0 - offset, pt1 - offset, img,color2);

		if (edge1 && edge2 && (color1 != color2))
			return true;
	}

	return false;
}

//True if majority of pixels are darker than center pixel
int FastQRFinder::GetCornerType(Point2i imgPoint, unsigned char threshold, Mat & img)
{
	//const int detectorRadius = 2;
	//const int totalPx = 21;
	const unsigned char majority = 9;
	int detectorX[] = {0,1,2,3,3,3,2,1,0,-1,-2,-3,-3,-3,-2,-1};
	int detectorY[] = {3,3,2,1,0,-1,-2,-3,-3,-3,-2,-1,0,1,2,3};

	
	
	/*for (int y = imgPoint.y - detectorRadius, i=0; y <= imgPoint.y + detectorRadius && y < img.rows && y > 0;y++,i++)
	{		
		const unsigned char* imgRow = img.ptr<unsigned char>(y);
		
		for (int x = imgPoint.x - detectorWidth[i]; x <= imgPoint.x + detectorWidth[i] && x < img.cols && x > 0;x++)
		{
			if (centerPx > imgRow[x])
				darkerCount++;
		}
	}*/
	//LOGD(LOGTAG_QRFAST,"Checking point(%d,%d),thresh=%d",imgPoint.x,imgPoint.y,threshold);
	int darkerCount= 0, brighterCount = 0;
	unsigned char centerPx_lower = img.at<unsigned char>(imgPoint.x,imgPoint.y) - threshold;
	unsigned char centerPx_upper = img.at<unsigned char>(imgPoint.x,imgPoint.y) + threshold;
	//LOGD(LOGTAG_QRFAST,"CenterPX-thresh = %u",centerPx);
	for (int i=0;i<16;i++)
	{
		unsigned char px = img.at<unsigned char>(detectorX[i] + imgPoint.x ,detectorY[i] + imgPoint.y);		
		LOGD(LOGTAG_QRFAST,"Px[%d]=%u ",i,px);
		if (centerPx_lower > px)
		{
			//LOGD(LOGTAG_QRFAST,"DarkerFound(%d,%d)+(%d,%d): %u > %u",imgPoint.x,imgPoint.y, detectorX[i] ,detectorY[i], centerPx_lower,px);
			darkerCount++;
		}
		else if (centerPx_upper < px)
		{			
			brighterCount++;
		}
	}

	if (darkerCount >= majority)
	{
		LOGD(LOGTAG_QRFAST,"DarkerFound(%d,%d),thresh=%d,center=%d,count=%d",imgPoint.x,imgPoint.y,threshold,centerPx_lower,darkerCount);
		return -1;
	}
	else if (brighterCount >= majority)
	{
		LOGD(LOGTAG_QRFAST,"BrighterFound(%d,%d),thresh=%d,center=%d,count=%d",imgPoint.x,imgPoint.y,threshold,centerPx_upper,brighterCount);
		return 1;
	}
	else
		return 0;

}

void FastQRFinder::FindContours(Mat & img, Point2i start, vector<vector<Point2i> > & contourPoints, map<int,map<int,Point2i>*> & regionMap)
{
	int minDelta = 3;
	//Point2i start(round(startFloat.x), round(startFloat.y));

	//get range of points
	map<int,map<int,Point2i>*>::iterator xIterator, xStart,xEnd;
	map<int,Point2i>::iterator yIterator, yStart,yEnd;
	LOGD(LOGTAG_QRFAST,"Starting at (%f,%f)",start.x,start.y);		

	for (int delta = 20; delta < 80; delta += 20)
	{
		LOGD(LOGTAG_QRFAST,"Delta=%d,minDelta=%d",delta,minDelta);

		xStart = regionMap.lower_bound(start.x - delta);
		xEnd = regionMap.upper_bound(start.x + delta);

		for (xIterator = xStart;xIterator != xEnd; xIterator++)
		{
			yStart = (*xIterator).second->lower_bound(start.y - delta);
			yEnd = (*xIterator).second->upper_bound(start.y + delta);

			for (yIterator = yStart; yIterator != yEnd; yIterator++)
			{
				Point2i pt = (*yIterator).second;
			//	LOGD(LOGTAG_QRFAST,"Pt=(%f,%f)",pt.x,pt.y);
				//Ensure points are far enough away
			//	map<float, set<float> >::iterator fpIt = foundPoints.find(pt.x);
			//	if (fpIt != foundPoints.end() && (*fpIt).second.find(pt.y) != (*fpIt).second.end())

				//LOGD(LOGTAG_QRFAST,"Checking for existing points");
				if (!contourPoints.empty())
					for (int i=0;i<contourPoints.back().size();i++)
					{
						if (contourPoints.back()[i] == pt)
						{
							LOGD(LOGTAG_QRFAST,"Point already added: (%f,%f)",pt.x,pt.y);
							continue;
						}
					}

				if (abs(pt.x - start.x) < minDelta && abs(pt.y - start.y) < minDelta)
				{
					LOGD(LOGTAG_QRFAST,"Points too close: (%f,%f) and (%f,%f)",pt.x,pt.y,start.x,start.y);
					continue;
				}

				//LOGD(LOGTAG_QRFAST,"Checking between (%f,%f) and (%f,%f)",pt.x,pt.y,start.x,start.y);				
				if (IsEdgeBetween(start,pt, img))
				{
					LOGD(LOGTAG_QRFAST,"Found between (%f,%f) and (%f,%f)",pt.x,pt.y,start.x,start.y);
					if (contourPoints.empty())
					{
						contourPoints.push_back(vector<Point2i>());
						contourPoints.back().push_back(start);
					}
					else if (contourPoints.back().empty())
					{
						contourPoints.back().push_back(start);
					}
				//	foundPoints[pt.x].insert(pt.y);


					contourPoints.back().push_back(pt);
					if (contourPoints.back().size() == 4)
					{
						contourPoints.push_back(vector<Point2i>());
						return;
					}
					else
					{
						FindContours(img,pt,contourPoints,regionMap);
					}
				}
			}
		}
		minDelta = delta;
	}





	/*for (int y = start.y - delta; y <= start.y + delta;y++)
	{
		for (int x = start.x - delta; x <= start.x + delta; x++)
		{
			if ((x != start.x + delta && x != start.x - delta) && (y != start.y + delta && y != start.y - delta))
				continue;


		}
	}*/
}

bool FastQRFinder::IsEdge(int detectorRadius, Mat & img, Point2i imgPoint, Point2i & exitPoint)
{
	int blackCount = 0;
	int whiteCount = 0;
	int xChanges = 0;
	int yChanges = 0;
	int totalPx = detectorRadius * detectorRadius * 4;
	int linearity = 0;
	//int contigousWhite = 0;

	
	unsigned char lastPx = -1;
	Point2i lastChangePos = Point2i(-1,-1);
	const unsigned char* lastImgRow;
	for (int y = imgPoint.y - detectorRadius; y < imgPoint.y + detectorRadius && y < img.rows && y > 0;y++)
	{		
		const unsigned char* imgRow = img.ptr<unsigned char>(y);
		
		for (int x = imgPoint.x - detectorRadius; x < imgPoint.x + detectorRadius && x < img.cols && x > 0;x++)
		{
			unsigned char px = imgRow[x];
			if (px == 0)
				blackCount++;
			else
				whiteCount++;
			
			lastPx = px;

			if (x != imgPoint.x - detectorRadius && lastPx != px)
			{
				xChanges++;
				if (lastChangePos.x >= 0)
				{
					if (abs(lastChangePos.x - x) < 3  && abs(lastChangePos.y - y) < 3)
					{
						linearity++;
						lastChangePos = Point2i(x,y);
						continue; //Dont want to increment or reset point for y
					}
				}
				else
					lastChangePos = Point2i(x,y);
			}

			if (y != imgPoint.y - detectorRadius && lastImgRow[x] != px)
			{
				yChanges++;
				if (lastChangePos.x >= 0)
				{
					if (abs(lastChangePos.x - x) < 3  && abs(lastChangePos.y - y) < 3)
					{
						linearity++;
						lastChangePos = Point2i(x,y);
					}
				}
				else
					lastChangePos = Point2i(x,y);
			}
		}
		lastImgRow = imgRow;
	}
	
	LOGD(LOGTAG_QRFAST,"Whitecount=%d,Blackcount=%d,XYChanges=(%d,%d),TotalPx = %d, Linearity = %d",whiteCount,blackCount,xChanges,yChanges,totalPx,linearity);
	totalPx = totalPx * 10;
	exitPoint = lastChangePos;
	return ((float)linearity > ((float)detectorRadius * 1.5f)) || ((totalPx / blackCount) > 15 && (totalPx / whiteCount) > 15 && (xChanges < detectorRadius * 2) && (yChanges < detectorRadius * 2));
}

float FastQRFinder::getBestEdgeSize(int detectorRadius, Mat & img, Point2i imgPoint, int recurseCount)
{
	if (recurseCount-- == 0)
		return 0;

	int delta = 1;
	int edgeSize = 0;
	for (int y = imgPoint.y - delta; y <= imgPoint.y + delta;y++)
	{
		for (int x = imgPoint.x - delta; x <= imgPoint.x + delta; x++)
		{
			if ((x != imgPoint.x + delta && x != imgPoint.x - delta) && (y != imgPoint.y + delta && y != imgPoint.y - delta))
				continue;
			Point2i endPoint;

			if (IsEdge(detectorRadius,img,Point2i(x,y),endPoint))
			{
				return edgeSize = edgeSize + detectorRadius + getBestEdgeSize(detectorRadius, img, endPoint, recurseCount);				
			}
			else
			{
				return 0;
			}
		}
	}
}

void FastQRFinder::SegmentByQuantity(map<int,multimap<int,Point2i>*> & regionMap, map<int,multimap<int,Point2i>*> & quantityMap)
{
		//Declaring iterators
		map<int,multimap<int,Point2i>*>::iterator xIterator;
		pair<multimap<int,Point2i>::iterator,multimap<int,Point2i>::iterator> yIteratorRange;
		multimap<int,Point2i>::iterator yIterator;

		xIterator = regionMap.begin();

		for (xIterator = regionMap.begin(); xIterator != regionMap.end(); xIterator++)
		{
			multimap<int,Point2i> * yPointMap = (*xIterator).second;
			yIterator = yPointMap->begin();
			for (;yIterator != yPointMap->end(); yIterator++)
			{

			}
		}

		

}

void FastQRFinder::GetRandomPoint(map<int,map<int,Point2i>*> & regionMap, Point2i & randPoint, Point2i & randPointKey)
{
	if (regionMap.size() == 0)
		return;
	//Declaring iterators
	map<int,map<int,Point2i>*>::iterator xIterator;
	map<int,Point2i>::iterator yIterator;

	xIterator = regionMap.begin();

	//Find a random point
	int randX = rand() % regionMap.size();
	LOGD(LOGTAG_QRFAST,"RandX = %d",randX);
	for (int i=0; i < randX; i++, xIterator++);
	map<int,Point2i> * yPointMap = (*xIterator).second;
	int randY = rand() %  yPointMap->size();
	yIterator = yPointMap->begin();
	for (int i=0;i<randY;i++, yIterator++);
	randPointKey = Point2i((*xIterator).first,(*yIterator).first);
	randPoint = (*yIterator).second;
	LOGD(LOGTAG_QRFAST,"Starting at random point(%d,%d)->(%f,%f)",randPointKey.x,randPointKey.y,randPoint.x,randPoint.y);
}

void FastQRFinder::FindPoint(Point2i region, IDetectableContour * contour, map<int,multimap<int,Point2i>*> & regionMap)
{
	Size2i cellSize(20,20);
	map<int,multimap<int,Point2i>*>::iterator xIterator;
	pair<multimap<int,Point2i>::iterator,multimap<int,Point2i>::iterator> yIteratorRange;
	multimap<int,Point2i>::iterator yIterator;
	int regionDistance = 1;
	LOGD(LOGTAG_QRFAST,"Looking for points, starting at region (%d,%d)",region.x,region.y);

	for (int xIndex = region.x - regionDistance; xIndex < region.x + regionDistance; xIndex++)
	{
		xIterator = regionMap.find(xIndex);
		if (xIterator != regionMap.end())
		{
			for (int yIndex = region.y - regionDistance; yIndex < region.y + regionDistance; yIndex++)
			{
				if (yIndex == region.y && xIndex == region.x) continue; //Skip start region

				yIteratorRange = (*xIterator).second->equal_range(yIndex);
				Point2i chosenPoint;
				vector<Point2i> pointsToChoose; //(yIteratorRange.first,yIteratorRange.second);
				for (yIterator = yIteratorRange.first; yIterator != yIteratorRange.second; yIterator++)
				{
					pointsToChoose.push_back((*yIterator).second);
				}
				
				if (pointsToChoose.empty())
					continue;


				float result = contour->ChooseNextPoint(pointsToChoose,chosenPoint, regionMap);						
				if (contour->IsComplete())
				{
					break;
				}
				else if (result > 0)
				{
					LOGD(LOGTAG_QRFAST,"Point added to square.(%f,%f)",chosenPoint.x,chosenPoint.y);
					FindPoint(GetRegion(chosenPoint,cellSize),contour,regionMap);
					break;
				}
			}
		}
	}
}

Point2i FastQRFinder::GetRegion(Point2i point, Size2i regionSize)
{
	return Point2i((int)round(point.x/regionSize.width),(int)round(point.y/regionSize.height));
}


bool FastQRFinder::PointInRegion(Point2i point, Point2i cellPosition, Size2i regionSize)
{
	if (point.x < regionSize.width * cellPosition.x)
		return false;
	if (point.x > regionSize.width * (cellPosition.x+1))
		return false;

	if (point.y < (regionSize.height * cellPosition.y))
		return false;
	if (point.y > (regionSize.height * (cellPosition.y+1)))
		return false;

	return true;
}