#include "FastQRFinder.hpp"


void FastQR::ThreePointLine::DrawDebug(vector<Drawable*> & debugVector)
{
	/*debugVector.push_back(new DebugCircle(pt0,6,Colors::Red,1));
	debugVector.push_back(new DebugCircle(pt1,6,Colors::Orange,1));
	debugVector.push_back(new DebugCircle(pt2,6,Colors::Yellow,1));*/

	debugVector.push_back(new DebugLine(pt1,pt2,Colors::Aqua,2));
	debugVector.push_back(new DebugLine(pt1,pt0,Colors::Sienna,2));
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

class ThreePointLineCompare_Cosine_Ascending
{
public:
	bool operator()(const FastQR::ThreePointLine & line0, const FastQR::ThreePointLine & line1)
	{
		return line0.cosine < line1.cosine;
	}
};




FastQRFinder::FastQRFinder(ARControllerDebugUI * debugUI)
{
	config = debugUI;

	
	config->AddNewParameter("Min Inner Points","MaxInnerPoints",8,1,1,50,"%2.0f","Debug");
	
	config->AddNewParameter("RadiusSearch","DoRadiusSearch",0,1,0,1,"%1.0f","FAST",true);
	config->AddNewParameter("FLANNRadius",25,1,3,300,"%3.0f","FAST",true);
	config->AddNewParameter("FlannIndexType",1,1,0,2,"%1.0f","FAST",true);


	config->AddNewParameter("AP: K Nearest","K-NN_AP",5,1,1,12,"%2.0f","FAST_AP");

	config->AddNewParameter("K Center","NumCenterPoints",2,1,1,12,"%2.0f","Fast");
	config->AddNewParameter("K Inner","NumInnerCorners",6,1,1,12,"%2.0f","Fast");
	config->AddNewParameter("K Outer","NumOuterCorners",2,1,1,12,"%2.0f","Fast");

	config->AddNewParameter("NumKDTrees",1,1,1,16,"%2.0f","FAST",true);
	config->AddNewParameter("MaxCosine",-0.6f,0.05f,-2.0f,2.0f,"%4.2f","FAST");
	config->AddNewParameter("FlannSearchParams",32,32,32,64,"%2.0f","FAST");
		
	config->AddNewParameter("FAST Debug Level","FASTDebug",0,1,-4,4,"%2.0f","Debug");
	config->AddNewParameter("FAST AP Debug Level","FASTAPDebug",0,1,-4,4,"%2.0f","Debug");
	
	config->AddNewParameter("Post-NonMaxSupress Size","MaxThreshSize",2,1,1,25,"%2.0f","FAST");
	config->AddNewParameter("MaxThresh Size","MaxThreshSize_AP",2,1,1,25,"%2.0f","FAST_AP");
	config->AddNewParameter("Post-NonMaxSupress Count","MaxThreshCount",2,1,1,25,"%2.0f","FAST");
	config->AddNewParameter("MaxThreshScale","MaxThreshScale",0.5f,0.1f,0.1f,2.0f,"%3.2f","FAST_AP");
			
	config->AddNewParameter("FAST Threshold","FastThresh",10,5,1,400,"%3.0f","FAST");
	config->AddNewParameter("NonMaxSuppress",0,1,0,1,"%1.0f","FAST");

	config->AddNewParameter("UseSubPix",1,1,0,1,"%1.0f","FAST");
	config->AddNewParameter("SubPixWindow",5,1,1,20,"%3.0f","FAST");
	config->AddNewParameter("SubPixDeadZone",1,1,-1,20,"%3.0f","FAST");
	config->AddNewParameter("SubPixMaxCount",60,10,1,300,"%3.0f","FAST");
	config->AddNewParameter("SubPixEpsilon",0.02,0.01,0,3,"%3.2f","FAST");

	

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

static void cornerHarris(Mat & src, vector<KeyPoint> & features, int threshold)
{
  Mat dst, dst_norm, dst_norm_scaled;
 
  dst = Mat::zeros(src.size(), CV_32FC1 );

  /// Detector parameters
  int blockSize = 2;
  int apertureSize = 3;
  double k = 0.04;

  /// Detecting corners
  cornerHarris( src, dst, blockSize, apertureSize, k, BORDER_DEFAULT );

  /// Normalizing
  normalize( dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat() );
  convertScaleAbs( dst_norm, dst_norm_scaled );

  /// Drawing a circle around corners
  for( int j = 0; j < dst_norm.rows ; j++ )  
  { 
	  for( int i = 0; i < dst_norm.cols; i++ )
	  {
		  if( (int) dst_norm.at<float>(j,i) > 200 )
		  {
			  features.push_back(KeyPoint(i,j,180,20));
		  }
	  }
  }
  //LOGV(LOGTAG_QRFAST,"Harris found %d features");

}

static void HarrisWindow(Mat & inputImg, vector<KeyPoint> & features, Rect window, int threshold, bool nonMax)
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
		cornerHarris(tMat,features,threshold);
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

static void processFinderPatternResults2(vector<FastQR::ThreePointLine> & resultVector, Point2i patternCenter, vector<FastQR::ThreePointLine> & sortedLines, vector<Drawable*> & debugVector, int debugLevel)
{

	//Calculate all the angles to horizontal
	for (int i=0;i<resultVector.size();i++)
	{
		//resultVector[i].angleToHorizontal = atanf(((float)resultVector[i].pt1.y)/((float)resultVector[i].pt1.x));
		FastQR::ThreePointLine line = resultVector[i];
		Point2f endPoint = Point2f(line.pt0.x-line.pt2.x,line.pt0.y-line.pt2.y);
		resultVector[i].angleToHorizontal = atanf(endPoint.y/endPoint.x);
	}

	multimap<int,FastQR::ThreePointLine> lineMap;
	
	float minAngleDiff = 10.0f * (PI/180.0f);

	Scalar friendlyColors[]  = {Colors::Red,Colors::Blue,Colors::DarkGoldenrod, Colors::AliceBlue,Colors::Aqua,Colors::DarkGray, Colors::DarkGreen};
	int colorIndex = 0, colorSize = 7;
	for (int i=0;i<resultVector.size();i++)
	{
		FastQR::ThreePointLine line = resultVector[i];
		int parallelCount = 0;
		for (int j=0;j<resultVector.size();j++)
		{
			if (i==j) continue;
			FastQR::ThreePointLine line2 = resultVector[j];


			if (abs(line2.angleToHorizontal-line.angleToHorizontal) < minAngleDiff ||
				abs(abs(line2.angleToHorizontal-line.angleToHorizontal) - PI) < minAngleDiff)
			{

				if (debugLevel == 3)
				{
					colorIndex = (colorIndex+1)%3;
					debugVector.push_back(new DebugLine(line.pt1,line.pt2,friendlyColors[colorIndex],2));
					debugVector.push_back(new DebugLine(line.pt1,line.pt0,friendlyColors[colorIndex],2));
					debugVector.push_back(new DebugLine(line2.pt1,line2.pt2,friendlyColors[colorIndex],2));
					debugVector.push_back(new DebugLine(line2.pt1,line2.pt0,friendlyColors[colorIndex],2));
				}

				LOGV(LOGTAG_QRFAST,"Pass! Angle1=%f,Angle2=%f",line.angleToHorizontal,line2.angleToHorizontal);
				parallelCount++;
			}			
			else
			{				
				LOGV(LOGTAG_QRFAST,"Fail! Angle1=%f,Angle2=%f",line.angleToHorizontal,line2.angleToHorizontal);
			}
		}
		lineMap.insert(pair<int,FastQR::ThreePointLine>(parallelCount,line));
	}

	pair<multimap<int,FastQR::ThreePointLine>::iterator,multimap<int,FastQR::ThreePointLine>::iterator> itPair = lineMap.equal_range(1);
	
	itPair = lineMap.equal_range(1);
	
	
	int count = 0;
	for (; itPair.first != itPair.second;itPair.first++)
	{
		FastQR::ThreePointLine line = (*itPair.first).second;
		
		if (count++ == 4)
			break;
		
		int d1 = GetSquaredDistance(line.pt0,patternCenter);
		int d2 = GetSquaredDistance(line.pt2,patternCenter);

		if (d2 > d1)
		{
			Point2i tmp = line.pt2;
			line.pt2 = line.pt0;
			line.pt0 = tmp;
		}
		if (debugLevel == 1)
		{
			debugVector.push_back(new DebugLine(line.pt1,line.pt2,Colors::Lime,2));
			debugVector.push_back(new DebugLine(line.pt1,line.pt0,Colors::Lime,2));
		}	
		sortedLines.push_back(line);
	}

	itPair = lineMap.equal_range(2);

	for (; itPair.first != itPair.second;itPair.first++)
	{
		FastQR::ThreePointLine line = (*itPair.first).second;
		if (debugLevel == 2)
		{
			debugVector.push_back(new DebugLine(line.pt1,line.pt2,Colors::Pink,2));
			debugVector.push_back(new DebugLine(line.pt1,line.pt0,Colors::Pink,2));
		}	
	}

}


static bool keyPointTest(vector<Point2i> & outerPoints, FinderPattern * pattern, int maxInnerPoints, float & radius2)
{	
	float minInnerPointRadius = pow(MIN(pattern->patternSize.width,pattern->patternSize.height)*(7.0f/6.0f)*1.4f,2);
	radius2 = minInnerPointRadius;

	int count = 0;
	for (int i = 0;i<outerPoints.size();i++)
	{
		float distance = GetSquaredDistance(pattern->pt,outerPoints[i]);
		if (distance >= minInnerPointRadius)
		{
			if (count++ > maxInnerPoints)
				return false;
		}
	}

	return true;
}


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

	struct timespec start,end;
	//SET_TIME(&start);	
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

	/*SET_TIME(&end);
	LOG_TIME_PRECISE("Building FLANN index",start,end);*/
}

static bool addNextVertex(int currentIndex, int lastIndex, set<int> & chosenIndices, vector<Point2i> & inputPoints, int itCount, vector<Drawable*> & debugVector, Scalar lineColor)
{
	if (itCount-- < 0)
		return false;
	
	if (chosenIndices.size() == 4)
		return true;

	//Check if we reached the first point (0)
	float angle = FastTracking::angle(inputPoints[0],inputPoints[lastIndex],inputPoints[currentIndex]);
	if (abs(angle) < 0.15f)
	{
		debugVector.push_back(new DebugArrow(inputPoints[currentIndex],inputPoints[0],lineColor,2));
		return true;
	}

	for (int j=0;j<inputPoints.size();j++)
	{
		if (j == currentIndex || j == lastIndex || chosenIndices.count(j) != 0)
			continue;	

		angle = FastTracking::angle(inputPoints[j],inputPoints[lastIndex],inputPoints[currentIndex]);
		if (abs(angle) < 0.15f)
		{
			debugVector.push_back(new DebugArrow(inputPoints[currentIndex],inputPoints[j],lineColor,2));
			if (addNextVertex(j,currentIndex,chosenIndices,inputPoints,itCount,debugVector,lineColor))
			{	
				chosenIndices.insert(j);
				return true;
			}
		}
	}
	return false;
}

static bool validateSquare(vector<Point2i> & inputPoints, vector<Point2i> & squarePoints, Point2i patternCenter, Size2i patternSize, vector<Drawable*> & debugVector)
{
	LOGD(LOGTAG_QR,"Validating square,points=%d",inputPoints.size());
	if (inputPoints.size() < 4 || inputPoints.size() > 20)
	{
		return false;
	}
	vector<Point2i> testPoints = inputPoints;

	/*float avgLength = 0;
	for (int i=0;i<inputPoints.size();i++)
	{
		for (int j=0;j<inputPoints.size();j++)
		{
			if (i==j) continue;
			avgLength += sqrtf(GetSquaredDistance(inputPoints[i],inputPoints[j]));

		}
	}
	int sampleSize = inputPoints.size() * (inputPoints.size()-1);
	avgLength = idiv(avgLength,sampleSize);
	
	float deviation = 0;
	for (int i=0;i<inputPoints.size();i++)
	{
		for (int j=0;j<inputPoints.size();j++)
		{
			if (i==j) continue;
			deviation += powf(avgLength - sqrtf(GetSquaredDistance(inputPoints[i],inputPoints[j])),2);

		}
	}
	deviation = idiv(deviation,sampleSize);
	deviation = sqrt(deviation);

	if (deviation/avgLength > 0.5f)
	{
		LOGD(LOGTAG_QRFAST,"Deviation %f is too high");
		return false;
	}
*/

	int currentIndex = 0;
	float minDistance = min(((float)patternSize.height * 0.9f),((float)patternSize.width * 0.9f));
	minDistance = powf(minDistance,2.0f);
	set<int> chosenIndices;
	chosenIndices.insert(0);
	
	Scalar friendlyColors[]  = {Colors::Red,Colors::Orange,Colors::Fuchsia,Colors::Lime, Colors::SkyBlue,Colors::Aqua};
	for (int i=1;i<inputPoints.size();i++)
	{
		LOGD(LOGTAG_QRFAST,"Testing point(%d,%d)",inputPoints[i].x,inputPoints[i].y);
		/*if (GetSquaredDistance(inputPoints[0],inputPoints[i]) > minDistance)
			continue;*/
		Scalar lineColor = friendlyColors[i%5];
		debugVector.push_back(new DebugArrow(inputPoints[0],inputPoints[i],lineColor,2));
		if (addNextVertex(i,0,chosenIndices,inputPoints,40,debugVector,lineColor))
		{
			chosenIndices.insert(i);
			break;
		}
	}

	LOGD(LOGTAG_QR,"Index set size = %d", chosenIndices.size());

	set<int>::iterator it = chosenIndices.begin();

	squarePoints.clear();
	for (;it != chosenIndices.end();it++)
	{
		LOGD(LOGTAG_QRFAST,"Square point (%d,%d)",inputPoints.at(*it).x,inputPoints.at(*it).y);
		squarePoints.push_back(inputPoints.at(*it));
	}

	/*while (chosenIndices.size() < 4)
	{
		for (int i=0;i<inputPoints.size();i++)
		{
			if (chosenIndices.count(i) != 0)
				continue;
			float distanceToLast = GetSquaredDistance(inputPoints[currentIndex],inputPoints[i-1]);
			if (sqrtf(distanceToLast) < minDistance)
			{
				LOGV(LOGTAG_QRFAST,"Point is too close to last point");
				continue;
			}

			

		}
	}*/

	//float minAngleDiff = 10.0f * (PI/180.0f);
	//
	//multimap<int,Point2i> parallelCountMap;
	//float minDistance = min(((float)patternSize.height * 0.9f),((float)patternSize.width * 0.9f));

	//for (int i=0;i<inputPoints.size();i++)
	//{
	//	
	//	/*	float distanceToLast = GetSquaredDistance(inputPoints[i],inputPoints[i-1]);
	//		if (sqrtf(distanceToLast) < minDistance)
	//		{
	//			LOGV(LOGTAG_QRFAST,"Point is too close to last point");
	//			continue;
	//		}
	//	}*/
	//	int parallelCount = 0;
	//	Point2i point = inputPoints[i];
	//	Point2f endPoint = Point2f(point.x - patternCenter.x, point.y - patternCenter.y);
	//	float angleToHorizontal = atanf(endPoint.y/endPoint.x);
	//	for (int j=0;j<inputPoints.size();j++)
	//	{
	//		if (i==j) continue;

	//		Point2i point_inner = inputPoints[j];
	//		endPoint = Point2f(point_inner.x - patternCenter.x, point_inner.y - patternCenter.y);
	//		float angleToHorizontal_inner = atanf(endPoint.y/endPoint.x);

	//		if (abs(angleToHorizontal_inner-angleToHorizontal) < minAngleDiff ||
	//			abs(abs(angleToHorizontal_inner-angleToHorizontal) - PI) < minAngleDiff)
	//		{
	//			parallelCount++;
	//		}	
	//	}
	//	parallelCountMap.insert(pair<int,Point2i>(parallelCount,point));
	//}

	//
	//pair<multimap<int,Point2i>::iterator,multimap<int,Point2i>::iterator> itPair = parallelCountMap.equal_range(1);
	//	
	//for (; itPair.first != itPair.second;itPair.first++)
	//{
	//	squarePoints.push_back((*itPair.first).second);
	//	if (squarePoints.size() == 4)
	//		return true;
	//}

	//for (map<float,Point2i>::iterator it = anglePointMap.begin(); it != anglePointMap.end(); it++)
	//{
	//	squarePoints.push_back((*it).second);
	//	if (squarePoints.size() == 4)
	//		return true;
	//}

	if (squarePoints.size() == 4)
			return true;
	return false;
}

void FastQRFinder::CheckAlignmentPattern(Mat & img, Point2i center, Size2f patternSize, vector<Point2i> & patternPoints, vector<Drawable*> & debugVector)
{	
	struct timespec startTotal,endTotal;
	SET_TIME(&startTotal);
	
	int apSearchSize = config->GetIntegerParameter("K-NN_AP");
	int threshold = config->GetParameter("FastThresh");
	bool nonMaxSuppress = config->GetBooleanParameter("NonMaxSuppress");
	int debugLevel = (int)config->GetParameter("FASTAPDebug");	
	int searchIterations = config->GetParameter("MaxThreshCount");
	int scoreSearchSize = config->GetParameter("MaxThreshSize_AP");
	float maxThreshScale = config->GetFloatParameter("MaxThreshScale");	
	bool subPix = config->GetBooleanParameter("UseSubPix");

	Rect fastRect = Rect(center.x-(patternSize.width/2.0f),center.y-(patternSize.height/2.0f),patternSize.width, patternSize.height);

	if (debugLevel == 1)
		debugVector.push_back(new DebugRectangle(fastRect,Colors::Azure,1));

	vector<KeyPoint> keypoints;
	FastWindow(img,keypoints,fastRect, threshold, nonMaxSuppress);
	LOGV(LOGTAG_QRFAST,"Fast_AP: %d points found",keypoints.size());

	//Draw points for debug view	
	if (debugLevel == -4)
	{
		for (int i=0;i<keypoints.size();i++)
		{
			Point2i drawPoint = Point2i(keypoints[i].pt.x,keypoints[i].pt.y);
			debugVector.push_back(new DebugCircle(drawPoint,1,Colors::Lime,-1));
		}
	}

	LOGV(LOGTAG_QRFAST,"MaxThresh, size=%d",scoreSearchSize);
	//Size2i((int)ceilf((float)patternSize.width*maxThreshScale),(int)ceilf((float)patternSize.height*maxThreshScale)); 
	Size2i searchSize = Size2i(scoreSearchSize,scoreSearchSize);
	vector<Point2i> insideCornerVector, outsideCornersVector;
	MapBasedSuppression(keypoints,outsideCornersVector,insideCornerVector,searchSize,searchIterations);
		

	//Draw points for debug view	
	if (debugLevel == -1 || debugLevel == -3)
	{
		for (int i=0;i<outsideCornersVector.size();i++)
		{
			debugVector.push_back(new DebugCircle(outsideCornersVector[i],3,Colors::Red,-1));
		}
	}
	if (debugLevel == -1 || debugLevel == -2)
	{
		for (int i=0;i<insideCornerVector.size();i++)
		{	
			debugVector.push_back(new DebugCircle(insideCornerVector[i],3,Colors::Gold,-1));		
		}
	}

	LOGV(LOGTAG_QRFAST,"Inner=%d,Outer=%d",insideCornerVector.size(),outsideCornersVector.size());

	//Not enough corners, exit
	if (insideCornerVector.size() < 4)
	{
		if (debugLevel == 1)
			debugVector.push_back(new DebugCircle(center,13,Colors::Purple,1));	
		return;
	}

	if (debugLevel == 1) 
	{	
		debugVector.push_back(new DebugCircle(center,3,Colors::Lime,2));	
	}

	float maxCenterSize = max((float)patternSize.width * 0.5f, (float)patternSize.height*0.5f);
	vector<pair<int,Point2i> > outerPoints;

	if (outsideCornersVector.size() > 0)
	{
		//Find the FP center
		Mat outsideCornerMatrix;
		flann::Index * outerIndex;
		BuildFLANNIndex(outerIndex,1,outsideCornersVector,outsideCornerMatrix);
		outerPoints =  getClosestInSet(center,outsideCornerMatrix,outerIndex,3,64, maxCenterSize);
		
	}
	if (outerPoints.empty())
	{
		outerPoints.push_back(pair<int,Point2i>(0,center));
	}
	

	for (vector<pair<int,Point2i> >::iterator centerIt = outerPoints.begin(); centerIt != outerPoints.end(); centerIt++)
	{
		center = (*centerIt).second;

		if (debugLevel == 1) 
		{	
			debugVector.push_back(new DebugCircle(center,1,Colors::Aqua,-1));	
			debugVector.push_back(new DebugCircle(center,4,Colors::Aqua,1));	
		}


		Mat cornerMatrix;
		flann::Index * cornerIndex;
		BuildFLANNIndex(cornerIndex,1,insideCornerVector,cornerMatrix);
		vector<pair<int,Point2i> > testPoints = getClosestInSet(center,cornerMatrix,cornerIndex,apSearchSize,64);

		if (!testPoints.empty())
		{
			
			if (debugLevel == 1)
				debugVector.push_back(new DebugCircle(testPoints[0].second,8,Colors::Red,2,true));

			for (vector<pair<int,Point2i> >::iterator it = testPoints.begin(); it != testPoints.end(); it++)
			{
				Point2i drawPoint = (*it).second;
				if (debugLevel == 2) 
					debugVector.push_back(new DebugCircle(drawPoint,3,Colors::Turquoise,-1));									
			}

			vector<Point2i> pointsOnly;
			for (int i=0;i<testPoints.size();i++)
			{
				pointsOnly.push_back(testPoints.at(i).second);
			}

			if (validateSquare(pointsOnly,patternPoints, center, patternSize, debugVector))
			{
				break;
			}
		}
	}

	if (subPix && patternPoints.size() >= 4)
	{
		int subPixWindowSize = config->GetIntegerParameter("SubPixWindow");
		int subPixDeadZoneSize = config->GetIntegerParameter("SubPixDeadZone");
		int subPixMaxCount = config->GetIntegerParameter("SubPixMaxCount");
		double subPixEpsilon = (double)(config->GetFloatParameter("SubPixEpsilon"));

		vector<Point2f> floatCorners;
		for (int i=0;i<patternPoints.size();i++)
		{
			if (debugLevel == 1)
			{
				debugVector.push_back(new DebugCircle(patternPoints[i],8,Colors::Lime,1,true));
			}
			floatCorners.push_back(Point2f(patternPoints[i].x,patternPoints[i].y));
		}

		cornerSubPix(img,floatCorners,Size2i(subPixWindowSize,subPixWindowSize),Size2i(subPixDeadZoneSize,subPixDeadZoneSize),TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, subPixMaxCount, subPixEpsilon));

		patternPoints.clear();
		for (int i=0;i<floatCorners.size();i++)
		{
			Point2i refinedPoint = Point2i((int)round(floatCorners[i].x),(int)round(floatCorners[i].y));
			if (debugLevel == 1)
			{
				debugVector.push_back(new DebugCircle(refinedPoint,8,Colors::Fuchsia,1,true));
			}
			patternPoints.push_back(refinedPoint);
		}
	}


	//if (debugLevel > 0)
	//{
	//	config->SetLabelValue("NumPoints",(float)insideCornerVector.size());		
	//}
	SET_TIME(&endTotal);
	LOG_TIME_PRECISE("AlignCheck",startTotal,endTotal);
}


vector<pair<int,Point2i> > FastQRFinder::getClosestInSet(Point2i point, Mat & pointMatrix, flann::Index * innerPointIndex, int searchSize,int searchParams, float maxDistance)
{
	Mat indexMatrix = Mat::ones(1, searchSize, CV_32S) * -1;
	Mat distanceMatrix = Mat::zeros(1, searchSize, CV_32F);	

	
	float searchPointData[] = {(float)point.x,(float)point.y};

	Mat searchPoint = Mat(1,2,CV_32F,searchPointData);	


	innerPointIndex->knnSearch(searchPoint,indexMatrix,distanceMatrix,searchSize,flann::SearchParams(searchParams));

	float * rowPtr;
	int * indexPtr = indexMatrix.ptr<int>(0);
	float * distPtr = distanceMatrix.ptr<float>(0);

	vector<pair<int,Point2i> > pointVector;
	for (int i=0;i<searchSize;i++)
	{
		if (indexPtr[i] < 0)
			break;

		rowPtr = pointMatrix.ptr<float>(indexPtr[i]);
		float distance = distPtr[i];
		if (maxDistance > 0 && distance >= maxDistance)
			continue;

		Point2i point = Point2i((int)(rowPtr[0]),(int)(rowPtr[1]));
		if (point.x == 0 && point.y == 0)
			break;
		pointVector.push_back(pair<int,Point2i>((int)distance,point));
	}
	return pointVector;
}


void FastQRFinder::LocateFPCorners(Mat & img, FinderPattern * pattern, vector<Point2i> & corners_out, vector<Drawable*> & debugVector)
{
	struct timespec startTotal,endTotal;
	SET_TIME(&startTotal);

	//Get parameters from UI. Need to store as local variables because they are accessed many times per frame.
	int debugLevel = (int)config->GetParameter("FASTDebug");
	int searchParams = config->GetIntegerParameter("FlannSearchParams");
	bool useRadius = config->GetBooleanParameter("DoRadiusSearch");
	int flannIndexType = config->GetIntegerParameter("FlannIndexType");
	int numTrees = config->GetIntegerParameter("NumKDTrees");
	int threshold = config->GetParameter("FastThresh");
	int maxInnerPoints = config->GetIntegerParameter("MaxInnerPoints");
	bool nonMaxSuppress = config->GetBooleanParameter("NonMaxSuppress");
	bool subPix = config->GetBooleanParameter("UseSubPix");
	float maxCosine = config->GetFloatParameter("MaxCosine");
	//int outerMostSearchK = config->GetIntegerParameter("K-NN");


	//FinderPattern search area
	float searchRegion = pattern->size * 1.4f;  //Window needs to be bigger to account for diagonals
	Rect window = Rect(pattern->pt.x - searchRegion/2.0f, pattern->pt.y-searchRegion/2.0f, searchRegion,searchRegion);
	float flannRadius = pow(MIN(pattern->patternSize.width,pattern->patternSize.height)/2.0f,2);
	

	if (debugLevel == 1)
	{
		debugVector.push_back(new DebugRectangle(window,Colors::Azure,1));
	}
	//FAST corner detection
	vector<KeyPoint> keypoints;

	struct timespec start,end;

	SET_TIME(&start);
	FastWindow(img,keypoints,window, threshold,nonMaxSuppress);
	SET_TIME(&end);
	LOG_TIME_PRECISE("FAST",start,end);
	
	if ( debugLevel == -4)
	{
		for (int i=0;i<keypoints.size();i++)
		{	
			debugVector.push_back(new DebugCircle(Point2i(keypoints[i].pt.x,keypoints[i].pt.y),2,Colors::DeepSkyBlue,-1));
		}
	}
	
	
	vector<Point2i> insideCornerVector, outsideCornersVector;
	
	if (!nonMaxSuppress)
	{
		int scoreSearchSize = config->GetParameter("MaxThreshSize");
		int searchIterations = config->GetParameter("MaxThreshCount");
		MapBasedSuppression(keypoints,outsideCornersVector,insideCornerVector,Size2i(scoreSearchSize,scoreSearchSize),searchIterations);
	}
	else
	{
		for (int i=0;i<keypoints.size();i++)
		{	
			if (keypoints[i].angle > 180)
			{
				insideCornerVector.push_back(Point2i(keypoints[i].pt.x,keypoints[i].pt.y));
			}
			else
			{
				outsideCornersVector.push_back(Point2i(keypoints[i].pt.x,keypoints[i].pt.y));
			}
		}
	}


	//No features to track, so return
	if (outsideCornersVector.size() < 4 || insideCornerVector.size() < 4)
	{
		if (debugLevel > 0)
			debugVector.push_back(new DebugCircle(pattern->pt,12,Colors::DarkCyan,2));
		//LOGV(LOGTAG_QRFAST,"Only %d inner and %d outer corners found. Exiting",insideCornerVector.size(),outsideCornersVector.size());
		return;
	}
	//if (debugLevel > 0)LOGV(LOGTAG_QRFAST,"%d inner and %d outer corners found!",insideCornerVector.size(),outsideCornersVector.size());

	for (int i=0;i<outsideCornersVector.size();i++)
	{
		if (debugLevel == -2 || debugLevel == -1)
			debugVector.push_back(new DebugCircle(outsideCornersVector[i],3,Colors::Red,-1));
	}

	for (int i=0;i<insideCornerVector.size();i++)
	{	
		if ( debugLevel == -3 || debugLevel == -1)
		{
			debugVector.push_back(new DebugCircle(insideCornerVector[i],3,Colors::Gold,-1));
		}
	}

	
	flann::Index * outerPointIndex, * innerPointIndex;
	Mat outsideCornersMatrix, insideCornersMatrix;
	

	struct timespec indexStart,indexEnd;
	SET_TIME(&indexStart);
	BuildFLANNIndex(outerPointIndex, numTrees, outsideCornersVector,outsideCornersMatrix);
	BuildFLANNIndex(innerPointIndex, numTrees, insideCornerVector,insideCornersMatrix);
	SET_TIME(&indexEnd);
	
	int centerSearchK = config->GetIntegerParameter("NumCenterPoints");
	int innerSearchK = config->GetIntegerParameter("NumInnerCorners");
	int outerMostSearchK = config->GetIntegerParameter("NumOuterCorners");

	//Find K closest outside corners
	//LOGD(LOGTAG_QRFAST,"Finding points near pattern center(%d,%d)",pattern->pt.x,pattern->pt.y);
	vector<pair<int,Point2i> >  innerPoints = getClosestInSet(pattern->pt,insideCornersMatrix,innerPointIndex,innerSearchK,64);

	vector<Point2i> corners;
	corners.clear();

	for (int i=0;i < innerPoints.size(); i++)
	{
		float pointDistance = innerPoints.at(i).first;
		Point2i innerPoint = innerPoints.at(i).second;
		
		if (debugLevel == 3)
			debugVector.push_back(new DebugCircle(innerPoint,4,Colors::DarkOrange,2));
		
		//LOGV(LOGTAG_QRFAST,"Now finding point closest to (%d,%d)",innerPoint.x,innerPoint.y);
		vector<pair<int,Point2i> >  closePointsVector = getClosestInSet(innerPoint,outsideCornersMatrix,outerPointIndex,outerMostSearchK,searchParams);

		if (closePointsVector.size() < 2) //Not enough points. Shouldn't happen
		{
			if (debugLevel > 0)
				debugVector.push_back(new DebugCircle(pattern->pt,16,Colors::Red,5));
			LOGD(LOGTAG_QRFAST,"Not enough points for FP! Size=%d",closePointsVector.size());
			return;
		}

		Point2i bestPoint(-1,-1);
		float lowestCosine = maxCosine;
		//Select the outer point that forms straightest line, and is farther from pattern center than inner point
		for (vector<pair<int,Point2i> >::iterator closePointsIt = closePointsVector.begin(); closePointsIt != closePointsVector.end(); closePointsIt++)
		{			

			Point2i testPoint = (*closePointsIt).second;
			
			//LOGV(LOGTAG_QRFAST,"Now finding point closest to (%d,%d)",testPoint.x,testPoint.y);
			
			if (GetSquaredDistance(testPoint,pattern->pt) < pointDistance)
				continue;

			if (debugLevel == 2)
				debugVector.push_back(new DebugCircle(testPoint,4,Colors::Yellow,2));

			float cosine = FastTracking::angle(pattern->pt,testPoint,innerPoint);
			if (cosine < lowestCosine)
			{
				lowestCosine = cosine;
				bestPoint = testPoint;
			}
		}

		//Didn't find an appropriate point
		if (bestPoint.x <= 0 || bestPoint.y <= 0)
		{			
			if (debugLevel == 2)
				debugVector.push_back(new DebugCircle(pattern->pt,11,Colors::Tan,1));
			LOGD(LOGTAG_QRFAST,"Failed to find collinear point",closePointsVector.size());
			continue;
		}
		else
		{			
			corners.push_back(bestPoint);
			if (debugLevel == 1)
				debugVector.push_back(new DebugCircle(corners.back(),8,Colors::GreenYellow,1,true));
		}
				
	}

	if (corners.size() >= 4)
	{
		bool success = false;
		vector<Point2i> squarePoints;

		for (int i=0;i<corners.size();i++)
		{
			if (validateSquare(corners,squarePoints,pattern->pt,pattern->patternSize,debugVector))
			{
				success = true;
				break;
			}
		}

		if (success)
		{
			if (subPix)
			{
				int subPixWindowSize = config->GetIntegerParameter("SubPixWindow");
				int subPixDeadZoneSize = config->GetIntegerParameter("SubPixDeadZone");
				int subPixMaxCount = config->GetIntegerParameter("SubPixMaxCount");
				double subPixEpsilon = (double)(config->GetFloatParameter("SubPixEpsilon"));

				//LOGD(LOGTAG_QRFAST,"Finding corner sub pixels for %d corners", corners.size());
				vector<Point2f> floatCorners;
				for (int i=0;i<squarePoints.size();i++)
				{
					floatCorners.push_back(Point2f(squarePoints[i].x,squarePoints[i].y));
				}
				struct timespec subCornerStart,subCornerEnd;
				SET_TIME(&subCornerStart);

				cornerSubPix(img,floatCorners,Size2i(subPixWindowSize,subPixWindowSize),Size2i(subPixDeadZoneSize,subPixDeadZoneSize),TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, subPixMaxCount, subPixEpsilon));
				SET_TIME(&subCornerEnd);
				LOG_TIME_PRECISE("Corner sub pixels",subCornerStart,subCornerEnd);

				for (int i=0;i<floatCorners.size();i++)
				{
					Point2i refinedPoint = Point2i((int)round(floatCorners[i].x),(int)round(floatCorners[i].y));
					if (debugLevel == 1)
					{
						debugVector.push_back(new DebugCircle(refinedPoint,8,Colors::Fuchsia,1,true));
					}
					corners_out.push_back(refinedPoint);
				}
			}
			else
				corners_out = squarePoints;
		}	
		else
			corners_out = corners;
	}
	else
		corners_out = corners;


}

#ifdef lolcats

void FastQRFinder::LocateFPCorners3(Mat & img, FinderPattern * pattern, vector<Point2i> & corners, vector<Drawable*> & debugVector)
{
	struct timespec startTotal,endTotal;
	SET_TIME(&startTotal);

	//Get parameters from UI. Need to store as local variables because they are accessed many times per frame.
	int debugLevel = (int)config->GetParameter("FASTDebug");
	int searchParams = config->GetIntegerParameter("FlannSearchParams");
	bool useRadius = config->GetBooleanParameter("DoRadiusSearch");
	int flannIndexType = config->GetIntegerParameter("FlannIndexType");
	int numTrees = config->GetIntegerParameter("NumKDTrees");
	int threshold = config->GetParameter("FastThresh");
	int maxInnerPoints = config->GetIntegerParameter("MaxInnerPoints");
	bool nonMaxSuppress = config->GetBooleanParameter("NonMaxSuppress");
	float maxCosine = config->GetFloatParameter("MaxCosine");
	int outerMostSearchK = config->GetIntegerParameter("K-NN");


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
	
	
	vector<Point2i> insideCornerVector, outsideCornersVector;

	//If didn't do nonmax during FAST, perform here using maps

	int scoreSearchSize = config->GetParameter("MaxThreshSize");
	int searchIterations = config->GetParameter("MaxThreshCount");
	MapBasedSuppression(keypoints,outsideCornersVector,insideCornerVector,Size2i(scoreSearchSize,scoreSearchSize),searchIterations);


	//No features to track, so return
	if (outsideCornersVector.size() < 4 || insideCornerVector.size() < 4)
	{
		if (debugLevel > 0)
			debugVector.push_back(new DebugCircle(pattern->pt,12,Colors::DarkCyan,2));
		//LOGV(LOGTAG_QRFAST,"Only %d inner and %d outer corners found. Exiting",insideCornerVector.size(),outsideCornersVector.size());
		return;
	}
	//if (debugLevel > 0)LOGV(LOGTAG_QRFAST,"%d inner and %d outer corners found!",insideCornerVector.size(),outsideCornersVector.size());

	for (int i=0;i<outsideCornersVector.size();i++)
	{
		if (debugLevel == -2 || debugLevel == -1)
			debugVector.push_back(new DebugCircle(outsideCornersVector[i],3,Colors::Red,-1));
	}

	for (int i=0;i<insideCornerVector.size();i++)
	{	
		if ( debugLevel == -3 || debugLevel == -1)
		{
			debugVector.push_back(new DebugCircle(insideCornerVector[i],3,Colors::Gold,-1));
		}
	}

	
	flann::Index * outerPointIndex, * innerPointIndex;
	Mat outsideCornersMatrix, insideCornersMatrix;

	int searchSize = 4;

	float objPointData[] = {(float)pattern->pt.x, (float)pattern->pt.y};
	Mat patternCenter = Mat(1,2,CV_32F,objPointData);	


	struct timespec indexStart,indexEnd;
	SET_TIME(&indexStart);
	BuildFLANNIndex(outerPointIndex, numTrees, outsideCornersVector,outsideCornersMatrix);
	BuildFLANNIndex(innerPointIndex, numTrees, insideCornerVector,insideCornersMatrix);
	SET_TIME(&indexEnd);
	//LOG_TIME_PRECISE("Building FLANN indices",indexStart,indexEnd);

	
	Mat indexMatrix = Mat::ones(1, searchSize, CV_32S) * -1;
	Mat distanceMatrix = Mat::zeros(1, searchSize, CV_32F);	

	//Find 4 closest outside corners
	struct timespec flannStart, flannEnd;
	SET_TIME(&flannStart);
	outerPointIndex->knnSearch(patternCenter, indexMatrix, distanceMatrix, searchSize, cv::flann::SearchParams(searchParams));
	SET_TIME(&flannEnd);
	//LOG_TIME_PRECISE("First FLANN search",flannStart,flannEnd);
	
	int * indexPtr = indexMatrix.ptr<int>(0);
	float * distPtr = distanceMatrix.ptr<float>(0);
	float * rowPtr;
	vector<Point2i> centerPointsVector;
	for (int i=0;i<searchSize;i++)
	{		
		rowPtr = outsideCornersMatrix.ptr<float>(indexPtr[i]);
		float distance = distPtr[i];
		Point2i centerPoint = Point2i((int)(rowPtr[0]),(int)(rowPtr[1]));
		centerPointsVector.push_back(centerPoint);	
	}

	for (int i=0;i < searchSize; i++)
	{
		float distance = distPtr[i];
		Point2i centerPoint = centerPointsVector.at(i);
		
		if (debugLevel == 4)
			debugVector.push_back(new DebugCircle(centerPoint,4,Colors::Red,2));

		LOGV(LOGTAG_QRFAST,"Finding point closest to (%d,%d)",centerPoint.x,centerPoint.y);
		Point2i innerPoint = getClosestInSet(centerPoint,insideCornersMatrix,innerPointIndex,1,searchParams)[0];
		
		if (debugLevel == 3)
			debugVector.push_back(new DebugCircle(innerPoint,4,Colors::DarkOrange,2));
		
		LOGV(LOGTAG_QRFAST,"Now finding point closest to (%d,%d)",innerPoint.x,innerPoint.y);
		vector<Point2i> closePointsVector = getClosestInSet(innerPoint,outsideCornersMatrix,outerPointIndex,outerMostSearchK,searchParams);

		if (closePointsVector.size() < 2) //Not enough points. Shouldn't happen
		{
			if (debugLevel > 0)
				debugVector.push_back(new DebugCircle(pattern->pt,16,Colors::Red,5));
			LOGD(LOGTAG_QRFAST,"Not enough points for FP! Size=%d",closePointsVector.size());
			return;
		}

		Point2i bestPoint(-1,-1);
		float lowestCosine = maxCosine;
		//Select the outer point
		for (vector<Point2i>::iterator closePointsIt = closePointsVector.begin(); closePointsIt != closePointsVector.end(); closePointsIt++)
		{			
			Point2i testPoint = (*closePointsIt);	
			
			//Test that the point is not one of the center points
			bool isCenter = false;
			for (int j=0;j < centerPointsVector.size(); j++)
			{
				if (testPoint == centerPointsVector[j])
					isCenter = true;
			}
			if (isCenter) continue;

			if (debugLevel == 2)
				debugVector.push_back(new DebugCircle(testPoint,4,Colors::Yellow,2));

			float cosine = FastTracking::angle(centerPoint,testPoint,innerPoint);
			if (cosine < lowestCosine)
			{
				lowestCosine = cosine;
				bestPoint = testPoint;
			}
		}

		//Didn't find an appropriate point
		if (bestPoint.x == -1 || bestPoint.y == -1)
		{			
			if (debugLevel == 2)
				debugVector.push_back(new DebugCircle(pattern->pt,11,Colors::Tan,1));
			LOGD(LOGTAG_QRFAST,"Failed to find collinear point",closePointsVector.size());
			return;
		}
		else
		{
			corners.push_back(bestPoint);
			if (debugLevel == 1)
				debugVector.push_back(new DebugCircle(corners.back(),6,Colors::GreenYellow,2));
		}
				
	}

}


void FastQRFinder::LocateFPCorners2(Mat & img, FinderPattern * pattern, vector<Point2i> & corners, vector<Drawable*> & debugVector)
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
	float maxCosine = config->GetFloatParameter("MaxCosine");


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
		if (debugLevel > 2 || debugLevel == -2)
			debugVector.push_back(new DebugCircle(outsideCornersVector[i],3,Colors::Red,-1));
	}

	for (int i=0;i<insideCornerVector.size();i++)
	{	
		if (debugLevel > 2 || debugLevel == -2)
		{
			debugVector.push_back(new DebugCircle(insideCornerVector[i],3,Colors::Gold,-1));
		}
	}
	//Sort inner points by distance to pattern center, closest first
	sort(insideCornerVector.begin(),insideCornerVector.end(),PointCompare_Ascending(pattern->pt));

	
	//Sort outer points by distance to pattern center, closest first
	//sort(outsideCornersVector.begin(),outsideCornersVector.end(),PointCompare_Ascending(pattern->pt));
	

	//Test keypoints for validity. If this test fails, then return.
	float minPointRadius = 0;
	if (!keyPointTest(outsideCornersVector,pattern,maxInnerPoints,minPointRadius))
	{
		if (debugLevel > 1)
		{
			minPointRadius = sqrt(minPointRadius);
			debugVector.push_back(new DebugCircle(pattern->pt,minPointRadius,Colors::DarkViolet,1));
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

		//float MaxCosine = -0.8f;
		float lowestCosine = maxCosine;

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

		if (debugLevel == 4)
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
	processFinderPatternResults2(resultVector,pattern->pt, sortedVector,debugVector,debugLevel);
	SET_TIME(&end);
	LOG_TIME_PRECISE("FP Corner Analysis",start,end);

	for (int i=0;i<4 && i < sortedVector.size();i++)
	{
		corners.push_back(sortedVector[i].pt0);
	}
	
	
	return;
}
#endif