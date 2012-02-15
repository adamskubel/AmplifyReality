#include "FastQRFinder.hpp"

FastQRFinder::FastQRFinder(ARControllerDebugUI * debugUI)
{
	config = debugUI;
	//config->AddNewParameter("NonMaxSuppress",1,1,0,1,"%1.f");
	srand(time(NULL));
}

//static bool getPointAtXLimit(Point2i & result, map<int,map<int,Point2i>*> & xKeyMap, int xLimit, Point2i approachDirection, int yRangeLower, int yRangeUpper=0)
//{
//	map<int,map<int,Point2i>*>::iterator xIterator;
//
//	xIterator = xKeyMap.lower_bound(xLimit);
//	if (xIterator == xKeyMap.end())
//		return false;
//
//	if (approachDirection.x < 0)
//	{
//		//If approaching X from negative side, decrement as we want the value that is right before the limit
//		xIterator--;
//		if (xIterator == xKeyMap.end())
//			return false;
//	}
//	else
//	{
//		;//If positive side, we just take the value right after the limit
//	}
//	
//	map<int,Point2i>::iterator yIterator = (*xIterator).second->lower_bound(yRangeUpper);
//
//	if (yIterator == (*xIterator).second->end())
//		return false;
//
//	if (approachDirection.y < 0)
//	{
//		//If approaching Y from negative side, decrement as we want the value that is right before the limit
//		yIterator--;
//		if (yIterator == (*xIterator).second->end())
//			return false;
//	}
//	else
//	{
//		;//If positive side, we just take the value right after the limit
//	}
//
//	result = (*yIterator).second;
//	return true;
//
//}
//
////static Point2i SearchRegion(map<int,map<int,Point2i>*> & pointMapX, map<int,map<int,Point2i>*> & pointMapY, int limit, Point2i yPoint, bool xMode)
////{
////	if (xMode)
////	{
////	}
////	else
////	{
////		Point2i result;
////		getPointAtXLimit(result,pointMapY,limit,Point2i(1,-1),yPoint.x)
////	}
////}
//
//static Point2i FindClosestPoints(map<int,map<int,Point2i>*> & pointMapX, map<int,map<int,Point2i>*> & pointMapY, Point2i searchPoint)
//{
//	////map<int,map<int,Point2i>*>::iterator xIterator
//
//	//Point2i startX;
//	//bool result = getPointAtXLimit(startX,pointMapX,searchPoint.x,Point2i(1,1),searchPoint.y);
//
//	//Point2i startY;
//	//result = result && getPointAtXLimit(startY,pointMapY,searchPoint.y,Point2i(1,1),searchPoint.x);
//
//	//if (startX == startY) //I wish!
//	//	return startX;
//
//	//int nextY = startY.y + (startX.y - startY.y)/2;
//	//SearchRegion(pointMapX,pointMapY,nextY,false);
//
//
//	//result = result && getPointAtXLimit(startY,pointMapY,searchPoint.y,Point2i(1,1),searchPoint.x);
//
//
//
//}

static int FastDistance(int dx, int dy)
{
	return ipow(dx,2) + ipow(dy,2);
	//dx=abs(dx);
	//dy=abs(dy);
	//int mn = min(dx,dy);

	//return(dx+dy-(mn>>1)-(mn>>2)+(mn>>4));
} //End FastDistance

static int FastDistance(Point2i pt0, Point2i pt1)
{
	return ipow(pt0.x-pt1.x,2) + ipow(pt0.y-pt1.y,2);
	//int dx=abs(pt0.x-pt1.x);
	//int dy=abs(pt0.y-pt1.y);
	//int mn = min(dx,dy);

	//return(dx+dy-(mn>>1)-(mn>>2)+(mn>>4));
} //End FastDistance

static KeyPoint getBestKeypoint(KeyPoint startPoint, map<int,map<int,KeyPoint>*> & pointMap, Size2i range)
{
	map<int,map<int,KeyPoint>*>::iterator xSearchIterator, xSearchEnd;
	map<int,KeyPoint>::iterator ySearchIterator, ySearchEnd;

	xSearchIterator = pointMap.lower_bound((int)startPoint.pt.x-range.width);
	xSearchEnd = pointMap.upper_bound((int)startPoint.pt.x + range.width);

	float maxScore = 0;
	KeyPoint maxPoint;

	for (;xSearchIterator != xSearchEnd;xSearchIterator++)
	{
		ySearchIterator = (*xSearchIterator).second->lower_bound((int)startPoint.pt.y - range.height);
		ySearchEnd = (*xSearchIterator).second->upper_bound((int)startPoint.pt.y + range.height);

		for (;ySearchIterator != ySearchEnd; ySearchIterator++)
		{
			KeyPoint keyPoint = (*ySearchIterator).second;
			if (keyPoint.response > maxScore)
			{
				maxScore = keyPoint.response;
				maxPoint = keyPoint;
			}
		}
	}
	if (maxScore == 0)
		return startPoint;
	else
		return maxPoint;
}

static bool searchSetForPoint(Point2i pt, map<int,set<int> > & searchSetMap, Size2i range)
{
	map<int,set<int> >::iterator xSearchIterator, xSearchEnd;
	set<int>::iterator ySearchIterator, ySearchEnd;

	xSearchIterator = searchSetMap.lower_bound(pt.x-range.width);
	xSearchEnd = searchSetMap.upper_bound(pt.x + range.width);

	for (;xSearchIterator != searchSetMap.end();xSearchIterator++)
	{
		//[0 1 2 4 5 7 9]
		//Search 4, 7
		// It1 = 4, It2 = 9
		// 4!=9
		//Search 5
		// It1 = 5, It2 = 7
		//good
		ySearchIterator = (*xSearchIterator).second.lower_bound(pt.y - range.height);
		ySearchEnd = (*xSearchIterator).second.upper_bound(pt.y + range.height);

		if (ySearchIterator != ySearchEnd)
		{
			return true;
		}
	}
	return false;
}

static bool searchSetForPoint(Point2i pt, map<int,set<int> > & searchSetMap)
{
	map<int,set<int> >::iterator searchIterator;
	searchIterator = searchSetMap.find( pt.x);
	if (searchIterator != searchSetMap.end())
	{
		if ((*searchIterator).second.count(pt.y) != 0)
		{
			return true;
		}
	}
	return false;
}

static void getClosestByQuadrant(map<int,map<int,Point2i>*> & points, Point2i start, map<int,Point2i> & closest)
{

	for (int i=0;i<4;i++)
	{
		//0 = (1,1)
		//1 = (1,-1)
		//2 = (-1,1)
		//3 = (-1,-1)

		map<int,Point2i> closestPoints;

		if (i == 2 || i == 3)
		{
			map<int,map<int,Point2i>*>::reverse_iterator xIterator(points.lower_bound(start.x));
			for (; xIterator != points.rend();++xIterator)
			{
				if (i == 2)
				{
					map<int,Point2i>::iterator yIterator = (*xIterator).second->lower_bound(start.y);
					for (; yIterator != (*xIterator).second->end(); yIterator++)
					{
						Point2i testPoint = (*yIterator).second;
						int dist = FastDistance(start,testPoint);
						if (dist > 0)
						{
							closestPoints.insert(pair<int,Point2i>(dist,testPoint));
						}
					}
				}
				else
				{					
					map<int,Point2i>::reverse_iterator yIterator((*xIterator).second->lower_bound(start.y));
					for (; yIterator != (*xIterator).second->rend();++yIterator)
					{
						Point2i testPoint = (*yIterator).second;
						int dist = FastDistance(start,testPoint);
						if (dist > 0)
						{
							closestPoints.insert(pair<int,Point2i>(dist,testPoint));
						}
					}
				}
			}
		} 
		else 
		{			

			map<int,map<int,Point2i>*>::iterator xIterator = points.lower_bound(start.x);
			for (; xIterator != points.end();xIterator++)
			{				
				if (i == 0)
				{
					map<int,Point2i>::iterator yIterator = (*xIterator).second->lower_bound(start.y);
					for (; yIterator != (*xIterator).second->end(); yIterator++)
					{
						Point2i testPoint = (*yIterator).second;
						int dist = FastDistance(start,testPoint);
						if (dist > 0)
						{
							closestPoints.insert(pair<int,Point2i>(dist,testPoint));
						}
					}
				}
				else
				{					
					map<int,Point2i>::reverse_iterator yIterator = map<int,Point2i>::reverse_iterator((*xIterator).second->lower_bound(start.y));
					for (; yIterator != (*xIterator).second->rend();++yIterator)
					{
						Point2i testPoint = (*yIterator).second;
						int dist = FastDistance(start,testPoint);
						if (dist > 0)
						{
							closestPoints.insert(pair<int,Point2i>(dist,testPoint));
						}
					}
				}
			}
		}
		closest.insert(pair<int,Point2i>(*closestPoints.begin()));
	}


}

static void sortKPMap(const map<int,map<int,KeyPoint>*> & keyPointMap_Start, map<int,map<int,KeyPoint>*> & sortedMap_Out, int numIterations, Size2i regionSize)
{
	map<int,map<int,KeyPoint>*> keyPointMap = keyPointMap_Start;
	map<int,map<int,KeyPoint>*> sortedMap;

	for (int i=0;i< numIterations;i++)
	{
		for (map<int,map<int,KeyPoint>*>::iterator kpXIterator = keyPointMap.begin(); kpXIterator != keyPointMap.end();kpXIterator++)
		{
			for (map<int,KeyPoint>::iterator kpYIterator = (*kpXIterator).second->begin(); kpYIterator != (*kpXIterator).second->end();kpYIterator++)
			{
				KeyPoint kp1 = (*kpYIterator).second;
				KeyPoint kp = getBestKeypoint(kp1,keyPointMap,regionSize);

				//if (kp1.pt != kp.pt)
				//	LOGV(LOGTAG_QRFAST,"Kp[%d,%d] = (%d,%d)",(int)kp1.pt.x,(int)kp1.pt.y,(int)kp.pt.x,(int)kp.pt.y);

				//Insert the kp.pt
				map<int,map<int,KeyPoint>* >::iterator it = sortedMap.find((int)kp.pt.x);
				if (it == sortedMap.end())
				{
					map<int,KeyPoint> * newMap = new map<int,KeyPoint>();
					sortedMap.insert(pair<int,map<int,KeyPoint>*>((int)kp.pt.x,newMap));
					newMap->insert(pair<int,KeyPoint>((int)kp.pt.x,kp));
				}
				else
				{
					(*it).second->insert(pair<int,KeyPoint>((int)kp.pt.y,kp));		
				}
			}
		}

		LOGV(LOGTAG_QRFAST,"Iteration %d complete. Rows: (%d) -> (%d)",i,keyPointMap.size(),sortedMap.size());

		//Only want to delete intermediate maps
		if (i > 0)
		{
			LOGV(LOGTAG_QRFAST,"Cleaning up map");
			for (map<int,map<int,KeyPoint>*>::iterator deleteIt = keyPointMap.begin(); deleteIt != keyPointMap.end();deleteIt++)
			{
				delete (*deleteIt).second;
			}
		}

		keyPointMap = sortedMap;
		sortedMap = map<int,map<int,KeyPoint>*>();
	}
	sortedMap_Out = keyPointMap;
	LOGD(LOGTAG_QRFAST,"Sorting complete.");
}

static void sortPointsByAngle(map<int,map<int,KeyPoint>*> & keyPointMap_Outer, map<int,map<int,KeyPoint>*> & keyPointMap_Inner, const vector<KeyPoint> keypoints)
{
	for (int i=0;i<keypoints.size();i++)
	{
		KeyPoint kp = keypoints[i];
		Point2i point = Point2i((int)kp.pt.x,(int)kp.pt.y);
		if (kp.angle > 180)
		{
			map<int,map<int,KeyPoint>* >::iterator it = keyPointMap_Inner.find(point.x);
			if (it == keyPointMap_Inner.end())
			{
				map<int,KeyPoint> * newMap = new map<int,KeyPoint>();
				keyPointMap_Inner.insert(pair<int,map<int,KeyPoint>*>(point.x,newMap));
				newMap->insert(pair<int,KeyPoint>(point.x,kp));
			}
			else
			{
				(*it).second->insert(pair<int,KeyPoint>(point.y,kp));		
			}
		}
		else
		{
			map<int,map<int,KeyPoint>* >::iterator it = keyPointMap_Outer.find(point.x);
			if (it == keyPointMap_Outer.end())
			{
				map<int,KeyPoint> * newMap = new map<int,KeyPoint>();
				keyPointMap_Outer.insert(pair<int,map<int,KeyPoint>*>(point.x,newMap));
				newMap->insert(pair<int,KeyPoint>(point.x,kp));
			}
			else
			{
				(*it).second->insert(pair<int,KeyPoint>(point.y,kp));		
			}
		}
	}
}

QRCode * FastQRFinder::FindQRCodes(Mat & img, Mat & binaryImage, vector<Drawable*> & debugVector)
{
	vector<KeyPoint> keypoints;

	struct timespec start,end;
	LOGD(LOGTAG_QRFAST,"Performing FAST detection");
	SET_TIME(&start);
	cv::FAST(img,keypoints,config->GetParameter("FastThresh"),(config->GetParameter("NonMaxSuppress") == 1.0f));
	SET_TIME(&end);
	LOG_TIME("FAST",start,end);

	LOGD(LOGTAG_QRFAST,"FAST complete. %d points found.",keypoints.size());


	LOGD(LOGTAG_QRFAST,"Sorting corners by sign.");
	SET_TIME(&start);
	map<int,map<int,KeyPoint>*> insideCornersMap_Unsorted;
	map<int,map<int,KeyPoint>*> outsideCornersMap_Unsorted;
	sortPointsByAngle(outsideCornersMap_Unsorted,insideCornersMap_Unsorted,keypoints);
	SET_TIME(&end);
	LOG_TIME("Sign sorting",start,end);


	//Extract local maximums
	LOGD(LOGTAG_QRFAST,"Filtering corners by threshold.");
	map<int,map<int,KeyPoint>*> insideCornersMap;
	map<int,map<int,KeyPoint>*> outsideCornersMap;
	SET_TIME(&start);
	sortKPMap(insideCornersMap_Unsorted,insideCornersMap,3,Size2i(2,2));
	sortKPMap(outsideCornersMap_Unsorted,outsideCornersMap,3,Size2i(2,2));
	SET_TIME(&end);
	LOG_TIME("Threshold filter",start,end);
	LOGD(LOGTAG_QRFAST,"New inside = %d, new outside = %d",insideCornersMap.size(),outsideCornersMap.size());

	//Add points to vectors
	vector<Point2i> insideCornerVector;
	vector<Point2i> outsideCornersVector;

	for (map<int,map<int,KeyPoint>*>::iterator  xIterator = insideCornersMap.begin();xIterator != insideCornersMap.end(); xIterator++)
	{
		Point2i avgPoint;
		int xCount=0, yCount=0;
		for (map<int,KeyPoint>::iterator yIterator = (*xIterator).second->begin(); yIterator != (*xIterator).second->end(); yIterator++)
		{
			Point2i point = Point2i((int)(*yIterator).second.pt.x,(int)(*yIterator).second.pt.y);
			insideCornerVector.push_back(point);
		}
	}

	for (map<int,map<int,KeyPoint>*>::iterator  xIterator = outsideCornersMap.begin();xIterator != outsideCornersMap.end(); xIterator++)
	{
		for (map<int,KeyPoint>::iterator yIterator = (*xIterator).second->begin(); yIterator != (*xIterator).second->end(); yIterator++)
		{
			Point2i point = Point2i((int)(*yIterator).second.pt.x,(int)(*yIterator).second.pt.y);
			outsideCornersVector.push_back(point);
			debugVector.push_back(new DebugCircle(point,4,Colors::Red));
		}
	}
	LOGD(LOGTAG_QRFAST,"InsideCorners=%d,OutsideCorners=%d",insideCornerVector.size(),outsideCornersVector.size());

#define FIND_PATTERNS
#ifdef FIND_PATTERNS
	//Find interesting patterns
	SET_TIME(&start);
	for (vector<Point2i>::iterator insideIt = insideCornerVector.begin(); insideIt != insideCornerVector.end(); insideIt++)
	{
		Point2i insideCornerPoint = *insideIt;
		debugVector.push_back(new DebugCircle(insideCornerPoint,4,Colors::Gold));

		multimap<int,Point2i> closestPoints;
		//getClosestByQuadrant(outsideCorners,insideCornerPoint,closestPoints);
		for (vector<Point2i>::iterator outsideIt = outsideCornersVector.begin(); outsideIt != outsideCornersVector.end(); outsideIt++)
		{
			Point2i outsideCornerPoint = *outsideIt;
			int dx = outsideCornerPoint.x - insideCornerPoint.x;
			int dy = outsideCornerPoint.y - insideCornerPoint.y;
			int dist = FastDistance(dx,dy);
			if (dist > 0)
			{
				closestPoints.insert(pair<int,Point2i>(dist,outsideCornerPoint));
			}
		}

		if (closestPoints.size() < 2)
		{
			debugVector.push_back(new DebugCircle(insideCornerPoint,12,Colors::CornflowerBlue));
			continue;
		}

		multimap<int,Point2i>::iterator closePointsIt, closePointsEnd;

		closePointsIt = closestPoints.begin();
		int closestDist = (*closePointsIt).first;
		Point2i firstPoint =  (*closePointsIt).second;
		closePointsIt++;
		closePointsEnd = closestPoints.upper_bound(closestDist * 4);

		float MaxCosine = -0.8f;
		float lowestCosine = -0.3f;

		Point2i bestPoint;

		for (; closePointsIt != closePointsEnd; closePointsIt++)
		{
			Point2i testPoint = (*closePointsIt).second;
			if (FastDistance(firstPoint,testPoint) < closestDist)
			{
			//	debugVector.push_back(new DebugLine(insideCornerPoint,testPoint,Colors::Blue));
				continue;
			}
			debugVector.push_back(new DebugLine(insideCornerPoint,testPoint,Colors::PeachPuff,1));

			float cosine = FastTracking::angle(firstPoint,testPoint,insideCornerPoint);
			if (cosine < lowestCosine)
			{
				lowestCosine = cosine;
				bestPoint = testPoint;
			}
		}

		//Validate slope
		if (bestPoint.x == 0 && bestPoint.y==0)
		{			
			//debugVector.push_back(new DebugLine(insideCornerPoint,firstPoint,Colors::Blue));
			continue;
		}
		else if (lowestCosine >= MaxCosine)
		{
			/*char str[150];
			sprintf(str,"Lowest Cosine=%f",lowestCosine);
			debugVector.push_back(new DebugLabel(insideCornerPoint,str,Colors::Black,1.2f));*/
		//	debugVector.push_back(new DebugLine(insideCornerPoint,bestPoint,Colors::Aqua));
			continue;
		}

		debugVector.push_back(new DebugLine(insideCornerPoint,firstPoint,Colors::Lime,2));
		debugVector.push_back(new DebugLine(insideCornerPoint,bestPoint,Colors::Lime,2));

	}
	SET_TIME(&end);
	LOG_TIME("Classifying points",start,end);
#endif

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
	
	//Cleanup maps
	LOGV(LOGTAG_QRFAST,"Cleaning up maps");
	for (map<int,map<int,KeyPoint>*>::iterator deleteIt = insideCornersMap.begin(); deleteIt != insideCornersMap.end();deleteIt++)
	{
		delete (*deleteIt).second;
	}

	for (map<int,map<int,KeyPoint>*>::iterator deleteIt = outsideCornersMap.begin(); deleteIt != outsideCornersMap.end();deleteIt++)
	{
		delete (*deleteIt).second;
	}
	
	for (map<int,map<int,KeyPoint>*>::iterator deleteIt = insideCornersMap_Unsorted.begin(); deleteIt != insideCornersMap_Unsorted.end();deleteIt++)
	{
		delete (*deleteIt).second;
	}

	for (map<int,map<int,KeyPoint>*>::iterator deleteIt = outsideCornersMap_Unsorted.begin(); deleteIt != outsideCornersMap_Unsorted.end();deleteIt++)
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