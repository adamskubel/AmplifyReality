#include "QRFinder.hpp"


#define AP_DEBUG_ENABLED true



void QRFinder::FindAlignmentPattern(Mat & inputImg, QRCode * newCode, vector<Drawable*>& debugVector)
{
	minimumAlignmentPatternScore = config->GetIntegerParameter("MinimumAPScore");
	alignDebugLevel = config->GetIntegerParameter("AlignDebug");
	int verticalResolution = config->GetIntegerParameter("YResolution");
	float fastRegionScale = config->GetFloatParameter("APFastScale");
	
	Rect searchRegion;
	Point2i guess;
	if (newCode->GuessAlignmentPosition(guess,searchRegion))
	{
		if (alignDebugLevel > 1)
			debugVector.push_back(new DebugCircle(guess,12,Colors::SkyBlue,1,true));

		//Test guessed position with FAST directly before proceeding with edge-based search
		vector<Point2i> patternPoints;
		fastQRFinder->CheckAlignmentPattern(inputImg,guess,Size2f(searchRegion.width,searchRegion.height),patternPoints,debugVector);
		for (int i=0;i<patternPoints.size();i++)
		{
			if (alignDebugLevel > 0)
				debugVector.push_back(new DebugCircle(patternPoints.at(i),4,Colors::Fuchsia,2));
		}
		if (patternPoints.size() == 4)
		{
			if (alignDebugLevel > 0)
				debugVector.push_back(new DebugRectangle(searchRegion,Colors::MediumSlateBlue,1));
			newCode->SetAlignmentCorners(patternPoints);
			return;
		}

		LOGD(LOGTAG_QR,"SearchRegion[%d,%d,%d,%d]",searchRegion.x,searchRegion.y,searchRegion.width,searchRegion.height);
		ConstrainRectangle(edgeArray,searchRegion);
		LOGD(LOGTAG_QR,"ConstrainedSearchRegion[%d,%d,%d,%d]",searchRegion.x,searchRegion.y,searchRegion.width,searchRegion.height);
		if (alignDebugLevel > 0)
			debugVector.push_back(new DebugRectangle(searchRegion,Colors::Coral,1));
	}
	else
	{
		if (alignDebugLevel > 0)
			debugVector.push_back(new DebugRectangle(searchRegion,Colors::Red,1));
		if (alignDebugLevel > 1)
			debugVector.push_back(new DebugCircle(guess,12,Colors::SkyBlue,1,true));

		LOGD(LOGTAG_QR,"SearchRegion2[%d,%d,%d,%d]",searchRegion.x,searchRegion.y,searchRegion.width,searchRegion.height);
		ConstrainRectangle(edgeArray,searchRegion);
		LOGD(LOGTAG_QR,"ConstrainedSearchRegion2[%d,%d,%d,%d]",searchRegion.x,searchRegion.y,searchRegion.width,searchRegion.height);
	}	
	
	int finderPatternSize = round(newCode->getAvgPatternSize());

		
	FindEdgesClosed(inputImg,searchRegion,edgeArray,edgeThreshold,nonMaxEnabled,detectorSize,verticalResolution);
	

	int startY = searchRegion.y;
	int startX = searchRegion.x;
	int endY = startY + searchRegion.height;
	int endX = startX + searchRegion.width;


	for (int y = startY;y < endY; y += verticalResolution)
	{
		int patternWidths[4] = { 0 };
		int k = 0;
		patternWidths[0] = patternWidths[1] = patternWidths[2] = patternWidths[3] = 0;
		const short * rowPointer = edgeArray.ptr<short>(y);

		for (int x = startX;x<endX;x++)
		{
			short transition = rowPointer[x];

			if (k == 0) //Haven't found edge yet
			{
				if (transition > 0) //Pattern starts on dark->light
				{					
					k++;
				}
			}
			else if (k == 1) 
			{
				if (transition < 0) //Center of pattern starts
				{					
					++k;
				}
			}
			else if (k == 2) 
			{
				if (transition > 0) //Rightmost white pattern area
				{					
					++k;
				}
			}
			else if (k == 3) 
			{
				if (transition < 0) //End of pattern
				{					
					++k;
				}
			}

			//Increment once pattern is found
			if (k > 0)
			{
				++patternWidths[k-1];
			}

			if (k == 4)
			{
				if (CheckAlignmentRatios(patternWidths,finderPatternSize))
				{
					//Center based on initial detection
					int tempXCenter = (x - patternWidths[2]) - (patternWidths[1] / 2);
					int verticalPatternWidths[3] = {0,0,0};

					//Vertical center
					int tempYCenter = FindAlignmentCenterVertical(inputImg,searchRegion, tempXCenter, y,finderPatternSize,verticalPatternWidths,patternWidths, debugVector);

					if (tempYCenter > 0)
					{
						int horizontalPatternWidths[3] = {0,0,0};
						int xCenterTest = FindAlignmentCenterHorizontal(searchRegion, tempXCenter, tempYCenter, finderPatternSize, verticalPatternWidths, horizontalPatternWidths, debugVector); 

						if (xCenterTest > 0)
						{								
							tempXCenter = xCenterTest;					
							if (true)//CheckAlignmentDiagonals(inputImg,Point2i(tempXCenter,tempYCenter),verticalPatternWidths,horizontalPatternWidths,debugVector))
							{									
								float apXSize =  horizontalPatternWidths[2] + horizontalPatternWidths[1] + horizontalPatternWidths[0];
								float apYSize =  verticalPatternWidths[2] + verticalPatternWidths[1] + verticalPatternWidths[0];

								int alignPatternSize = MAX(apXSize,apYSize);
								
								vector<Point2i> patternPoints;								

								Size2f patternSizeRect = Size2f(apXSize * fastRegionScale,apYSize * fastRegionScale);
								fastQRFinder->CheckAlignmentPattern(inputImg,Point2i(tempXCenter,tempYCenter),patternSizeRect,patternPoints,debugVector);

								for (int i=0;i<patternPoints.size();i++)
								{
									if (alignDebugLevel > 0)
										debugVector.push_back(new DebugCircle(patternPoints.at(i),4,Colors::Lime,2));
								}

								if (patternPoints.size() == 4)
								{
									if (alignDebugLevel > -1)
										debugVector.push_back(new DebugCircle(Point2i(tempXCenter,tempYCenter),(alignPatternSize/2.0f),Colors::Blue,1));

									newCode->SetAlignmentCorners(patternPoints);
									/*newCode->alignmentPattern = Point2i(tempXCenter,tempYCenter);*/
									return;
								}
							}	
							else
							{			
								if (alignDebugLevel > 1)
									debugVector.push_back(new DebugCircle(Point2i(tempXCenter,tempYCenter),12, Colors::BlueViolet,1));

							}
						}
						else
						{
							if (alignDebugLevel > 2)
								debugVector.push_back(new DebugCircle(Point2i(tempXCenter,tempYCenter),12, Colors::SlateBlue,1));

						}
					}
					else
					{
						if (alignDebugLevel > 3)
						{
							if (tempYCenter == 0) //ratio fail
								debugVector.push_back(new DebugCircle(Point2i(tempXCenter,y),10,Colors::Aqua,1));
							else if (tempYCenter == -1)	//topcheck fail				
								debugVector.push_back(new DebugCircle(Point2i(tempXCenter,y),10,Colors::Orange,1));
							else if (tempYCenter == -2)	//bottomcheck fail							
								debugVector.push_back(new DebugCircle(Point2i(tempXCenter,y),10,Colors::Lime,1));
						}
					}

				}
				else
				{
					if (alignDebugLevel > 4)
						debugVector.push_back(new DebugCircle(Point2i(x,y),4,Colors::Maroon,1));
				}
				k = 2;
				patternWidths[0] = patternWidths[2];
				patternWidths[1] = patternWidths[3];
				patternWidths[2] = 0;
				patternWidths[3] = 0;
			}
		}
	}
}

int QRFinder::FindAlignmentCenterVertical(const Mat & image, Rect searchRegion, int x, int y, int finderPatternSize, 
	int * verticalWidths, int * horizontalWidths, vector<Drawable*> & debugVector)
{
	int debugCircleSize = 3;
	int debugThickness = 1;

	int k = 0;
	int i = y;
	verticalWidths[0] = verticalWidths[1] = verticalWidths[2] = 0;

	FindEdgesVerticalClosed(image,x);
	const short * rowPtr = verticalEdgeArray.ptr<short>(x);

	for (; i >= searchRegion.y; i --)
	{		
		int transition = -rowPtr[i]; //invert because we are moving up

		if (k == 0) //Haven't found edge yet
		{
			++verticalWidths[1];
			if (transition > 0) //dark to light
			{	
				if (AP_DEBUG_ENABLED && alignDebugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(x,i),debugCircleSize,Colors::Aquamarine,1));
				k++;
			}
		}
		else if (k == 1) 
		{
			++verticalWidths[0];
			if (transition < 0) //light->dark, end of pattern
			{
				if (AP_DEBUG_ENABLED && alignDebugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(x,i),debugCircleSize,Colors::SeaGreen,1));
				++k;
				break;
			}
		}
	}

	if (k != 2)
	{
		return -1;
	}

	i = y;
	k = 0;
	for (; i < (searchRegion.y + searchRegion.height); i++)
	{		
		int transition = rowPtr[i];

		if (k == 0) //Haven't found edge yet
		{
			++verticalWidths[1];
			if (transition > 0) //dark to light
			{	
				if (AP_DEBUG_ENABLED && alignDebugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(x,i),debugCircleSize,Colors::Yellow,1));
				k++;
			}
		}
		else if (k == 1) 
		{
			++verticalWidths[2];
			if (transition < 0) //light to dark, pattern end
			{
				if (AP_DEBUG_ENABLED && alignDebugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(x,i),debugCircleSize,Colors::Orange,1));
				++k;
				break;
			}
		}
	}

	if (k != 2)
	{
		return -2;
	}
	--verticalWidths[1];

	if (CheckAlignmentRatios(verticalWidths, horizontalWidths) == false)
	{
		//LOGV(LOGTAG_QR,"FindAlignmentCenterVertical F: verticalWidths= %d,%d,%d",verticalWidths[0],verticalWidths[1],verticalWidths[2]);
		//LOGV(LOGTAG_QR,"Failed ratio check for finding vertical center, FPSize=%d",finderPatternSize);
		return 0;
	}

	return (i - verticalWidths[2]) - (verticalWidths[1] / 2);
}

bool QRFinder::CheckAlignmentDiagonals(const Mat& image, Point2i center, int * verticalWidths, int * horizontalBw, vector<Drawable*> & debugVector)
{
	int xMin = center.x - (horizontalBw[0] + horizontalBw[1])/2;

	int xMax = center.x + (horizontalBw[2] + horizontalBw[1])/2;


	int  yMin = center.y - (verticalWidths[0] + verticalWidths[1])/2;
	int  yMax = center.y + (verticalWidths[2] + verticalWidths[1])/2;
		
	//Check upper Y
	short * rowPtr = edgeArray.ptr<short>(yMin);
	int j = xMin;
	
	for (; j < xMax && rowPtr[j] == 0; j++);
	if (j < xMax)
	{
		if (AP_DEBUG_ENABLED && alignDebugLevel > 1)
		{
			debugVector.push_back(new DebugLine(Point2i(xMin,yMin), Point2i(xMax,yMin),Colors::Red));		
		}
		return false;
	}

	if (AP_DEBUG_ENABLED && alignDebugLevel > 1)
		debugVector.push_back(new DebugLine(Point2i(xMin,yMin), Point2i(xMax,yMin),Colors::DeepSkyBlue));

	//Check lower Y
	rowPtr = edgeArray.ptr<short>(yMax);
	j = xMin;

	for (; j < xMax && rowPtr[j] == 0; j++);
	if (j < xMax)
	{
		if (AP_DEBUG_ENABLED && alignDebugLevel > 1)
			debugVector.push_back(new DebugRectangle(Rect(Point2i(xMin,yMax), Point2i(xMax,yMax)),Colors::DeepPink));
		return false;
	}
	
	if (AP_DEBUG_ENABLED && alignDebugLevel > 1)
		debugVector.push_back(new DebugRectangle(Rect(Point2i(xMin,yMax), Point2i(xMax,yMax)),Colors::DeepSkyBlue));

	return true;
}

int QRFinder::FindAlignmentCenterHorizontal(Rect searchRegion, int x, int y, int finderPatternSize, int * verticalWidths, int * horizontalWidths, vector<Drawable*> & debugVector)
{
	int debugCircleSize = 3;
	int debugThickness = 1;

	int k = 0;
	int j = x;
	
	const short * rowPtr = edgeArray.ptr<short>(y);

	for (; j >= searchRegion.x; j --)
	{		
		int transition = -rowPtr[j]; //invert because we are moving up

		if (k == 0) //Haven't found edge yet
		{
			++horizontalWidths[1];
			if (transition > 0) //dark to light
			{	
				if (AP_DEBUG_ENABLED && alignDebugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(j,y),debugCircleSize,Colors::Aquamarine,1));
				k++;
			}
		}
		else if (k == 1) 
		{
			++horizontalWidths[0];
			if (transition < 0) //light->dark, end of pattern
			{
				if (AP_DEBUG_ENABLED && alignDebugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(j,y),debugCircleSize,Colors::SeaGreen,1));
				++k;
				break;
			}
		}
	}

	if (k != 2)
	{
		return 0;
	}

	j = x;
	k = 0;

	for (; j < (searchRegion.x + searchRegion.width); j++)
	{		
		int transition = rowPtr[j];

		if (k == 0) //Haven't found edge yet
		{
			++horizontalWidths[1];
			if (transition > 0) //dark to light
			{	
				if (AP_DEBUG_ENABLED && alignDebugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(j,y),debugCircleSize,Colors::Yellow,1));
				k++;
			}
		}
		else if (k == 1) 
		{
			++horizontalWidths[2];
			if (transition < 0) //light to dark, pattern end
			{
				if (AP_DEBUG_ENABLED && alignDebugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(j,y),debugCircleSize,Colors::Orange,1));
				++k;
				break;
			}
		}
	}

	if (k != 2)
	{
		return 0;
	}

	if (CheckAlignmentRatios(horizontalWidths, verticalWidths) == false)
	{
		//LOGV(LOGTAG_QR,"FindAlignmentCenterVertical F: verticalWidths= %d,%d,%d",verticalWidths[0],verticalWidths[1],verticalWidths[2]);
		//LOGV(LOGTAG_QR,"Failed ratio check for finding vertical center, FPSize=%d",finderPatternSize);
		return 0;
	}

	return (j - horizontalWidths[2]) - (horizontalWidths[1] / 2);
}

bool QRFinder::CheckAlignmentRatios(int * newBw, int finderPatternSize)
{
	int alignmentPatternSum = 0;
	float moduleUnitSize = finderPatternSize/7.0f;
	for (int i = 0; i < 3; ++i)
	{
		if (newBw[i] == 0)
		{
			return false; 
		}
		alignmentPatternSum += newBw[i];	
	}

	if (alignmentPatternSum < 3)
	{
		return false; 
	}
	int alignmentPatternSize = (int)round((float)alignmentPatternSum / 3.0f);


	int variance	= alignmentPatternSize >> 2;
	int variance2	= alignmentPatternSize >> 1;

	int a = newBw[0];	int b = newBw[1];	int c = newBw[2];

	int symmetryScore = (abs(a - c) < variance2) * 100;

	int sizeScore = 		
		(abs(alignmentPatternSize - a) < variance2)*22 + 
		(abs(alignmentPatternSize - b) < variance2)*22 + 
		(abs(alignmentPatternSize - c) < variance2)*22; 

	int oldNewCompareScore = 50 * (abs(alignmentPatternSum - 3*moduleUnitSize) < (finderPatternSize >> 2));


	int score = (sizeScore + symmetryScore + oldNewCompareScore);	

	if (score > minimumAlignmentPatternScore)
	{				
		//LOGV(LOGTAG_QR,"CheckAlignRatios T: New= %d,%d,%d",newBw[0],newBw[1],newBw[2]);
		//LOGV(LOGTAG_QR,"Found alignment pattern! Scores were: Sym=%d,Size=%d,OldNewScore=%d < MinScore=%d, FPSize=%d",symmetryScore,sizeScore,oldNewCompareScore,minimumAlignmentPatternScore,finderPatternSize);
		return true;
	}
	else
	{
		//LOGV(LOGTAG_QR,"CheckAlignRatios F: New= %d,%d,%d",newBw[0],newBw[1],newBw[2]);
		//LOGV(LOGTAG_QR,"Scores: Sym=%d,Size=%d,OldNewScore=%d < MinScore=%d, FPSize=%d",symmetryScore,sizeScore,oldNewCompareScore,minimumAlignmentPatternScore,finderPatternSize);
		return false;
	}

}

bool QRFinder::CheckAlignmentRatios(int * newBw, int * previousBw, bool log)
{
	int alignmentPatternSum = 0, horizontalSum = 0;

	for (int i = 0; i < 3; ++i)
	{
		if (newBw[i] == 0)
		{
			if (log)
			{
				//LOGD(LOGTAG_QR,"Zero for width #%d",i);
			}
			return false; 
		}
		alignmentPatternSum += newBw[i];	

		horizontalSum += previousBw[i];	
	}

	if (alignmentPatternSum < 3)
	{
		if (log)
		{
			//LOGD(LOGTAG_QR,"Alignment Pattern too small: %d < 3",alignmentPatternSum);
		}
		return false; 
	}
	int alignmentPatternSize = (int)round((float)alignmentPatternSum / 3.0f);


	int variance	= alignmentPatternSize >> 2;
	int variance2	= alignmentPatternSize >> 1;

	int a = newBw[0];	int b = newBw[1];	int c = newBw[2];

	int symmetryScore = (abs(a - c) < variance2) * 100;

	int sizeScore = 		
		(abs(alignmentPatternSize - a) < variance2)*33 + 
		(abs(alignmentPatternSize - b) < variance2)*22 + 
		(abs(alignmentPatternSize - c) < variance2)*33; 

	int oldNewCompareScore = 60 * (abs(alignmentPatternSum - horizontalSum) < (horizontalSum >> 2)) + 40*(abs(newBw[1] - previousBw[1]) < variance2);


	int score = (sizeScore + symmetryScore + oldNewCompareScore);	

	if (score > minimumAlignmentPatternScore)
	{		
		if (log)
		{
			//LOGV(LOGTAG_QR,"CheckAlignRatios T: New= %d,%d,%d - Old = %d,%d,%d",newBw[0],newBw[1],newBw[2],previousBw[0],previousBw[1],previousBw[2]);
			//LOGV(LOGTAG_QR,"Found alignment pattern! Scores were: Sym=%d,Size=%d,OldNewScore=%d < MinScore=%d",symmetryScore,sizeScore,oldNewCompareScore,minimumAlignmentPatternScore);
		}
		return true;
	}
	else
	{
		if (log)
		{
			//LOGV(LOGTAG_QR,"CheckAlignRatios F: New= %d,%d,%d - Old = %d,%d,%d",newBw[0],newBw[1],newBw[2],previousBw[0],previousBw[1],previousBw[2]);
			//LOGV(LOGTAG_QR,"Scores: Sym=%d,Size=%d,OldNewScore=%d < MinScore=%d",symmetryScore,sizeScore,oldNewCompareScore,minimumAlignmentPatternScore);
		}
		return false;
	}

}
