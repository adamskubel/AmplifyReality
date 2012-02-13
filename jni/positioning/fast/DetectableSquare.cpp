#include "FastQRFinder.hpp"


DetectableSquare::DetectableSquare()
{
	avgDistance = 0;
	isClockwise = false;
}

int DetectableSquare::NumPoints()
{
	return 4;
}

float DetectableSquare::AddNextPoint(Point2i selectedPoint)
{
	pointVector.push_back(selectedPoint);
}

static Point2i getRegion(Point2i point)
{
	Size2i regionSize(20,20);
	return 	Point2i((int)round(point.x/regionSize.width),(int)round(point.y/regionSize.height));

}

bool DetectableSquare::validateSquare(map<int,multimap<int,Point2i>*> & regionMap)
{
	vector<Point2i> regionBoundary;
	
	map<int,multimap<int,Point2i>*>::iterator xIterator, xIteratorStart,xIteratorEnd;
	pair<multimap<int,Point2i>::iterator,multimap<int,Point2i>::iterator> yIteratorRange;
	multimap<int,Point2i>::iterator yIterator;

	Point2i lowerLimit, upperLimit;

	for (int i=0;i<4;i++)
	{
		Point2i regionPoint = getRegion(pointVector[i]);
		MAX(upperLimit.x,regionPoint.x);
		MAX(upperLimit.y,regionPoint.y);

		MIN(lowerLimit.x, regionPoint.x);
		MIN(lowerLimit.y, regionPoint.y);
	}

	xIteratorStart = regionMap.lower_bound(lowerLimit.x);
	xIteratorEnd = regionMap.upper_bound(upperLimit.x);



	for (xIterator = xIteratorStart;xIterator != xIteratorEnd; xIterator++)
	{

	}


}



float DetectableSquare::ChooseNextPoint(vector<Point2i> & options, Point2i & selectedPoint, map<int,multimap<int,Point2i>*> & regionMap)
{
	vector<Point2i>::iterator pointIterator = options.begin();

	//Mat xCoords(1,options.size(),CV_32FC1);
	//Mat yCoords(1,options.size(),CV_32FC1);
	//Mat distances;
	//xCoords.push_back((*pointIterator).x);
	//yCoords.push_back((*pointIterator).y);

	float result = 0;

	//Should already have one point before this is called
	if (pointVector.size() == 0)
		result = -1;

	//Second point.
	//Choose first point with reasonable distance
	else if (pointVector.size() == 1)
	{

		for (;pointIterator != options.end(); pointIterator++)
		{
			float distance = GetPointDistance(pointVector[0],*pointIterator);
			if (distance > minPointSpacing)
			{
				avgDistance = distance;
				distanceError = avgDistance * 0.3f;
				pointVector.push_back(*pointIterator);
				result = 1;
				break;
			}		
		}
	}

	//Third point
	//Choose point that forms appropriate angle and has reasonable distance
	else if (pointVector.size() == 2)
	{			
		for (;pointIterator != options.end(); pointIterator++)
		{
			Point2i testPoint = *pointIterator;
			if ( DistanceTest(pointVector.back(),testPoint))
			{
				float cosine = FastTracking::angle(pointVector[0],testPoint,pointVector[1]);
				if (cosine < maxCosine)
				{
					isClockwise = IsClockWise(pointVector[0],testPoint,pointVector[1]);
					pointVector.push_back(testPoint);
					result = 1;
					break;
				}
			}		
		}
	}
	//Fourth point. 
	//Choose point that forms appropriate angles, and will allow a square to be formed.
	else if (pointVector.size() == 3)
	{			
		for (;pointIterator != options.end(); pointIterator++)
		{
			Point2i testPoint = *pointIterator;
			if ( GetPointDistance(pointVector.back(),testPoint) > minPointSpacing)
			{
				if (IsClockWise(pointVector[1],testPoint,pointVector[2]) == isClockwise)
				{
					float cosine = FastTracking::angle(pointVector[1],testPoint,pointVector[2]);
					if (cosine < maxCosine)
					{
						//Check if closing contour will form a square. First point [0], is the test point now.
						if ( DistanceTest(testPoint,pointVector[0]))
						{
							if (IsClockWise(pointVector[2],pointVector[0],testPoint) == isClockwise)
							{
								float cosine = FastTracking::angle(pointVector[2],pointVector[0],testPoint);
								if (cosine < maxCosine)
								{
									if (validateSquare(regionMap))
									{
										pointVector.push_back(testPoint);
										result = 1;
										break;
									}
								}
							}
						}	
					}
				}
			}		
		}
	}

	if (result > 0)
		selectedPoint = pointVector.back();

	return result;
}

bool DetectableSquare::DistanceTest(Point2i p0, Point2i p1)
{
	if (avgDistance == 0)
	{
		return GetPointDistance(p0,p1) > minPointSpacing;
	}
	else
	{
		return abs(GetPointDistance(p0,p1) - avgDistance) < distanceError;
	}
}

bool DetectableSquare::IsComplete()
{
	return (pointVector.size() == 4);
}

void DetectableSquare::Draw(Mat * rgbaImage)
{
	DebugPoly(pointVector,Colors::Aqua,2).Draw(rgbaImage);
}