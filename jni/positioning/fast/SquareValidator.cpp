#include "SquareValidator.hpp"

#define SGN(a) (a == 0 ? 0 : ((a < 0) ? -1 : 1))

SquareValidator::SquareValidator()
{
	minDistance = INT_MIN;
	maxDistance = INT_MAX;
}

void SquareValidator::SetParameters(int _debugLevel, float _maxAngle)
{
	maxAngle = _maxAngle;
	debugLevel = _debugLevel;
}

bool SquareValidator::ValidateSquare(vector<Point2i> _inputPoints, vector<Point2i> & squarePoints, int maxStartIndex, Point2i patternCenter, Size2i patternSize, vector<Drawable*> * _debugVector)
{
	debugVector = _debugVector;
	numIterations = 0;
	inputPoints = _inputPoints;

	//LOGV(LOGTAG_QR,"Validating square,points=%d,maxStartIndex=%d",inputPoints.size(),maxStartIndex);
	if (inputPoints.size() < 4 || inputPoints.size() > 20)
	{
		return false;
	}
	vector<Point2i> testPoints = inputPoints;

	if (inputPoints.size() - maxStartIndex < 4)
	{
		maxStartIndex = inputPoints.size() - 4;
		LOGD(LOGTAG_QRFAST,"Adjusting startIndex to %d",maxStartIndex);
	}

	if (patternSize.width > 0)
	{
		minDistance = min(((float)patternSize.height / 2.0f),((float)patternSize.width / 2.0f));
		minDistance = (int)round(powf(minDistance,2.0f));
		maxDistance = max(((float)patternSize.height * 1.2f),((float)patternSize.width * 1.2f));
		maxDistance = (int)round(powf(maxDistance,2.0f));
	}
	else
	{
		maxDistance = INT_MAX;
		minDistance = INT_MIN;
	}
	struct timespec start,end;

	SET_TIME(&start);
	numIterations = 0;
	bool success = false;
	Scalar friendlyColors[]  = {Colors::Red,Colors::Orange,Colors::Fuchsia,Colors::Lime, Colors::SkyBlue,Colors::Aqua};
	for (startIndex = 0;startIndex <= maxStartIndex; startIndex++)
	{

		chosenIndices.clear();
		chosenIndices.insert(startIndex);
		for (int i=startIndex+1;i<inputPoints.size();i++)
		{
			//LOGV(LOGTAG_QRFAST,"Testing point(%d,%d)",inputPoints[i].x,inputPoints[i].y);
			Scalar lineColor = friendlyColors[i%5];
			
			
			int distance = GetSquaredDistance(inputPoints[startIndex],inputPoints[i]);
			if (distance > minDistance && distance < maxDistance)
			{
				/*if (debugLevel > 0)
					debugVector->push_back(new DebugArrow(inputPoints[startIndex],inputPoints[i],lineColor,2));*/
				if (addNextVertex(i,startIndex,0,0,lineColor))
				{
					chosenIndices.insert(i);
					break;
				}
			}
			else
			{
				if (debugLevel > 1)
					debugVector->push_back(new DebugArrow(inputPoints[startIndex],inputPoints[i],Colors::Red,2));
				//LOGV(LOGTAG_QRFAST,"Too far: point(%d,%d)->(%d,%d). Dist=%d !E {%d,%d}",inputPoints[startIndex].x,inputPoints[startIndex].y,inputPoints[i].x,inputPoints[i].y,distance,minDistance,maxDistance);
			}
		}

		if (chosenIndices.size() == 4)
		{
			success = true;
			break;
		}

	}
	squarePoints.clear();
	for (set<int>::iterator it = chosenIndices.begin();it != chosenIndices.end();it++)
	{
		LOGV(LOGTAG_QRFAST,"Square point (%d,%d)",inputPoints.at(*it).x,inputPoints.at(*it).y);
		squarePoints.push_back(inputPoints.at(*it));
	}
	SET_TIME(&end);
	LOGD(LOGTAG_QRFAST,"Square recursion took %lf us for %d points and %d recursions, found %d square vertices, Finishing startIndex=%d",calc_time_double(start,end),inputPoints.size(),numIterations,chosenIndices.size(),startIndex);

	return success;
}

bool SquareValidator::addNextVertex(int currentIndex, int lastIndex, int lastSign, int depth, Scalar lineColor)
{
	if (debugLevel > 0)
		debugVector->push_back(new DebugArrow(inputPoints[lastIndex],inputPoints[currentIndex],lineColor,2));

	numIterations++;
	
	if (chosenIndices.size() == 4)
		return true;

	float angle = 0;
	int distance = 0;
	if (depth++ == 2)
	{
		//Check distance from first point
		distance = GetSquaredDistance(inputPoints[currentIndex],inputPoints[startIndex]);
		if (distance > minDistance && distance < maxDistance)
		{
			//Check if we reached the first point (0) if 3 vertices found
			angle = FastTracking::angle(inputPoints[startIndex],inputPoints[lastIndex],inputPoints[currentIndex]);
			if (abs(angle) < maxAngle)
			{
				if (debugLevel > 0)
				{
					/*debugVector->push_back(new DebugArrow(inputPoints[currentIndex],inputPoints[startIndex],lineColor,-1));
					debugVector->push_back(new DebugArrow(inputPoints[currentIndex],inputPoints[startIndex],lineColor,1));*/
					debugVector->push_back(new DebugLine(inputPoints[currentIndex],inputPoints[startIndex],Colors::Blue,2));
				}
				return true;
			}
			else //If not, then this isn't a square
			{
				return false;
			}
		}
	}
	else
	{
		for (int j=startIndex+1;j<inputPoints.size();j++)
		{
			if (j == currentIndex || j == lastIndex || chosenIndices.count(j) != 0)
				continue;	

			distance = GetSquaredDistance(inputPoints[j],inputPoints[currentIndex]);
			if (distance < minDistance || distance > maxDistance)
				continue;

			angle = FastTracking::angle(inputPoints[j],inputPoints[lastIndex],inputPoints[currentIndex]);
			
			if (abs(angle) < maxAngle)
			{
				int angleSign = SGN(angle);
				//If angle difference from 90 is greater than 5 degrees, check the sign. Must be opposite to proceed.
				if (angleSign == lastSign && abs(angle) >= 0.9f)
					continue;				
				
				if (addNextVertex(j,currentIndex,angleSign,depth,lineColor))
				{	
					chosenIndices.insert(j);
					return true; //True if this point ends in a square
				}
			}
		}
		return false; //Return false if all points are searched and no success found
	}
}
