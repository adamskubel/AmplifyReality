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

QRCode * FastQRFinder::FindQRCodes(Mat & img, vector<Drawable*> & debugVector)
{
	//Steps:
	//1. Divide the keypoints into regions. Regions are formed by a grid of set size. 
	//2. Score each region.
	//3. Select regions with the highest score
	//4. If more than one, combine
	//5. Score points in region

	float cellDimension = 100;
	Size2i cellSize(cellDimension,cellDimension);


	vector<KeyPoint> keypoints;
	//Do FAST
	
	struct timespec start,end;
	LOGD(LOGTAG_QRFAST,"Performing FAST detection");
	SET_TIME(&start)
	cv::FAST(img,keypoints,20,true);
	SET_TIME(&end)
	LOG_TIME("FAST",start,end);

	/*LOGD(LOGTAG_QRFAST,"FAST complete. %d points found.",keypoints.size());
	vector<KeyPoint>::iterator pointIterator = keypoints.begin();
	for (; pointIterator != keypoints.end(); pointIterator++)
	{
		float size = (fabs(((*pointIterator).response / 30.0f)) * 5.0f) + 5.0f;
		debugVector.push_back(new DebugCircle((*pointIterator).pt,size,((*pointIterator).response < 0.0f) ? Colors::Gold : Colors::Red));
	}*/

	//LOGD(LOGTAG_QRFAST,"Calling SURF");

	//vector<float>
	//cv::SURF()(img,Mat(),keypoints,descriptorVector,true);

	
	

	map<int,multimap<int,Point2f>*> regionMap;
	
	LOGD(LOGTAG_QRFAST,"Regioning points.");
	
	SET_TIME(&start)
	for (int i=0;i<keypoints.size();i++)
	{
		Point2f point = keypoints.at(i).pt;
		Point2i key = GetRegion(point,cellSize);
		map<int,multimap<int,Point2f>* >::iterator it = regionMap.find(key.x);
		if (it == regionMap.end())
		{
			multimap<int,Point2f> * newMultimap = new multimap<int,Point2f>();
			regionMap.insert(pair<int,multimap<int,Point2f>*>(key.x,newMultimap));
			newMultimap->insert(pair<int,Point2f>(key.x,point));
		}
		else
		{
			(*it).second->insert(pair<int,Point2f>(key.y,point));		
		}		

	}
	
	SET_TIME(&end)
	LOG_TIME("RegioningPoints",start,end);

	
	//LOGD(LOGTAG_QRFAST,"Regioning complete. Drawing points by region.");
	//Test this

#ifdef FAST_QR_DEBUGGING
	Scalar colorArray[5] = {Colors::Red,Colors::Blue,Colors::Green,Colors::Gold,Colors::Lime};
	SET_TIME(&start)
	int count =0;
	for (int x=0;x<=maxDim.width;x++)
	{
		for (int y=0;y<=maxDim.height;y++)
		{			
			Scalar regionColor = colorArray[(count++)%2];
			map<int,multimap<int,Point2f>*>::iterator it = regionMap.find(x);
			if (it != regionMap.end())
			{
				pair<multimap<int,Point2f>::iterator,multimap<int,Point2f>::iterator> innerItPair = (*it).second->equal_range(y);
				for (multimap<int,Point2f>::iterator innerIt = innerItPair.first; innerIt != innerItPair.second; innerIt++)
				{
					//LOGD(LOGTAG_QRFAST,"Region(%d,%d) has %d points",x,y,kpvec.size());
					Point2f pt = (*innerIt).second;
					debugVector.push_back(new DebugCircle(Point2i(round(pt.x),round(pt.y)),4,regionColor));					
				}
			}
		}
	}
#endif


	//Random 4-point contour search
	//Select point at random
	//Choose next point based on Rule_Dist
	//Choose next point based on Rule_Dist and Rule_Angle
	//Choose next point based on Rule_Dist and Rule_Angle and Rule_Angle_Distribution
	//Evaluate contour using Rule_Evaluate
	
	Size2i maxDim = Size2i((int)round(img.cols/cellSize.width),(int)round(img.rows/cellSize.height));

	

	//Declaring iterators
	map<int,multimap<int,Point2f>*>::iterator xIterator;
	pair<multimap<int,Point2f>::iterator,multimap<int,Point2f>::iterator> yIteratorRange;
	multimap<int,Point2f>::iterator yIterator;

	xIterator = regionMap.begin();
	
	//Find a random point
	int randX = rand() % regionMap.size();
	for (int i=0; i < randX; i++, xIterator++);
	multimap<int,Point2f> * yPointMap = (*xIterator).second;
	int randY = rand() %  yPointMap->size();
	yIterator = yPointMap->begin();
	for (int i=0;i<randY;i++, yIterator++);
	Point2i randPointKey = Point2i((*xIterator).first,(*yIterator).first);
	Point2f randPoint = (*yIterator).second;
	LOGD(LOGTAG_QRFAST,"Starting at random point(%d,%d)->(%f,%f)",randPointKey.x,randPointKey.y,randPoint.x,randPoint.y);

	//Look for contours from point
	
	
	//Initialize desired contour
	IDetectableContour * square = new DetectableSquare();

	//Try and build square
	for (int attempts = 0;attempts < 20; attempts++)
	{
		int regionDistance = 1;
		//Get nearby points
		for (int xIndex = randPointKey.x - regionDistance; xIndex < randPointKey.x + regionDistance; xIndex++)
		{
			xIterator = regionMap.find(xIndex);
			if (xIterator != regionMap.end())
			{
				for (int yIndex = randPointKey.y - regionDistance; yIndex < randPointKey.y + regionDistance; yIndex++)
				{
					if (yIndex == randPointKey.y && xIndex == randPointKey.x) continue; //Skip start region

					yIteratorRange = (*xIterator).second->equal_range(yIndex);
					
				//	for (yIterator = yIteratorRange.first; yIterator != yIteratorRange.second; yIterator++)
				//	{
					square->ChooseNextPoint(vector<Point2f>(yIteratorRange.first,yIteratorRange.second));
				//	}
				}
			}
		}
		xIterator = regionMap.find(randPointKey.x - regionDistance);
		
	}

	



	//Cleanup map
	LOGD(LOGTAG_QRFAST,"Cleaning up map");
	for (map<int,multimap<int,Point2f>*>::iterator deleteIt = regionMap.begin(); deleteIt != regionMap.end();deleteIt++)
	{
		delete (*deleteIt).second;
	}


	SET_TIME(&end)
	LOG_TIME("DrawingPoints",start,end);

	


}

Point2i FastQRFinder::GetRegion(Point2f point, Size2i regionSize)
{
	return Point2i((int)round(point.x/regionSize.width),(int)round(point.y/regionSize.height));
}


bool FastQRFinder::PointInRegion(Point2f point, Point2i cellPosition, Size2i regionSize)
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