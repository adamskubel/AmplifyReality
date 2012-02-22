#include "FastQRFinder.hpp"

class PointCompare
{
	public:
      PointCompare(Point2i _center)
	  {
		  center = _center;
	  }
      bool operator()(const Point2i pt0, const Point2i pt1)
	  {
		  return (ipow(pt0.x-center.x,2) + ipow(pt0.y-center.y,2)) > (ipow(pt1.x-center.x,2) + ipow(pt1.y-center.y,2)) ;
	  }
   private:
      Point2i center;
};

FastQRFinder::FastQRFinder(ARControllerDebugUI * debugUI)
{
	config = debugUI;

	
	config->AddNewParameter("MinClusterSize",2,1,1,20,"%1.0f",2);
	config->AddNewParameter("ClusterK-NN",4,1,1,20,"%1.0f",2);
	config->AddNewParameter("ClusterRadius",25,1,3,300,"%3.0f",2);

	config->AddNewParameter("RadiusSearch","DoRadiusSearch",0,1,0,1,"%1.0f",2);
	config->AddNewParameter("FLANNRadius",25,1,3,300,"%3.0f",2);
	config->AddNewParameter("FlannIndexType",1,1,0,2,"%1.0f",2);


	config->AddNewParameter("K Nearest","K-NN",6,1,1,12,"%2.0f",2);
	config->AddNewParameter("NumKDTrees",1,1,1,16,"%2.0f",2);
	config->AddNewParameter("FlannSearchParams",32,32,32,64,"%2.0f",2);
		
	config->AddNewParameter("FAST Debug Level","FASTDebug",0,1,-2,4,"%2.0f",2);
	
	config->AddNewParameter("Post-NonMaxSupress Size","MaxThreshSize",2,1,0,25,"%2.0f",2);
	config->AddNewParameter("Post-NonMaxSupress Count","MaxThreshCount",2,1,0,25,"%2.0f",2);
			
	config->AddNewParameter("FAST Threshold","FastThresh",10,5,1,400,"%3.0f",2);
	config->AddNewParameter("NonMaxSuppress",1,1,0,1,"%1.0f",2);

	flannTime = 10;
	pointTime = 0;
	maxThreshTime = 20;
	fastTime = 0;
	clusterTime = 0;
	
	
	config->AddNewLabel("FAST time"," ms ", 1);
	config->AddNewLabel("FlannTime"," ms ", 1);
	config->AddNewLabel("TotalTime"," ms ", 1);
	config->AddNewLabel("AvgPointTime"," us ", 1);
	config->AddNewLabel("NumPoints","", 1);
	config->AddNewLabel("MaxPt Time"," ms ", 1);
	config->AddNewLabel("Cluster Time"," ms ", 1);
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

		//LOGV(LOGTAG_QRFAST,"Iteration %d complete. Rows: (%d) -> (%d)",i,keyPointMap.size(),sortedMap.size());

		//Only want to delete intermediate maps
		if (i > 0)
		{
			//LOGV(LOGTAG_QRFAST,"Cleaning up map");
			for (map<int,map<int,KeyPoint>*>::iterator deleteIt = keyPointMap.begin(); deleteIt != keyPointMap.end();deleteIt++)
			{
				delete (*deleteIt).second;
			}
		}

		keyPointMap = sortedMap;
		sortedMap = map<int,map<int,KeyPoint>*>();
	}
	sortedMap_Out = keyPointMap;
	//LOGD(LOGTAG_QRFAST,"Sorting complete.");
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

static void addExclusionZone(Point2i exclusionPoint, int exclusionRadius, flann::Index *& searchExclusionIndex, vector<pair<int,Point2i> > & exclusionDistances)
{	
	//int newPointIndex = exclusionDistances.size();
	exclusionDistances.push_back(pair<int,Point2i>(exclusionRadius,exclusionPoint));

	Mat exclusionDistanceMatrix = Mat(Size2i(2,exclusionDistances.size()), CV_32F);
	for (int i=0;i<exclusionDistances.size();i++)
	{
		exclusionDistanceMatrix.at<float>(i,0) = exclusionDistances[i].second.x;
		exclusionDistanceMatrix.at<float>(i,1) = exclusionDistances[i].second.y;
	}

	if (searchExclusionIndex != NULL)
	{
		//LOGV(LOGTAG_QRFAST,"Deleting old search index");
		delete searchExclusionIndex;
		//LOGV(LOGTAG_QRFAST,"Deletion complete.");
	}
	
	//LOGV(LOGTAG_QRFAST,"Creating new search index for %d points, vector.size=%d",exclusionDistanceMatrix.rows,exclusionDistances.size());
	searchExclusionIndex = new flann::Index(exclusionDistanceMatrix,flann::KDTreeIndexParams(2));
}

static bool checkExclusionZone(Mat searchPoint, flann::Index *& searchExclusionIndex, vector<pair<int,Point2i> > & exclusionDistances, Mat & indexMatrix, Mat & distanceMatrix, Point2i & matchingPoint)
{	
	if (searchExclusionIndex == NULL)
		return false;
	//LOGV(LOGTAG_QRFAST,"Searching exclusion zone for point(%f,%f)",searchPoint.at<float>(0,0),searchPoint.at<float>(0,1));
	searchExclusionIndex->knnSearch(searchPoint,indexMatrix,distanceMatrix,1,flann::SearchParams(32));

	int * indexPtr = indexMatrix.ptr<int>(0);
	float * distPtr = distanceMatrix.ptr<float>(0);
	for (int i=0;i < 1;i++)
	{
		//LOGV(LOGTAG_QRFAST,"Exclusion index =%d",indexPtr[i]);
		if (indexPtr[i] < 0 || indexPtr[i] >= exclusionDistances.size())
			break;

		int minDistance = exclusionDistances.at(indexPtr[i]).first;
		int actualDistance = (int)distPtr[i];
		if (actualDistance < minDistance)
		{
			matchingPoint = exclusionDistances.at(indexPtr[i]).second;
			return true;
		}
	}
	//LOGV(LOGTAG_QRFAST,"Point is not in any exclusion zone");
	return false;
}

static void FastWindow(Mat & inputImg, vector<KeyPoint> & features, Rect window, int threshold, bool nonMax)
{	
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
		cv::FAST(tMat,features,threshold,nonMax);
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
}


void FastQRFinder::EnhanceQRCodes(Mat & img, QRCode * code, vector<Drawable*> & debugVector)
{
	int debugLevel = (int)config->GetParameter("FASTDebug");
	for (int i=0;i<code->finderPatterns.size();i++)
	{
		FinderPattern * fp = code->finderPatterns.at(i);
		if (fp->size == 0)
			continue;
		
		float windowSize = fp->size * 1.4f;  //Window needs to be bigger to account for diagonals
	
		Rect window = Rect(fp->pt.x - windowSize/2.0f, fp->pt.y-windowSize/2.0f, windowSize,windowSize);
		if (debugLevel > 0)
			debugVector.push_back(new DebugRectangle(window,Colors::Aqua));

		LocateFPCorners(img, window,debugVector);
	}
}

void FastQRFinder::LocateFPCorners(Mat & img, Rect roi, vector<Drawable*> & debugVector)
{
	struct timespec startTotal,endTotal;
	SET_TIME(&startTotal);

	//Get parameters from UI. Need to store as local variables because they are accessed many times per frame.
	int debugLevel = (int)config->GetParameter("FASTDebug");
	int searchSize = config->GetIntegerParameter("K-NN");
	double flannRadius = std::pow(config->GetParameter("FLANNRadius"),2.0);
	int searchParams = config->GetIntegerParameter("FlannSearchParams");
	bool useRadius = config->GetBooleanParameter("DoRadiusSearch");
	int flannIndexType = config->GetIntegerParameter("FlannIndexType");
	int numTrees = config->GetIntegerParameter("NumKDTrees");
	int threshold = config->GetParameter("FastThresh");
	bool nonMaxSuppress = config->GetBooleanParameter("NonMaxSuppress");

	vector<KeyPoint> keypoints;

	struct timespec start,end;
	LOGD(LOGTAG_QRFAST,"Performing FAST detection");
	SET_TIME(&start);
	FastWindow(img,keypoints,roi, threshold,nonMaxSuppress);
	SET_TIME(&end);
	double fastTime_local = calc_time_double(start,end);
	fastTime = (fastTime + fastTime_local)/2.0;
	config->SetLabelValue("FAST time",(float)fastTime/1000.0f);

	LOGD(LOGTAG_QRFAST,"FAST complete. %d points found.",keypoints.size());

	
	
	vector<Point2i> insideCornerVector, outsideCornersVector;
	Point2i centroid(0,0);

	//If didn't do nonmax during FAST, perform here using maps
	//Otherwise, just add the points to vectors
	if (!nonMaxSuppress)
	{
		SET_TIME(&start);
		map<int,map<int,KeyPoint>*> insideCornersMap_Unsorted;
		map<int,map<int,KeyPoint>*> outsideCornersMap_Unsorted;
		sortPointsByAngle(outsideCornersMap_Unsorted,insideCornersMap_Unsorted,keypoints);
		SET_TIME(&end);
		LOG_TIME_PRECISE("Sign sorting",start,end);

		//Extract local maximums	
		LOGD(LOGTAG_QRFAST,"Filtering corners by threshold.");
		map<int,map<int,KeyPoint>*> insideCornersMap;

		int scoreSearchSize = config->GetParameter("MaxThreshSize");
		int searchIterations = config->GetParameter("MaxThreshCount");

		map<int,map<int,KeyPoint>*> outsideCornersMap;
		SET_TIME(&start);
		sortKPMap(insideCornersMap_Unsorted,insideCornersMap,searchIterations,Size2i(scoreSearchSize,scoreSearchSize));
		sortKPMap(outsideCornersMap_Unsorted,outsideCornersMap,searchIterations,Size2i(scoreSearchSize,scoreSearchSize));
		SET_TIME(&end);
		double maxThreshTime_local = calc_time_double(start,end);
		maxThreshTime = (maxThreshTime + maxThreshTime_local)/2.0;
		config->SetLabelValue("MaxPt Time", (float)maxThreshTime/1000.0f);

		//Add points to vectors
		for (map<int,map<int,KeyPoint>*>::iterator  xIterator = insideCornersMap.begin();xIterator != insideCornersMap.end(); xIterator++)
		{	
			for (map<int,KeyPoint>::iterator yIterator = (*xIterator).second->begin(); yIterator != (*xIterator).second->end(); yIterator++)
			{
				Point2i point = Point2i((int)(*yIterator).second.pt.x,(int)(*yIterator).second.pt.y);
				insideCornerVector.push_back(point);
				centroid += point;
			}
		}
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

		//Cleanup maps
		LOGV(LOGTAG_QRFAST,"Cleaning up maps");
		for (map<int,map<int,KeyPoint>*>::iterator deleteIt = insideCornersMap.begin(); deleteIt != insideCornersMap.end();deleteIt++) delete (*deleteIt).second;
		for (map<int,map<int,KeyPoint>*>::iterator deleteIt = outsideCornersMap.begin(); deleteIt != outsideCornersMap.end();deleteIt++) delete (*deleteIt).second;
		for (map<int,map<int,KeyPoint>*>::iterator deleteIt = insideCornersMap_Unsorted.begin(); deleteIt != insideCornersMap_Unsorted.end();deleteIt++) delete (*deleteIt).second;
		for (map<int,map<int,KeyPoint>*>::iterator deleteIt = outsideCornersMap_Unsorted.begin(); deleteIt != outsideCornersMap_Unsorted.end();deleteIt++) delete (*deleteIt).second;
	}
	else
	{	
#define DO_CLUSTERING false
#if DO_CLUSTERING
			
		double clusterRadius = std::pow(config->GetParameter("ClusterRadius"),2.0);
		int minClusterCount = config->GetIntegerParameter("MinClusterSize");
		int maxClusterCount = config->GetIntegerParameter("ClusterK-NN");

		vector<FastQR::Node*> vecNodes;
		Mat kpMat = Mat(keypoints.size(),2,CV_32F);	
		int clusterPointMatCount = 0;
		for (vector<KeyPoint>::iterator kpIt = keypoints.begin(); kpIt != keypoints.end(); kpIt++, clusterPointMatCount++)
		{
			kpMat.at<float>(clusterPointMatCount,0) = (*kpIt).pt.x;
			kpMat.at<float>(clusterPointMatCount,1) = (*kpIt).pt.y;
			vecNodes.push_back(new FastQR::Node(*kpIt));
		}

		LOGD(LOGTAG_QRFAST,"Added %d nodes to vector, doing clustering",vecNodes.size());

		flann::Index * kdIndex = new flann::Index(kpMat,flann::KDTreeIndexParams(1));

		SET_TIME(&start);
		int numClusters = RunDBScan(vecNodes,kdIndex,clusterRadius,minClusterCount, maxClusterCount);	
		SET_TIME(&end);
		double clusterTime_local = calc_time_double(start,end);
		clusterTime = (clusterTime + clusterTime_local)/2.0;
		config->SetLabelValue("Cluster Time",(float)clusterTime);
		
		if (kdIndex != NULL)
			delete kdIndex;

		LOGD(LOGTAG_QRFAST,"Found %d clusters. VecNodeSize=%d",numClusters,vecNodes.size());
		Scalar clusterColors[] = {Colors::Fuchsia, Colors::DeepSkyBlue, Colors::Yellow, Colors::Green, Colors::PowderBlue};
		for (vector<FastQR::Node*>::iterator kpIt = vecNodes.begin(); kpIt != vecNodes.end(); kpIt++)
		{
			Point2i point = Point2i((*kpIt)->nodePoint.at<float>(0,0),(*kpIt)->nodePoint.at<float>(0,1));
			int clusterIndex = (*kpIt)->clusterIndex;
			if (clusterIndex >= 0)
				debugVector.push_back(new DebugCircle(point,15,clusterColors[clusterIndex % 5],2));
			else
				debugVector.push_back(new DebugCircle(point,15,Colors::Gray,2));

		}

		while (!vecNodes.empty())
		{
			delete vecNodes.back();
			vecNodes.pop_back();
		}
#endif

		SET_TIME(&start);
		//Sort points by angle and add to vectors
		for (int i=0;i<keypoints.size();i++)
		{
			KeyPoint kp = keypoints[i];
			Point2i point = Point2i((int)kp.pt.x,(int)kp.pt.y);	
			if (kp.angle > 180)
			{
				insideCornerVector.push_back(point);
				centroid += point;
			}
			else
			{	
				if (debugLevel > 2)
					debugVector.push_back(new DebugCircle(point,4,Colors::Red));
				outsideCornersVector.push_back(point);
			}
		}	
		SET_TIME(&end);
		double maxThreshTime_local = calc_time_double(start,end);
		maxThreshTime = (maxThreshTime + maxThreshTime_local)/2.0;
	}

	//No features to track, so return NULL
	if (outsideCornersVector.size() < 2 || insideCornerVector.size() < 2)
	{
		return;
	}

	
	

	
	centroid = Point2i((int)round(((float)centroid.x)/((float)insideCornerVector.size())), 
		(int)round(((float)centroid.y)/((float)insideCornerVector.size())));

	SET_TIME(&start);
	LOGD(LOGTAG_QRFAST,"Sorting inside points according to distance from (%d,%d)",centroid.x,centroid.y);
	sort(insideCornerVector.begin(),insideCornerVector.end(),PointCompare(centroid));
	SET_TIME(&end);
	LOG_TIME_PRECISE("Sorting by distance",start,end);

	/*debugVector.push_back(new DebugCircle(insideCornerVector[0],15,Colors::DeepSkyBlue,2));	
	debugVector.push_back(new DebugCircle(insideCornerVector[insideCornerVector.size()-1],15,Colors::Fuchsia,2));*/
	
	int numFeatures = outsideCornersVector.size();	


	Mat outsideCornerMat = Mat(numFeatures,2,CV_32F);
	for (int i = 0;i < outsideCornersVector.size();i++)
	{
		outsideCornerMat.at<float>(i,0) = (float)outsideCornersVector.at(i).x;
		outsideCornerMat.at<float>(i,1) = (float)outsideCornersVector.at(i).y;
	}
	
	LOGD(LOGTAG_QRFAST,"InsideCorners=%d,OutsideCorners=%d,OCM=%d",insideCornerVector.size(),outsideCornersVector.size(),outsideCornerMat.size().area());
		
	
	

	
	//Build the FLANN index
	SET_TIME(&start);	
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

	SET_TIME(&end);
	LOG_TIME_PRECISE("Building FLANN index",start,end);
		

	//Results
	vector<QRCorner> resultVector;
	
	//Loop variable declarations
	Mat indexMatrix = Mat::ones(1, searchSize, CV_32S) * -1;
	Mat distanceMatrix = Mat::zeros(1, searchSize, CV_32F);	
	vector<pair<int,Point2i> > closePointsVector(searchSize);
	Mat objectPoints = Mat(1,2,CV_32F);

	//Search exclusion
	int excludeSize =insideCornerVector.size();
	Mat exclusionIndexMatrix = Mat::ones(excludeSize, searchSize, CV_32S) * -1;
	Mat exclusionDistanceMatrix = Mat::zeros(excludeSize, searchSize, CV_32F);	
	cv::flann::Index * searchExclusionIndex = NULL;
	Mat exclusionPoints = Mat(1,2,CV_32F);
	vector<pair<int,Point2i> > exclusionDistances;

	//Timer declarations
	struct timespec pointStart,pointEnd;
	double flannTime_local = 0;
	SET_TIME(&pointStart);

	for (vector<Point2i>::iterator insideIt = insideCornerVector.begin(); insideIt != insideCornerVector.end(); insideIt++)
	{
		Point2i insideCornerPoint = *insideIt;
		if (debugLevel > 2)
			debugVector.push_back(new DebugCircle(insideCornerPoint,4,Colors::Gold));

		objectPoints.at<float>(0,0) = (float)insideCornerPoint.x;
		objectPoints.at<float>(0,1) = (float)insideCornerPoint.y;

		//If point is within a defined exclusion zone, don't check it
		Point2i match(0,0);
		/*if (checkExclusionZone(objectPoints,searchExclusionIndex,exclusionDistances,exclusionIndexMatrix,exclusionDistanceMatrix,match))
		{
			if (debugLevel > 1)
				debugVector.push_back(new DebugLine(insideCornerPoint,match,Colors::OrangeRed,2));
			continue;
		}
*/


		struct timespec innerStart,innerEnd;
		SET_TIME(&innerStart);

		int result = searchSize;

		if (useRadius)
		{
			int radiusSearchResult = flannPointIndex->radiusSearch(objectPoints, indexMatrix, distanceMatrix,flannRadius, searchSize , cv::flann::SearchParams(searchParams)); 
			if (radiusSearchResult < searchSize)
				result = radiusSearchResult;
		}
		else
		{
			flannPointIndex->knnSearch(objectPoints, indexMatrix, distanceMatrix, searchSize, cv::flann::SearchParams(searchParams));
		}
		//LOGV(LOGTAG_QRFAST,"RadiusMode=%d,Pt(%d,%d) - FLANN(%d < %lf) complete(%d).",useRadius,insideCornerPoint.x,insideCornerPoint.y,searchSize,flannRadius,result);
		

		SET_TIME(&innerEnd);
		flannTime_local += calc_time_double(innerStart,innerEnd);


		//Clear results from last point
		closePointsVector.clear();
		if (result != searchSize)
			closePointsVector.resize(result);
		
		//Put results into a vector for processing
		int * indexPtr = indexMatrix.ptr<int>(0);
		float * distPtr = distanceMatrix.ptr<float>(0);
		float * rowPtr;
		for (int i=0;i < result;i++)
		{
			//Ignore points farther than search radius
			if (!useRadius && distPtr[i] > flannRadius)
				break;

			rowPtr = outsideCornerMat.ptr<float>(indexPtr[i]);
			closePointsVector.push_back(pair<int,Point2i>(distPtr[i],Point2i((int)(rowPtr[0]),(int)(rowPtr[1]))));
		}
		
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

		vector<pair<int,Point2i> >::iterator closePointsIt, closePointsEnd;

		closePointsIt = closePointsVector.begin();
		int closestDist = (*closePointsIt).first;
				
		Point2i firstPoint =  (*closePointsIt).second;
		closePointsIt++;
		closePointsEnd = closePointsVector.end();

		float MaxCosine = -0.8f;
		float lowestCosine = -0.3f;

		Point2i bestPoint;
		int bestPointDistance;
		
		//Accept points that form correct angle with closest
		for (; closePointsIt != closePointsEnd; closePointsIt++)
		{			
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
				bestPointDistance = (*closePointsIt).first;
			}
		}
		
		
		
		//Validate slope
		if (bestPoint.x == 0 && bestPoint.y==0)
		{		
			if (debugLevel > 2)
			{
				debugVector.push_back(new DebugLine(insideCornerPoint,firstPoint,Colors::Aqua));
			}
			continue;
		}
			


		//LOGV(LOGTAG_QRFAST,"Adding point to exclusion zone. Pt=(%d,%d),Radius=%d",bestPoint.x,bestPoint.y,bestPointDistance);
		addExclusionZone(bestPoint,bestPointDistance,searchExclusionIndex,exclusionDistances);
		resultVector.push_back(QRCorner(insideCornerPoint,bestPoint,firstPoint,10));

		if (debugLevel > 3)
		{
			char str[50];
			sprintf(str,"#%d",resultVector.size());
			debugVector.push_back(new DebugLabel(insideCornerPoint,str,Colors::Black,1.0f));
		}

		if (debugLevel > -1)
		{
			debugVector.push_back(new DebugLine(insideCornerPoint,firstPoint,Colors::Lime,2));
			debugVector.push_back(new DebugLine(insideCornerPoint,bestPoint,Colors::Lime,2));
		}
		
		indexMatrix = Mat::ones(1, searchSize, CV_32S) * -1;

	
	}
	SET_TIME(&pointEnd);
	double pointTime_local = calc_time_double(pointStart,pointEnd);


	flannTime = (flannTime  + flannTime_local)/2.0;
	pointTime = (pointTime + pointTime_local)/2.0;
	avgPerPointTime = (avgPerPointTime + pointTime_local/((double)insideCornerVector.size()))/2.0;
	
	config->SetLabelValue("FlannTime",flannTime/1000.0);
	config->SetLabelValue("TotalTime",pointTime/1000.0);
	config->SetLabelValue("NumPoints",(float)insideCornerVector.size());
	config->SetLabelValue("AvgPointTime",avgPerPointTime);
	
	
	delete flannPointIndex;

	
	
	SET_TIME(&endTotal);
	LOG_TIME("TotalFASTQR",startTotal,endTotal);
	return;
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

