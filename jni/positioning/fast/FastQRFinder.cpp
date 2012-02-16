#include "FastQRFinder.hpp"

FastQRFinder::FastQRFinder(ARControllerDebugUI * debugUI)
{
	config = debugUI;
	config->AddNewParameter("FlannRadius",25,1,3,300,"%3.0f",2);

	
	config->AddNewParameter("UseRadius",0,1,0,1,"%1.0f",2);
	config->AddNewParameter("FlannIndexType",0,1,0,2,"%1.0f",2);

	config->AddNewParameter("K-NN",6,1,1,12,"%2.0f",2);
	config->AddNewParameter("NumKDTrees",1,1,1,16,"%2.0f",2);
	config->AddNewParameter("FlannSearchParams",32,32,32,64,"%2.0f",2);
		
	config->AddNewParameter("FASTDebug",2,1,-2,4,"%2.0f",2);
	
	config->AddNewParameter("ScoreMaxDimension",2,1,0,25,"%2.0f",2);
	config->AddNewParameter("ScoreMaxIterations",2,1,0,25,"%2.0f",2);

//	config->AddNewParameter("DistanceMode",1,1,0,1,"%1.0f",1);
	srand(time(NULL));

	flannTime = 2000;

	patternTimes[0] = 1000;
	patternTimes[1] = 1000;
	patternTimes[2] = 1000;
	patternTimes[3] = 1000;

	pointTime =0;


	config->AddNewLabel("FlannTime","[]", 1);
	config->AddNewLabel("SortPointsTime","[]", 1);
	config->AddNewLabel("CheckSlopeTime","[]", 1);
	config->AddNewLabel("CheckPointsTime","[]", 1);
	config->AddNewLabel("PerPointTime","[]", 1);
	config->AddNewLabel("TotalPatternTime","[]", 1);
	config->AddNewLabel("NumPoints","[]", 1);
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

#define AbsoluteMacro(x) (x >= 0) ? x : x


//Faster? Less accurate for certain.


int FastQRFinder::GetDistanceFast(int dx, int dy)
{
	dx = AbsoluteMacro(dx);
	dy = AbsoluteMacro(dy);
	int mn = MIN(dx,dy);
	return(dx+dy-(mn>>1)-(mn>>2)+(mn>>4));
} 

//Faster? Less accurate for certain.
int FastQRFinder::GetDistanceFast(Point2i pt0, Point2i pt1)
{
	int dx = (pt0.x-pt1.x);
	int dy = (pt0.y-pt1.y);

	dx = AbsoluteMacro(dx);
	dy = AbsoluteMacro(dy);

	int mn = MIN(dx,dy);
	return(dx+dy-(mn>>1)-(mn>>2)+(mn>>4));
} 

int FastQRFinder::GetSquaredDistance(int dx, int dy)
{
	return ipow(dx,2) + ipow(dy,2);
} 


int FastQRFinder::GetSquaredDistance(Point2i pt0, Point2i pt1)
{
	return ipow(pt0.x-pt1.x,2) + ipow(pt0.y-pt1.y,2);
} 

//static bool isQuadrantEmpty(Point2i start, Point2i middle, Point2i end, 
//	map<int,map<int,Point2i>*> & xSorted, map<int,map<int,Point2i>*> & ySorted)
//	/*map<int,Point2i>::iterator & xStart,
//								map<int,Point2i>::iterator & xEnd,
//								map<int,Point2i>::iterator & yStart, 
//								map<int,Point2i>::iterator & yEnd)*/
//{
//	map<int,Point2i>::iterator xStart = 
//	map<int,Point2i>::iterator xEnd;
//	map<int,Point2i>::iterator yStart; 
//	map<int,Point2i>::iterator yEnd;
//
//
//	if (xStart != xEnd && yStart != yEnd)
//	{
//		int xStartVal = (*xStart).second.x;
//		int xEndVal = (*(--xEnd)).second.x);
//		int xMiddle = xStartVal + (xEndVal-xStartVal)/2;
//
//		int yStartVal = (*yStart).second.y;
//		int yEndVal = (*(--yEnd)).second.y);
//		int yMiddle = yStartVal + (yEndVal-yStartVal)/2;
//
//
//
//		return;		
//	}
//
//	return false;
//}
//
//static void RecursiveQuadrantSearch(map<int,map<int,Point2i>*> & xSorted, map<int,map<int,Point2i>*> & ySorted)
//{
//
//}

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

//static bool searchSetForPoint(Point2i pt, map<int,set<int> > & searchSetMap)
//{
//	map<int,set<int> >::iterator searchIterator;
//	searchIterator = searchSetMap.find( pt.x);
//	if (searchIterator != searchSetMap.end())
//	{
//		if ((*searchIterator).second.count(pt.y) != 0)
//		{
//			return true;
//		}
//	}
//	return false;
//}
//
//static void getClosestByQuadrant(map<int,map<int,Point2i>*> & points, Point2i start, multimap<int,Point2i> & closest)
//{
//
//	for (int i=0;i<4;i++)
//	{
//		//0 = (1,1)
//		//1 = (1,-1)
//		//2 = (-1,1)
//		//3 = (-1,-1)
//
//		multimap<int,Point2i> closestPoints;
//
//		if (i == 2 || i == 3)
//		{
//			map<int,map<int,Point2i>*>::reverse_iterator xIterator(points.lower_bound(start.x));
//			for (; xIterator != points.rend();++xIterator)
//			{
//				if (i == 2)
//				{
//					map<int,Point2i>::iterator yIterator = (*xIterator).second->lower_bound(start.y);
//					for (; yIterator != (*xIterator).second->end(); yIterator++)
//					{
//						Point2i testPoint = (*yIterator).second;
//						int dist = GetSquaredDistance(start,testPoint);
//						if (dist > 0)
//						{
//							closestPoints.insert(pair<int,Point2i>(dist,testPoint));
//						}
//					}
//				}
//				else
//				{					
//					map<int,Point2i>::reverse_iterator yIterator((*xIterator).second->lower_bound(start.y));
//					for (; yIterator != (*xIterator).second->rend();++yIterator)
//					{
//						Point2i testPoint = (*yIterator).second;
//						int dist = GetSquaredDistance(start,testPoint);
//						if (dist > 0)
//						{
//							closestPoints.insert(pair<int,Point2i>(dist,testPoint));
//						}
//					}
//				}
//			}
//		} 
//		else 
//		{			
//
//			map<int,map<int,Point2i>*>::iterator xIterator = points.lower_bound(start.x);
//			for (; xIterator != points.end();xIterator++)
//			{				
//				if (i == 0)
//				{
//					map<int,Point2i>::iterator yIterator = (*xIterator).second->lower_bound(start.y);
//					for (; yIterator != (*xIterator).second->end(); yIterator++)
//					{
//						Point2i testPoint = (*yIterator).second;
//						int dist = GetSquaredDistance(start,testPoint);
//						if (dist > 0)
//						{
//							closestPoints.insert(pair<int,Point2i>(dist,testPoint));
//						}
//					}
//				}
//				else
//				{					
//					map<int,Point2i>::reverse_iterator yIterator = map<int,Point2i>::reverse_iterator((*xIterator).second->lower_bound(start.y));
//					for (; yIterator != (*xIterator).second->rend();++yIterator)
//					{
//						Point2i testPoint = (*yIterator).second;
//						int dist = GetSquaredDistance(start,testPoint);
//						if (dist > 0)
//						{
//							closestPoints.insert(pair<int,Point2i>(dist,testPoint));
//						}
//					}
//				}
//			}
//		}
//		closest.insert(pair<int,Point2i>(*closestPoints.begin()));
//	}
//
//
//}

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
	struct timespec startTotal,endTotal;
	SET_TIME(&startTotal);
	int debugLevel = (int)config->GetParameter("FASTDebug");

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

	int scoreSearchSize = config->GetParameter("ScoreMaxDimension");
	int searchIterations = config->GetParameter("ScoreMaxIterations");

	map<int,map<int,KeyPoint>*> outsideCornersMap;
	SET_TIME(&start);
	sortKPMap(insideCornersMap_Unsorted,insideCornersMap,searchIterations,Size2i(scoreSearchSize,scoreSearchSize));
	sortKPMap(outsideCornersMap_Unsorted,outsideCornersMap,searchIterations,Size2i(scoreSearchSize,scoreSearchSize));
	SET_TIME(&end);
	LOG_TIME("Threshold filter",start,end);
	LOGD(LOGTAG_QRFAST,"New inside = %d, new outside = %d",insideCornersMap.size(),outsideCornersMap.size());

	//Add points to vectors
	vector<Point2i> insideCornerVector;
	vector<Point2i> outsideCornersVector;
	

	SET_TIME(&start);
	for (map<int,map<int,KeyPoint>*>::iterator  xIterator = insideCornersMap.begin();xIterator != insideCornersMap.end(); xIterator++)
	{	
		for (map<int,KeyPoint>::iterator yIterator = (*xIterator).second->begin(); yIterator != (*xIterator).second->end(); yIterator++)
		{
			Point2i point = Point2i((int)(*yIterator).second.pt.x,(int)(*yIterator).second.pt.y);
			insideCornerVector.push_back(point);
		}
	}

	LOGV(LOGTAG_QRFAST,"Added inside corners, now adding outside");

	for (map<int,map<int,KeyPoint>*>::iterator  xIterator = outsideCornersMap.begin();xIterator != outsideCornersMap.end(); xIterator++)
	{
		for (map<int,KeyPoint>::iterator yIterator = (*xIterator).second->begin(); yIterator != (*xIterator).second->end(); yIterator++)
		{
			KeyPoint kp = (*yIterator).second;
			Point2i point = Point2i((int)kp.pt.x,(int)kp.pt.y);
			outsideCornersVector.push_back(point);

			if (debugLevel > 2)
				debugVector.push_back(new DebugCircle(point,4,Colors::Red));
		}
	}

	
	
	int numFeatures = outsideCornersVector.size();

	//Nothing to track!
	if (numFeatures < 2 || insideCornerVector.size() < 2)
	{
		return NULL;
	}


	Mat outsideCornerMat = Mat(numFeatures,2,CV_32F);// m_object.convertTo(obj_32f,CV_32FC2);
	for (int i = 0;i < outsideCornersVector.size();i++)
	{
		outsideCornerMat.at<float>(i,0) = (float)outsideCornersVector.at(i).x;
		outsideCornerMat.at<float>(i,1) = (float)outsideCornersVector.at(i).y;
	}


	SET_TIME(&end);
	LOG_TIME("Vector conversion",start,end);
	LOGD(LOGTAG_QRFAST,"InsideCorners=%d,OutsideCorners=%d,OCM=%d",insideCornerVector.size(),outsideCornersVector.size(),outsideCornerMat.size().area());

	bool squareDistance = true;// config->GetParameter("DistanceMode")==1.0f;

	int numTrees = (int)config->GetParameter("NumKDTrees");

	SET_TIME(&start);
	int flannIndexType = (int)config->GetParameter("FlannIndexType");

	cv::flann::Index * flannPointIndex;
	if (flannIndexType == 0)
	{		
		LOGD(LOGTAG_QRFAST,"Creating linear index");
		flannPointIndex = new flann::Index(outsideCornerMat, cv::flann::LinearIndexParams());
	}
	else if (flannIndexType == 1)
	{
		LOGD(LOGTAG_QRFAST,"Creating KDTree Index with %d trees",numTrees);
		flannPointIndex = new flann::Index(outsideCornerMat,cv::flann::KDTreeIndexParams(numTrees));  
	}
	else if (flannIndexType == 2)
	{		
		LOGD(LOGTAG_QRFAST,"Creating KMeans index");
		flannPointIndex = new flann::Index(outsideCornerMat,cv::flann::KMeansIndexParams());  
	}
	else
	{
		LOGE("No index type defined!");
		throw exception();
	}
	//else 
	//{		
	//	LOGD(LOGTAG_QRFAST,"Creating Autotuned index");
	//	flannPointIndex = new flann::Index(outsideCornerMat,cv::flann::AutotunedIndexParams(0.7f,0.01f,0.0f,0.5f));  
	//}
	//

	SET_TIME(&end);
	LOG_TIME_PRECISE("Building FLANN index",start,end);
	

	double flannTime_local = 0, analyzeTime_local;
	
	int searchSize = (int)(config->GetParameter("K-NN"));
	double flannRadius = std::pow(config->GetParameter("FlannRadius"),2);
	int searchParams = (int)config->GetParameter("FlannSearchParams");

	bool useRadius = (config->GetParameter("UseRadius") == 1.0f);
	
	double patternTime_local[4] = {0,0,0,0};

	struct timespec pointStart,pointEnd;
	double pointTime_local =0;
	//Find interesting patterns
	
	Mat indexMatrix = Mat::ones(1, searchSize, CV_32S) * -1;
	Mat distanceMatrix = Mat::zeros(1, searchSize, CV_32F);
	
	vector<pair<int,Point2i> > closePointsVector(searchSize);
	Mat objectPoints = Mat(1,2,CV_32F);
	for (vector<Point2i>::iterator insideIt = insideCornerVector.begin(); insideIt != insideCornerVector.end(); insideIt++)
	{
		SET_TIME(&pointStart);
		Point2i insideCornerPoint = *insideIt;
		if (debugLevel > 2)
			debugVector.push_back(new DebugCircle(insideCornerPoint,4,Colors::Gold));

		objectPoints.at<float>(0,0) = (float)insideCornerPoint.x;
		objectPoints.at<float>(0,1) = (float)insideCornerPoint.y;

		struct timespec innerStart,innerEnd;
		SET_TIME(&innerStart);

		int result = searchSize;
		
		if (useRadius)
		{
			int radiusSearch = flannPointIndex->radiusSearch(objectPoints, indexMatrix, distanceMatrix,flannRadius, searchSize , cv::flann::SearchParams(searchParams)); 
			if (radiusSearch < searchSize)
				result = radiusSearch;
		}
		else
		{
			flannPointIndex->knnSearch(objectPoints, indexMatrix, distanceMatrix, searchSize, cv::flann::SearchParams(searchParams));
		}
		//LOGV(LOGTAG_QRFAST,"RadiusMode=%d,Pt(%d,%d) - FLANN(%d < %lf) complete(%d).",useRadius,insideCornerPoint.x,insideCornerPoint.y,searchSize,flannRadius,result);
		

		SET_TIME(&innerEnd);
		flannTime_local += calc_time_double(innerStart,innerEnd);

	//	LOGD_Mat(LOGTAG_QRFAST,"KNNIndex",&indexMatrix);

		if (result != searchSize)
			closePointsVector.resize(result);
		SET_TIME(&innerStart);
		/*clock_gettime(CLOCK_MONOTONIC_HR, &innerStart);*/
		multimap<int,Point2i> closestPoints;
		
		int * indexPtr = indexMatrix.ptr<int>(0);
		float * distPtr = distanceMatrix.ptr<float>(0);
		float * rowPtr;
		for (int i=0;i < result;i++)
		{
			rowPtr = outsideCornerMat.ptr<float>(indexPtr[i]);
			closePointsVector.push_back(pair<int,Point2i>(distPtr[i],Point2i((int)(rowPtr[0]),(int)(rowPtr[1]))));
		}
		SET_TIME(&innerEnd);
		/*clock_gettime(CLOCK_MONOTONIC_HR , &innerEnd);
		LOGD(LOGTAG_QRFAST,"VectoringResult=%ld",innerEnd.tv_nsec-innerStart.tv_nsec);*/
		//LOG_TIME_PRECISE("Result vectorizing",innerStart,innerEnd);
		patternTime_local[0] += calc_time_double(innerStart,innerEnd);
		
		if (closePointsVector.size() < 2)
		{
			if (debugLevel > 0)
			{
				debugVector.push_back(new DebugCircle(insideCornerPoint,12,Colors::CornflowerBlue));
				if (debugLevel > 1 && closePointsVector.size() == 1)
				{
					debugVector.push_back(new DebugLine(insideCornerPoint,(*closePointsVector.begin()).second,Colors::CornflowerBlue));
				}
			}
			continue;
		}

		/*	multimap<int,Point2i>::iterator closePointsIt, closePointsEnd;*/
		vector<pair<int,Point2i> >::iterator closePointsIt, closePointsEnd;

		closePointsIt = closePointsVector.begin();
		int closestDist = (*closePointsIt).first;
		Point2i firstPoint =  (*closePointsIt).second;
		closePointsIt++;
		closePointsEnd = closePointsVector.end();//closestPoints.upper_bound(closestDist * 4);

		float MaxCosine = -0.8f;
		float lowestCosine = -0.3f;

		Point2i bestPoint;
		
		SET_TIME(&innerStart);
		for (; closePointsIt != closePointsEnd; closePointsIt++)
		{			
		/*	if ((*closePointsIt).first > closestDist * 4)
				break;
			
*/
			//LOGV(LOGTAG_QRFAST,"Vector: %d->(%d,%d)",(*closePointsIt).first,(*closePointsIt).second.x,(*closePointsIt).second.y);
			Point2i testPoint = (*closePointsIt).second;
			if (GetSquaredDistance(firstPoint,testPoint) < closestDist)
			{
				
				if (debugLevel > 1)
					debugVector.push_back(new DebugLine(insideCornerPoint,testPoint,Colors::Blue));
				continue;
			}
			if (debugLevel > 0)
				debugVector.push_back(new DebugLine(insideCornerPoint,testPoint,Colors::PeachPuff,1));

			float cosine = FastTracking::angle(firstPoint,testPoint,insideCornerPoint);
			if (cosine < lowestCosine)
			{
				lowestCosine = cosine;
				bestPoint = testPoint;
			}
		}
		SET_TIME(&innerEnd);
		patternTime_local[1] += calc_time_double(innerStart,innerEnd);

		
		SET_TIME(&innerStart);
		//Validate slope
		if (bestPoint.x == 0 && bestPoint.y==0)
		{		
			if (debugLevel > 2)
			{
				debugVector.push_back(new DebugLine(insideCornerPoint,firstPoint,Colors::Blue));
			}
			continue;
		}
		else if (lowestCosine >= MaxCosine)
		{
			if (debugLevel > 1)
			{
				if (debugLevel > 3)
				{
					char str[150];
					sprintf(str,"Lowest Cosine=%f",lowestCosine);
					debugVector.push_back(new DebugLabel(insideCornerPoint,str,Colors::Black,1.2f));
				}
				debugVector.push_back(new DebugLine(insideCornerPoint,bestPoint,Colors::Aqua));
			}
			continue;
		}		
		SET_TIME(&innerEnd);
		patternTime_local[2] += calc_time_double(innerStart,innerEnd);

		if (debugLevel > -1)
		{
			debugVector.push_back(new DebugLine(insideCornerPoint,firstPoint,Colors::Lime,2));
			debugVector.push_back(new DebugLine(insideCornerPoint,bestPoint,Colors::Lime,2));
		}
		
		closePointsVector.clear();
		indexMatrix = Mat::ones(1, searchSize, CV_32S) * -1;

		SET_TIME(&pointEnd);
		pointTime_local += calc_time_double(pointStart,pointEnd);
	}
	patternTime_local[3] = pointTime_local;


	//flannTime = (flannTime  + (flannTime_local))/2.0;

	config->SetLabelValue("FlannTime",flannTime_local/1000.0);
	for (int i=0;i<4;i++)
	{
		patternTimes[i] = (patternTimes[i] + patternTime_local[i])/2.0;
	}
	config->SetLabelValue("SortPointsTime",patternTimes[0]);
	config->SetLabelValue("CheckPointsTime",patternTimes[1]);
	config->SetLabelValue("CheckSlopeTime",patternTimes[2]);

	pointTime_local /= (double)insideCornerVector.size();
	pointTime = (pointTime + pointTime_local)/2.0;
	config->SetLabelValue("PerPointTime",pointTime);
	config->SetLabelValue("TotalPatternTime",patternTimes[3]/1000.0);
	config->SetLabelValue("NumPoints",(float)insideCornerVector.size());

	struct timespec resolution, currentTime;
	clock_getres(CLOCK_MONOTONIC_HR,&resolution);
	clock_getres(CLOCK_MONOTONIC_HR,&currentTime);
	LOGD(LOGTAG_QRFAST,"MonoHRRes=%ld,%ld",resolution.tv_sec,resolution.tv_nsec);
	LOGD(LOGTAG_QRFAST,"MonoHRTime=%ld,%ld",currentTime.tv_sec,currentTime.tv_nsec);
	clock_getres(CLOCK_REALTIME_HR,&resolution);
	clock_getres(CLOCK_REALTIME_HR,&currentTime);
	LOGD(LOGTAG_QRFAST,"RealHRRes=%ld,%ld",resolution.tv_sec,resolution.tv_nsec);
	LOGD(LOGTAG_QRFAST,"RealHRTime=%ld,%ld",currentTime.tv_sec,currentTime.tv_nsec);
	clock_getres(CLOCK_MONOTONIC,&resolution);
	clock_getres(CLOCK_MONOTONIC,&currentTime);
	LOGD(LOGTAG_QRFAST,"MonoRes=%ld,%ld",resolution.tv_sec,resolution.tv_nsec);
	LOGD(LOGTAG_QRFAST,"MonoTime=%ld,%ld",currentTime.tv_sec,currentTime.tv_nsec);
	clock_getres(CLOCK_REALTIME,&resolution);
	clock_getres(CLOCK_REALTIME,&currentTime);
	LOGD(LOGTAG_QRFAST,"RealRes=%ld,%ld",resolution.tv_sec,resolution.tv_nsec);
	LOGD(LOGTAG_QRFAST,"RealTime=%ld,%ld",currentTime.tv_sec,currentTime.tv_nsec);
	clock_getres(CLOCK_THREAD_CPUTIME_ID,&resolution);
	clock_getres(CLOCK_THREAD_CPUTIME_ID,&currentTime);
	LOGD(LOGTAG_QRFAST,"ThreadRes=%ld,%ld",resolution.tv_sec,resolution.tv_nsec);
	LOGD(LOGTAG_QRFAST,"ThreadTime=%ld,%ld",currentTime.tv_sec,currentTime.tv_nsec);
	clock_getres(CLOCK_PROCESS_CPUTIME_ID,&resolution);
	clock_getres(CLOCK_PROCESS_CPUTIME_ID,&currentTime);
	LOGD(LOGTAG_QRFAST,"ProcessRes=%ld,%ld",resolution.tv_sec,resolution.tv_nsec);
	LOGD(LOGTAG_QRFAST,"ProcessTime=%ld,%ld",currentTime.tv_sec,currentTime.tv_nsec);
	clock_getres(CLOCK_SGI_CYCLE,&resolution);
	clock_getres(CLOCK_SGI_CYCLE,&currentTime);
	LOGD(LOGTAG_QRFAST,"SGIRes=%ld,%ld",resolution.tv_sec,resolution.tv_nsec);
	LOGD(LOGTAG_QRFAST,"SGITime=%ld,%ld",currentTime.tv_sec,currentTime.tv_nsec);
	
	
	
	
	delete flannPointIndex;
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
	
	SET_TIME(&endTotal);
	LOG_TIME("TotalFASTQR",startTotal,endTotal);
	return NULL;
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

