#include "FastQRFinder.hpp"


void FastQR::ThreePointLine::DrawDebug(vector<Drawable*> & debugVector)
{
	debugVector.push_back(new DebugCircle(pt0,6,Colors::Red,1));
	debugVector.push_back(new DebugCircle(pt1,6,Colors::Orange,1));
	debugVector.push_back(new DebugCircle(pt2,6,Colors::Yellow,1));
}


class ThreePointLineCompare
{
public:      
	ThreePointLineCompare()
	{
		//maxVariance = _maxVariance;
	}

	bool operator()(const multimap<float,FastQR::ThreePointLine> & pt0, const multimap<float,FastQR::ThreePointLine> & pt1)
	{
		float totalVariance0 = 0;
		int count0 = 0;
		for (multimap<float,FastQR::ThreePointLine>::const_iterator it = pt0.begin(); it != pt0.end();it++)
		{
			totalVariance0 += (*it).first;
			count0++;
		}
		if (count0 == 0)
			totalVariance0 = MAXFLOAT;
		else
			totalVariance0 /= (float)count0;

		float totalVariance1 = 0;
		int count1 = 0;
		for (multimap<float,FastQR::ThreePointLine>::const_iterator it = pt1.begin(); it != pt1.end();it++)
		{
			totalVariance1 += (*it).first;
			count1++;
		}
		if (count1 == 0)
			totalVariance1 = MAXFLOAT;
		else
			totalVariance1 /= (float)count1;

		return totalVariance0 < totalVariance1;
	}
//private:
	//float maxVariance;
};




FastQRFinder::FastQRFinder(ARControllerDebugUI * debugUI)
{
	config = debugUI;

	
	config->AddNewParameter("Min Inner Points","MaxInnerPoints",8,1,1,50,"%2.0f","Debug");
	
	config->AddNewParameter("RadiusSearch","DoRadiusSearch",0,1,0,1,"%1.0f","FAST",true);
	config->AddNewParameter("FLANNRadius",25,1,3,300,"%3.0f","FAST");
	config->AddNewParameter("FlannIndexType",1,1,0,2,"%1.0f","FAST",true);


	config->AddNewParameter("K Nearest","K-NN",6,1,1,12,"%2.0f","FAST");
	config->AddNewParameter("NumKDTrees",1,1,1,16,"%2.0f","FAST");
	config->AddNewParameter("FlannSearchParams",32,32,32,64,"%2.0f","FAST");
		
	config->AddNewParameter("FAST Debug Level","FASTDebug",0,1,-2,4,"%2.0f","Debug");
	config->AddNewParameter("FAST AP Debug Level","FASTAPDebug",0,1,-2,4,"%2.0f","Debug");
	
	config->AddNewParameter("Post-NonMaxSupress Size","MaxThreshSize",2,1,0,25,"%2.0f","FAST");
	config->AddNewParameter("Post-NonMaxSupress Count","MaxThreshCount",2,1,0,25,"%2.0f","FAST");
	config->AddNewParameter("AP MaxThreshScale","MaxThreshScale",0.5f,0.1f,0.1f,2.0f,"%3.2f","FAST");
			
	config->AddNewParameter("FAST Threshold","FastThresh",10,5,1,400,"%3.0f","FAST");
	config->AddNewParameter("NonMaxSuppress",0,1,0,1,"%1.0f","FAST");

	

	flannTime = 10;
	pointTime = 0;
	maxThreshTime = 20;
	fastTime = 0;
	clusterTime = 0;
	
	
	//config->AddNewLabel("FAST time"," ms ");
	//config->AddNewLabel("FlannTime"," ms ");
	//config->AddNewLabel("AvgPointTime"," us ");
	//config->AddNewLabel("NumFPPoints","");
	//config->AddNewLabel("MaxPt Time"," ms ");
	//config->AddNewLabel("Cluster Time"," ms ");
}

#define AbsoluteMacro(x) (x >= 0) ? x : x


//Faster? Less accurate for certain.
int FastQRFinder::GetDistanceFast(int dx, int dy)
{
	dx = AbsoluteMacro(dx);
	dy = AbsoluteMacro(dy);
	int mn = MIN(dx,dy);
	return(dx+dy-(mn>>1)-(mn>>2)+(mn>>4));
} 
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
					newMap->insert(pair<int,KeyPoint>((int)kp.pt.y,kp));
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

static void sortPointsByAngle(map<int,map<int,KeyPoint>*> & keyPointMap_Outer, map<int,map<int,KeyPoint>*> & keyPointMap_Inner, const vector<KeyPoint> keypoints, bool insideOnly = false)
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
		else if (!insideOnly) 
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


static void processFinderPatternResults(vector<FastQR::ThreePointLine> & resultVector, Point2i patternCenter, vector<FastQR::ThreePointLine> & sortedLines, vector<Drawable*> & debugVector, int debugLevel)
{
	int avgSize = 0;
	for (int i=0;i<resultVector.size();i++)
	{
		FastQR::ThreePointLine line = resultVector[i];
		avgSize += line.length;
	}
	
	if (debugLevel > 0)
	{
		debugVector.push_back(new DebugCircle(patternCenter,1,Colors::Azure,-1));
		debugVector.push_back(new DebugCircle(patternCenter,10,Colors::Azure,1));
	}

	vector<multimap<float,FastQR::ThreePointLine> > pointsByVariance;

	for (int i=0;i<resultVector.size();i++)
	{
		multimap<float,FastQR::ThreePointLine> varianceMap;
		for (int j=0;j<resultVector.size();j++)
		{
			float variance = abs(((float)resultVector[j].length/(float)resultVector[i].length)  - 1.0f);
			varianceMap.insert(pair<float,FastQR::ThreePointLine>(variance,resultVector[j]));
		}
		pointsByVariance.push_back(varianceMap);
	}
	
	//Sort vector by average variance in each map
	std::sort(pointsByVariance.begin(),pointsByVariance.end(),ThreePointLineCompare());

	//First value in vector should contain most similar lines
	for (int i=0;i<pointsByVariance.size();i++)
	{
		if (pointsByVariance[i].size() >= 4)
		{
			int count = 0;
			for (multimap<float,FastQR::ThreePointLine>::iterator it = pointsByVariance[i].begin(); it != pointsByVariance[i].end(); it++)
			{
				if (count++ == 4)
					break;

				FastQR::ThreePointLine line = (*it).second;

				int d1 = GetSquaredDistance(line.pt0,patternCenter);
				int d2 = GetSquaredDistance(line.pt2,patternCenter);

				if (d2 > d1)
				{
					Point2i tmp = line.pt2;
					line.pt2 = line.pt0;
					line.pt0 = tmp;
				}
								
				if (debugLevel > 0)
				{
					debugVector.push_back(new DebugLine(line.pt1,line.pt2,Colors::Lime,2));
					debugVector.push_back(new DebugLine(line.pt1,line.pt0,Colors::Lime,2));
				}	

				sortedLines.push_back(line);
			}
			//sortedLines.push_back(resultVector[i].firs);
			break;
		}
	}
}

static bool keyPointTest(vector<Point2i> & innerPoints, FinderPattern * pattern, int maxInnerPoints)
{	
	float minInnerPointRadius = pow(MIN(pattern->patternSize.width,pattern->patternSize.height)/3.0f,2);

	int count = 0;
	for (int i = 0;i<innerPoints.size();i++)
	{
		float distance = GetSquaredDistance(pattern->pt,innerPoints[i]);
		if (distance >= minInnerPointRadius)
		{
			if (count++ > maxInnerPoints)
				return false;
		}
	}

	return true;
}

//void FastQRFinder::EnhanceQRCodes(Mat & img, QRCode * code, vector<Drawable*> & debugVector)
//{
//	int debugLevel = (int)config->GetParameter("FASTDebug");
//	for (int i=0;i<code->finderPatterns.size();i++)
//	{
//		FinderPattern * fp = code->finderPatterns.at(i);
//		if (fp->size == 0)
//			continue;
//		
//		float windowSize = fp->size * 1.4f;  //Window needs to be bigger to account for diagonals
//	
//		Rect window = Rect(fp->pt.x - windowSize/2.0f, fp->pt.y-windowSize/2.0f, windowSize,windowSize);
//				
//		vector<Point2i> corners;
//		LocateFPCorners(img, window,corners,debugVector);
//
//		for (int j=0;j<corners.size();j++)
//		{
//			if (debugLevel > -1)
//			{
//				debugVector.push_back(new DebugCircle(corners.at(j),12,Colors::MediumVioletRed,2));
//			}		
//		}
//
//		if (debugLevel > 0)
//		{
//			if (corners.size() == 0)
//				debugVector.push_back(new DebugRectangle(window,Colors::Aqua));
//			else
//				debugVector.push_back(new DebugRectangle(window,Colors::DodgerBlue));
//		}
//	}
//}

static void MapBasedSuppression(vector<KeyPoint> & keypoints, vector<Point2i> & outsideCornersVector, vector<Point2i> & insideCornersVector, Size2i searchSize, int iterations, bool insideOnly = false)
{
	map<int,map<int,KeyPoint>*> insideCornersMap_Unsorted;
	map<int,map<int,KeyPoint>*> outsideCornersMap_Unsorted;
	sortPointsByAngle(outsideCornersMap_Unsorted,insideCornersMap_Unsorted,keypoints);
	
	map<int,map<int,KeyPoint>*> insideCornersMap;	
	map<int,map<int,KeyPoint>*> outsideCornersMap;

	//Extract local maximums	
	//SET_TIME(&start);
	sortKPMap(insideCornersMap_Unsorted,insideCornersMap,iterations,searchSize);

	if (!insideOnly) 
		sortKPMap(outsideCornersMap_Unsorted,outsideCornersMap,iterations,searchSize);

	//Add points to vectors
	for (map<int,map<int,KeyPoint>*>::iterator  xIterator = insideCornersMap.begin();xIterator != insideCornersMap.end(); xIterator++)
	{	
		for (map<int,KeyPoint>::iterator yIterator = (*xIterator).second->begin(); yIterator != (*xIterator).second->end(); yIterator++)
		{
			Point2i point = Point2i((*xIterator).first,(*yIterator).first);//(int)(*yIterator).second.pt.x,(int)(*yIterator).second.pt.y);
			insideCornersVector.push_back(point);
		}
	}
	if (!insideOnly) 
	{
		for (map<int,map<int,KeyPoint>*>::iterator  xIterator = outsideCornersMap.begin();xIterator != outsideCornersMap.end(); xIterator++)
		{
			for (map<int,KeyPoint>::iterator yIterator = (*xIterator).second->begin(); yIterator != (*xIterator).second->end(); yIterator++)
			{
				KeyPoint kp = (*yIterator).second;
				Point2i point = Point2i((int)kp.pt.x,(int)kp.pt.y);
				outsideCornersVector.push_back(point);
			}
		}
	}
	//Cleanup maps
	for (map<int,map<int,KeyPoint>*>::iterator deleteIt = insideCornersMap.begin(); deleteIt != insideCornersMap.end();deleteIt++) delete (*deleteIt).second;
	for (map<int,map<int,KeyPoint>*>::iterator deleteIt = outsideCornersMap.begin(); deleteIt != outsideCornersMap.end();deleteIt++) delete (*deleteIt).second;
	for (map<int,map<int,KeyPoint>*>::iterator deleteIt = insideCornersMap_Unsorted.begin(); deleteIt != insideCornersMap_Unsorted.end();deleteIt++) delete (*deleteIt).second;
	for (map<int,map<int,KeyPoint>*>::iterator deleteIt = outsideCornersMap_Unsorted.begin(); deleteIt != outsideCornersMap_Unsorted.end();deleteIt++) delete (*deleteIt).second;
}

static void BuildFLANNIndex(cv::flann::Index *& flannPointIndex, int numTrees, vector<Point2i> & features,	Mat & outsideCornerMat, int flannIndexType = 1)
{
	int numFeatures = features.size();
	outsideCornerMat = Mat(numFeatures,2,CV_32F);
	for (int i = 0;i < features.size();i++)
	{
		outsideCornerMat.at<float>(i,0) = (float)features.at(i).x;
		outsideCornerMat.at<float>(i,1) = (float)features.at(i).y;
	}


	//Build the FLANN index
	struct timespec start,end;
	SET_TIME(&start);	
	if (flannIndexType == 0)
	{		
		flannPointIndex = new flann::Index(outsideCornerMat, cv::flann::LinearIndexParams());
	}
	else if (flannIndexType == 1)
	{
		flannPointIndex = new flann::Index(outsideCornerMat,cv::flann::KDTreeIndexParams(numTrees));  
	}
	else if (flannIndexType == 2)
	{		
		flannPointIndex = new flann::Index(outsideCornerMat,cv::flann::KMeansIndexParams());
	}
	else
	{
		LOGE("No index type defined!");
		throw exception();
	}

	SET_TIME(&end);
	LOG_TIME_PRECISE("Building FLANN index",start,end);
}

static bool validateSquare(vector<Point2i> & inputPoints, vector<Point2i> & squarePoints)
{
	const float minCosine = 0.3f;
		
	if (inputPoints.size() < 4)
		return false;

	vector<Point2i> testPoints = inputPoints;

	for (int i=0;i<inputPoints.size();i++)
	{
		sort(testPoints.begin(),testPoints.end(),PointCompare(inputPoints[i]));

		//Skip first one, will be equal to test point, and thus zero distance
		float angle = FastTracking::angle(testPoints[1],testPoints[2],inputPoints[i]);

		if (angle <= minCosine)
		{
			squarePoints.push_back(inputPoints[i]);
			if (squarePoints.size() == 4)
				return true;
		}
	}
	return false;
}

void FastQRFinder::CheckAlignmentPattern(Mat & img, Point2i center, Size2f patternSize, vector<Point2i> & patternPoints, vector<Drawable*> & debugVector)
{	
	struct timespec startTotal,endTotal;
	SET_TIME(&startTotal);
	int threshold = config->GetParameter("FastThresh");
	bool nonMaxSuppress = config->GetBooleanParameter("NonMaxSuppress");
	int debugLevel = (int)config->GetParameter("FASTAPDebug");	
	int searchIterations = config->GetParameter("MaxThreshCount");
	float maxThreshScale = config->GetFloatParameter("MaxThreshScale");	
	float fastRegionScale = config->GetFloatParameter("APFastScale");
	//int scoreSearchSize = config->GetParameter("MaxThreshSize");

	Size2f patternSizeRect = Size2f(patternSize.width * fastRegionScale,patternSize.height * fastRegionScale);
	Rect fastRect = Rect(center.x-(patternSizeRect.width/2.0f),center.y-(patternSizeRect.height/2.0f),patternSizeRect.width, patternSizeRect.height);

	if (debugLevel > 0)
		debugVector.push_back(new DebugRectangle(fastRect,Colors::LawnGreen,1));

	vector<KeyPoint> keypoints;
	FastWindow(img,keypoints,fastRect, threshold, nonMaxSuppress);
	
	Size2i searchSize = Size2i((int)round((float)patternSize.width*maxThreshScale),(int)round((float)patternSize.height*maxThreshScale)); //Size2i(scoreSearchSize,scoreSearchSize)
	vector<Point2i> insideCornerVector, outsideCornersVector;
	MapBasedSuppression(keypoints,outsideCornersVector,insideCornerVector,searchSize,searchIterations);
		

	//Draw points for debug view	
	//if (debugLevel > 4)
	//{
	//	for (int i=0;i<outsideCornersVector.size();i++)
	//	{
	//		debugVector.push_back(new DebugCircle(outsideCornersVector[i],4,Colors::Red));
	//	}
	//}
	if (debugLevel > 2)
	{
		for (int i=0;i<insideCornerVector.size();i++)
		{	
			debugVector.push_back(new DebugCircle(insideCornerVector[i],4,Colors::Gold,1,false));		
			//LOGD(LOGTAG_QRFAST,"AP-Point[%d]=(%d,%d)",i,insideCornerVector[i].x,insideCornerVector[i].y);
		}
	}

	std::sort(insideCornerVector.begin(),insideCornerVector.end(),PointCompare(center));

	vector<Point2i> testPoints;
	int count = 0;
	for (vector<Point2i>::iterator it = insideCornerVector.begin(); it != insideCornerVector.end(); it++)
	{
		if (count++ == 8) 
			break;
		Point2i newPoint = (*it);
		testPoints.push_back(newPoint);
		if (debugLevel > 1) 
			debugVector.push_back(new DebugCircle(newPoint,10,Colors::OrangeRed,1,true));									
	}

	//vector<Point2i> squarePoints;
	validateSquare(testPoints,patternPoints);

	if (debugLevel > 0)
	{
		config->SetLabelValue("NumPoints",(float)insideCornerVector.size());		
	}
	SET_TIME(&endTotal);
	LOG_TIME_PRECISE("AlignCheck",startTotal,endTotal);
}

void FastQRFinder::LocateFPCorners(Mat & img, FinderPattern * pattern, vector<Point2i> & corners, vector<Drawable*> & debugVector)
{
	struct timespec startTotal,endTotal;
	SET_TIME(&startTotal);

	//Get parameters from UI. Need to store as local variables because they are accessed many times per frame.
	int debugLevel = (int)config->GetParameter("FASTDebug");
	int searchSize = config->GetIntegerParameter("K-NN");
	 //= std::pow(config->GetParameter("FLANNRadius"),2.0);
	int searchParams = config->GetIntegerParameter("FlannSearchParams");
	bool useRadius = config->GetBooleanParameter("DoRadiusSearch");
	int flannIndexType = config->GetIntegerParameter("FlannIndexType");
	int numTrees = config->GetIntegerParameter("NumKDTrees");
	int threshold = config->GetParameter("FastThresh");
	int maxInnerPoints = config->GetIntegerParameter("MaxInnerPoints");
	bool nonMaxSuppress = config->GetBooleanParameter("NonMaxSuppress");

	//FinderPattern search area
	float searchRegion = pattern->size * 1.4f;  //Window needs to be bigger to account for diagonals
	Rect window = Rect(pattern->pt.x - searchRegion/2.0f, pattern->pt.y-searchRegion/2.0f, searchRegion,searchRegion);
	float flannRadius = pow(MIN(pattern->patternSize.width,pattern->patternSize.height)/2.0f,2);
	
	//FAST corner detection
	vector<KeyPoint> keypoints;

	struct timespec start,end;

	SET_TIME(&start);
	FastWindow(img,keypoints,window, threshold,nonMaxSuppress);
	SET_TIME(&end);
	LOG_TIME_PRECISE("FAST",start,end);
	//double fastTime_local = calc_time_double(start,end);
	//fastTime = (fastTime + fastTime_local)/2.0;
	//config->SetLabelValue("FAST time",(float)fastTime/1000.0f);
	//LOGD(LOGTAG_QRFAST,"FAST complete. %d points found.",keypoints.size());

	
	
	vector<Point2i> insideCornerVector, outsideCornersVector;

	//If didn't do nonmax during FAST, perform here using maps
	if (!nonMaxSuppress)
	{		
		int scoreSearchSize = config->GetParameter("MaxThreshSize");
		int searchIterations = config->GetParameter("MaxThreshCount");
		SET_TIME(&start);
		MapBasedSuppression(keypoints,outsideCornersVector,insideCornerVector,Size2i(scoreSearchSize,scoreSearchSize),searchIterations);
		SET_TIME(&end);
		LOG_TIME_PRECISE("MaxThresh",start,end);
		//double maxThreshTime_local = calc_time_double(start,end);
		//maxThreshTime = (maxThreshTime + maxThreshTime_local)/2.0;
		//config->SetLabelValue("MaxPt Time", (float)maxThreshTime/1000.0f);
	}
	//Otherwise, just add the points to vectors
	else
	{	
		SET_TIME(&start);
		//Sort points by angle and add to vectors
		for (int i=0;i<keypoints.size();i++)
		{
			KeyPoint kp = keypoints[i];
			Point2i point = Point2i((int)kp.pt.x,(int)kp.pt.y);	
			if (kp.angle > 180)
			{
				insideCornerVector.push_back(point);
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

	for (int i=0;i<outsideCornersVector.size();i++)
	{
		if (debugLevel > 2)
			debugVector.push_back(new DebugCircle(outsideCornersVector[i],4,Colors::Red));
	}

	for (int i=0;i<insideCornerVector.size();i++)
	{	
		if (debugLevel > 2)
		{
			debugVector.push_back(new DebugCircle(insideCornerVector[i],4,Colors::Gold));
		}
	}
	//Sort by distance to pattern center, farthest first
	sort(insideCornerVector.begin(),insideCornerVector.end(),PointCompare(pattern->pt));
	

	//Test keypoints for validity. If this test fails, then return.
	if (!keyPointTest(insideCornerVector,pattern,maxInnerPoints))
	{
		if (debugLevel > 1)
		{
			debugVector.push_back(new DebugCircle(pattern->pt,15,Colors::DarkViolet,1));
		}
		return;
		SET_TIME(&endTotal);
		LOG_TIME("TotalFASTQR-KPFail",startTotal,endTotal);
	}


	Mat outsideCornerMat;
	cv::flann::Index * flannPointIndex;
	BuildFLANNIndex(flannPointIndex, numTrees, outsideCornersVector,outsideCornerMat,flannIndexType);
	
	//Results
	vector<FastQR::ThreePointLine> resultVector;
	
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

		int result;
		if (useRadius)
		{
			int radiusSearchResult = flannPointIndex->radiusSearch(objectPoints, indexMatrix, distanceMatrix,flannRadius, searchSize , cv::flann::SearchParams(searchParams)); 
			result = MIN(radiusSearchResult,searchSize);
		}
		else
		{
			flannPointIndex->knnSearch(objectPoints, indexMatrix, distanceMatrix, searchSize, cv::flann::SearchParams(searchParams));
			result = MIN(searchSize,outsideCornerMat.rows);
		}
		//LOGV(LOGTAG_QRFAST,"RadiusMode=%d,Pt(%d,%d) - FLANN(%d < %lf) complete(%d).",useRadius,insideCornerPoint.x,insideCornerPoint.y,searchSize,flannRadius,result);
		SET_TIME(&innerEnd);
		flannTime_local += calc_time_double(innerStart,innerEnd);


		//Clear results from last point
		closePointsVector.clear();
		if (result != searchSize)
		{
			closePointsVector.resize(result);
		}
		
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
			closePointsVector.push_back(pair<int,Point2i>((int)round(distPtr[i]),Point2i((int)(rowPtr[0]),(int)(rowPtr[1]))));
		}
		
		if (closePointsVector.size() < 2)
		{
			if (debugLevel > 2)
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
		float lowestCosine = -0.6f;

		Point2i bestPoint;
		int bestPointDistance;
		
		//int testDist = GetSquaredDistance(firstPoint,insideCornerPoint);
		//LOGV(LOGTAG_QRFAST,"Dist=%d,MyDist=%d",closestDist,testDist);

		//Accept points that form correct angle with closest
		for (; closePointsIt != closePointsEnd; closePointsIt++)
		{			
			Point2i testPoint = (*closePointsIt).second;
			if (GetSquaredDistance(firstPoint,testPoint) < closestDist)
			{				
				if (debugLevel > 2)
					debugVector.push_back(new DebugLine(insideCornerPoint,testPoint,Colors::Blue));
				continue;
			}
			if (debugLevel > 2)
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

		FastQR::ThreePointLine line = FastQR::ThreePointLine(firstPoint,insideCornerPoint,bestPoint,bestPointDistance,lowestCosine);
		resultVector.push_back(line);

		if (debugLevel > 0)
		{/*
			debugVector.push_back(new DebugLine(line.pt1,line.pt2,Colors::Green,1));
			debugVector.push_back(new DebugLine(line.pt1,line.pt0,Colors::Green,1));*/
			line.DrawDebug(debugVector);
		}		
		indexMatrix = Mat::ones(1, searchSize, CV_32S) * -1;
	}
	SET_TIME(&pointEnd);


	//flannTime = (flannTime  + flannTime_local)/2.0;
	//pointTime = (pointTime + pointTime_local)/2.0;
	//avgPerPointTime = (avgPerPointTime + pointTime_local/((double)insideCornerVector.size()))/2.0;
	
	if (debugLevel > 0)
	{
		//config->SetLabelValue("FlannTime",flannTime/1000.0);
		//config->SetLabelValue("NumPoints",(float)insideCornerVector.size());
		//config->SetLabelValue("AvgPointTime",avgPerPointTime);
	}
	//Clean up FLANN index
	delete flannPointIndex;

	vector<FastQR::ThreePointLine> sortedVector;
	SET_TIME(&start);
	processFinderPatternResults(resultVector,pattern->pt, sortedVector,debugVector,debugLevel);
	SET_TIME(&end);
	LOG_TIME_PRECISE("FP Corner Analysis",start,end);

	for (int i=0;i<4 && i < sortedVector.size();i++)
	{
		corners.push_back(sortedVector[i].pt0);
	}
	
	
	return;
}
