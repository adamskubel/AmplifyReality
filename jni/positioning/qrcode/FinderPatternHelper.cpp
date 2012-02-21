#include "QRFinder.hpp"

int QRFinder::MinimumFinderPatternScore = 180;

#define DRAW_FP_DEBUG_LAYER
#define FP_DEBUG_ENABLED true
#define DETECTOR_SIZE 2

static int getTransitionVertical(const Mat & img, int x, int y, int threshold)
{
	int detectorTop[] = {-2,-1};
	int detectorBottom[] = {1,2};

	unsigned char testPx = img.at<unsigned char>(y,x);
	unsigned char pxLower = (threshold > testPx) ? 0 : testPx - threshold;
	unsigned char pxUpper = img.at<unsigned char>(y,x) + threshold;

	int lightToDark = 0, darkToLight = 0;
	for (int i=0;i<2;i++)
	{
		unsigned char px = img.at<unsigned char>((detectorTop[i] + y),x);
		if (px < pxLower)
		{
			darkToLight++;
		}
		else if (px > pxUpper)
		{
			lightToDark++;
		}
	}
	for (int i=0;i<2;i++)
	{
		unsigned char px = img.at<unsigned char>((detectorBottom[i] + y),x);
		if (px < pxLower)
		{
			lightToDark++;
		}
		else if (px > pxUpper)
		{
			darkToLight++;
		}
	}
	
	if (darkToLight > lightToDark)
		return 1;
	else if (lightToDark == darkToLight)
		return 0;
	else
		return -1; //Positive transition: top is darker, bottom is lighter
}

static int getTransition(const unsigned char* rowPtr, int x, int threshold)
{
	unsigned char pxLower = rowPtr[x] - threshold;
	unsigned char pxUpper = rowPtr[x] + threshold;

	int lightToDark  = 0, darkToLight = 0;
	for (int i=1;i<3;i++)
	{
		unsigned char px = rowPtr[x - i];
		if (px < pxLower)
		{
			darkToLight++;
		}
		else if (px > pxUpper)
		{
			lightToDark++;
		}

		px = rowPtr[x + i];
		if (px < pxLower)
		{
			lightToDark++;
		}
		else if (px > pxUpper)
		{
			darkToLight++;
		}
	}	
	
	if (darkToLight > lightToDark)
		return 1;
	else if (lightToDark == darkToLight)
		return 0;
	else
		return -1; //Positive transition: left is darker, right side is light
}

static bool isInRadius(vector<Point3i> & patterns, Point2i point)
{
	for (int i=0;i<patterns.size();i++)
	{
		Point2i patternPoint = Point2i(patterns[i].x,patterns[i].y);
		int distanceSquared = GetSquaredDistance(patternPoint,point);
		if (distanceSquared < patterns[i].z)
			return true;
	}
	return false;
}

void QRFinder::FindFinderPatterns(cv::Mat& M, vector<FinderPattern*> & finderPatterns, vector<Drawable*> & debugVector)
{
	struct timespec start,end;
	SET_TIME(&start);
	//LOGD(LOGTAG_QR,"Enter FFP");
	bool skip = true;
	int bw[6] = { 0 };
	int k = 0;

	int threshold = config->GetIntegerParameter("EdgeThreshold");
	int debugLevel = config->GetIntegerParameter("DebugLevel");
	int verticalResolution = config->GetIntegerParameter("YResolution");
	if (verticalResolution < 1)
		verticalResolution = 1;
	int yBorder = DETECTOR_SIZE;

	vector<Point3i> fpVector;

	//TESTING
	//for (int y = 2; y < M.rows-2; y += 1)
	//{	
	//	for (int x = 2; x < (M.cols-2); x += 4)
	//	{
	//		int transition = getTransitionVertical(M,x,y,threshold);
	//		if (transition < 0)
	//		{
	//			debugVector.push_back(new DebugCircle(Point2i(x,y),5,Colors::Blue));
	//		}
	//		else if (transition > 0)
	//		{
	//			debugVector.push_back(new DebugCircle(Point2i(x,y),5,Colors::Green));
	//		}
	//	}
	//}
	//END
	//return;

	for (int y = yBorder; y < M.rows-yBorder; y += verticalResolution)
	{	

		k = 0;
		bw[0] = bw[1] = bw[2] = bw[3] = bw[4] = bw[5] = 0;

		const unsigned char* Mi = M.ptr<unsigned char>(y);
		for (int x = 2; x < (M.cols-2); x++)
		{
			if (isInRadius(fpVector,Point2i(x,y)))
				continue;

			int transition = getTransition(Mi,x,threshold);

			
			if (k == 0) //Haven't found edge yet
			{
				if (transition < 0) /* Light->dark transistion */
				{
					if (FP_DEBUG_ENABLED && debugLevel > 3)
					{
						debugVector.push_back(new DebugCircle(Point2i(x,y),1,Colors::OrangeRed,true));
					}	
					k++;
				}
			}
			else //Found at least one edge
			{
				if ((k & 1) == 1) //Counting dark
				{
					if (transition > 0) //dark to light
					{
						if (FP_DEBUG_ENABLED && debugLevel > 3)
						{
							debugVector.push_back(new DebugCircle(Point2i(x,y),1,Colors::Blue,true));
						}						
						++k;
					}
				}
				else //Counting light
				{
					if (transition < 0) //light to dark
					{
						if (FP_DEBUG_ENABLED && debugLevel > 3)
						{
							debugVector.push_back(new DebugCircle(Point2i(x,y),1,Colors::Lime,true));
						}					
						++k;
					}
				}
			}

			if (k > 0) ++bw[k-1];

			/*if (FP_DEBUG_ENABLED && debugLevel > 4)
			{
				if (transition < 0)
					debugVector.push_back(new DebugCircle(Point2i(x,y),1,Colors::Red,true));
				else if (transition > 0)
					debugVector.push_back(new DebugCircle(Point2i(x,y),1,Colors::Gold,true));
			}	*/


			if (k == 6)
			{		
				bool result = false;
			/*	if (fpv->size() > 0)
				{
					result = CheckRatios(bw,fpv->back()->patternWidths);
				}
				else
				{*/
				result = CheckRatios(bw,NULL);
				//}

				if (result)
				{
					//Center based on initial ratio
					int tempXCenter = (x - bw[4] - bw[3]) - (bw[2] / 2); 
					
					//y coordinate of center. If check fails, returns 0.
					int tempYCenter = FindCenterVertical(M, tempXCenter, y, bw, debugVector, debugLevel);

					if (tempYCenter > 0)
					{
						int tempXCenter_2 = FindCenterHorizontal(M, tempXCenter, tempYCenter, bw); /* Calculate it again. */

						if (tempXCenter_2 != 0)
						{
							Point2i finderPatternCenter = Point2i(tempXCenter_2,tempYCenter); //Center of finder pattern

							int finderPatternSize =  bw[0] + bw[1] + bw[2] + bw[3] + bw[4];
							int fpRadius = finderPatternSize/2;
							int fpRadiusExclude = ipow(finderPatternSize,2);

							fpVector.push_back(Point3i(finderPatternCenter.x,finderPatternCenter.y, fpRadiusExclude));
							finderPatterns.push_back(new FinderPattern(finderPatternCenter,finderPatternSize));

							if (FP_DEBUG_ENABLED && debugLevel > 0)
								debugVector.push_back(new DebugCircle(finderPatternCenter,fpRadius,Colors::DeepSkyBlue,3));

							//	for (int arrayCopy=0;arrayCopy<5;arrayCopy++)
							//	{
							//		fp->patternWidths[arrayCopy] = bw[arrayCopy];
							//	}
							//	LOGD(LOGTAG_QR,"Copied size array");
						
							//if (skip)
							//{
							//	int s = SkipHeuristic(fpv);

							//	if (s > 0)
							//	{
							//		y = y + s - bw[2] - 2;
							//		skip = false; /* Do not skip again! */

							//		if (y > M.rows) /* Something went wrong, back up. */
							//		{
							//			y = y - s + bw[2] + 2;
							//		}
							//	}
							//}
						}
						else
						{
							//Vertical check succeeded, but horizontal re-check failed
							if (FP_DEBUG_ENABLED && debugLevel > 1)
								debugVector.push_back(new DebugCircle(Point2i(tempXCenter,tempYCenter),12, Colors::OrangeRed,1));
						}

					} else
					{
						//Ratios were correct but vertical check failed
						if (FP_DEBUG_ENABLED && debugLevel > 2)
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
					//Found correct number of transitions, but ratios were incorrect
					if (FP_DEBUG_ENABLED && debugLevel > 4)									
						debugVector.push_back(new DebugCircle(Point2i(x,y),3,Colors::Yellow,1));
					
				}

				k = 4;
				bw[0] = bw[2];
				bw[1] = bw[3];
				bw[2] = bw[4];
				bw[3] = bw[5];
				bw[4] = 0;
				bw[5] = 0;
			}
		}
	}
	
	SET_TIME(&end);
	double finderPatternTime_local = calc_time_double(start,end);
	finderPatternTime = (finderPatternTime + finderPatternTime_local)/2.0;
	config->SetLabelValue("FinderPatternTime",(float)(finderPatternTime/1000.0));
}

bool QRFinder::CheckRatios(int * newBw, int * oldBw, float scoreModifier)
{
	
	int newModuleSum = 0, oldModuleSum = 0;
	bool useOld = oldBw != NULL;

	for (int i = 0; i < 5; ++i)
	{
		if (newBw[i] < 2)
		{
			return false; 
		}
		newModuleSum += newBw[i];
		if (useOld)
		{
			oldModuleSum += oldBw[i];
		}
	}
	
	int a = newBw[0];	int b = newBw[1];	int c = newBw[2];	int d = newBw[3];	int e = newBw[4];

	bool comparisonScore = (c > b && c > d && c > a && c > e);

	if (!comparisonScore) return false; //If comparison fails, exit immediately. 
	
	int newModuleSize = (int)round((float)newModuleSum / 7.0f);
	int variance	= newModuleSize >> 2;
	int variance2	= newModuleSize >> 1;
	int symmetryScore = (abs(a - e) < variance2)*50 + (abs(b - d) < variance2)*50;



	int sizeScore = 		
			(abs(newModuleSize - a) < variance2)*14 + 
			(abs(newModuleSize - b) < variance2)*14 + 
			(abs(newModuleSize - d) < variance2)*14 + 
			(abs(newModuleSize - e) < variance2)*14 + 
			(abs((3 * newModuleSize) - c) < (3 * variance2))*42; 


	int oldNewCompareScore = 40;

	//Compare with previous results, if exist
	// Points for difference between modules less than 25% of last module size
	// Points for center width being less than 25% of previous center size
	if (useOld)
	{		
		oldNewCompareScore =
			(
				(abs(oldModuleSum-newModuleSum) < (oldModuleSum >> 2))*20 +
				(abs(newBw[2] - oldBw[2]) < oldBw[2] >> 1)*20
			);
	
	}

	int score = comparisonScore * (sizeScore + symmetryScore + oldNewCompareScore);	
	
	if (score > (MinimumFinderPatternScore))
	{		
		//LOGV(LOGTAG_QR,"Found pattern! Scores were: Sym=%d,Size=%d,Compare=%d,OldNewScore=%d < MinimumFinderPatternScore=%d",symmetryScore,sizeScore,comparisonScore,oldNewCompareScore,MinimumFinderPatternScore);
		return true;
	}
	else
	{
		//LOGV(LOGTAG_QR,"CheckRatios F: New= %d,%d,%d,%d,%d,%d",newBw[0],newBw[1],newBw[2],newBw[3],newBw[4],newBw[5]);
		//LOGV(LOGTAG_QR,"Scores: Sym=%d,Size=%d,Compare=%d,OldNewScore=%d < MinimumFinderPatternScore=%d",symmetryScore,sizeScore,comparisonScore,oldNewCompareScore,MinimumFinderPatternScore);
		return false;
	}
}
//
//int QRFinder::SkipHeuristic(FinderPattern_vector * fpv)
//{
//	long skip = 0;
//
//	if (fpv->HitConfidence() >= 2)
//	{
//		skip = abs(fpv->at(0)->pt.x - fpv->at(1)->pt.x) - abs(fpv->at(0)->pt.y - fpv->at(1)->pt.y);
//		/* Get space between the
//		 two "top" finder patterns. */
//	}
//
//	return skip;
//}

int QRFinder::FindCenterHorizontal(const Mat& image, int x, int y, int fpbw[], int yDelta)
{
	int threshold =10;
	int bw[5] = { 0 };
	int k = 0;

	const unsigned char * rowPtr = image.ptr<unsigned char>(y);
	
	int j = x;
	for (; j >= 2; j --)
	{		
		int transition = -getTransition(rowPtr,j,threshold); //Invert because going left

		if (k == 0) //Haven't found edge yet
		{
			++bw[2];
			if (transition > 0) //dark to light
			{	
				//debugVector.push_back(new DebugCircle(Point2i(x,i),4,Colors::Aquamarine,true));
				k++;
			}
		}
		else if (k == 1) 
		{
			++bw[3];
			if (transition < 0) //dark to light
			{
				//debugVector.push_back(new DebugCircle(Point2i(x,i),4,Colors::SeaGreen,true));
				k++;
			}
		}
		else if (k == 2) //Found second sedge
		{
			++bw[4];
			if (transition > 0) //dark to light - end of pattern
			{
				//debugVector.push_back(new DebugCircle(Point2i(x,i),4,Colors::Purple,true));
				k++;
				break; //Found last edge, break loop
			}
		}
	}

	if (k < 3) //Exit because we didn't find enough edges
	{
		//LOGV(LOGTAG_QR,"Only found %d edges for top",k);
		return 0;
	}
	//Check bottom
	k = 0;
	for ( j = x+1; j < image.cols-2; j++)
	{		
		int transition = getTransition(rowPtr,j,threshold);

		if (k == 0) //Haven't found edge yet
		{
			++bw[2];
			if (transition > 0)//dark to light
			{				
				//debugVector.push_back(new DebugCircle(Point2i(x,i),4,Colors::Orange,true));
				k++;
			}
		}
		else if (k == 1) //Found first edge
		{
			++bw[1];
			if (transition < 0) //light to dark
			{
				//debugVector.push_back(new DebugCircle(Point2i(x,i),4,Colors::Crimson,true));
				k++;
			}
		}
		else if (k== 2) //Found second sedge
		{
			++bw[0];
			if (transition > 0) //dark to light
			{
				//debugVector.push_back(new DebugCircle(Point2i(x,i),4,Colors::Olive,true));
				k++;
				break; //Found last edge, break loop
			}
		}
	}

	if (k < 3) //Exit because we didn't find enough edges
	{
		//LOGV(LOGTAG_QR,"Only found %d edges for bottom",k);
		return 0;
	}

	
	if (CheckRatios(bw,fpbw))
	{
		//LOGV(LOGTAG_QR,"CheckRatios Vertical T: New= %d,%d,%d,%d,%d",bw[0],bw[1],bw[2],bw[3],bw[4]);
		return (j - bw[4] - bw[3]) - (bw[2] / 2);
	}
	else
	{
		//LOGV(LOGTAG_QR,"CheckRatios Vertical F: New= %d,%d,%d,%d,%d",bw[0],bw[1],bw[2],bw[3],bw[4]);
		return 0;
	}
}

int QRFinder::FindCenterVertical(const Mat& image, int x, int y, int fpbw[], vector<Drawable*> & debugVector, int debugLevel)
{
	int threshold =10;
	int bw[5] = { 0 };
	int k = 0;
	int i = y;
	for (; i >= 2; i --)
	{		
		int transition = -getTransitionVertical(image,x,i,threshold); //invert because we are moving up

		if (k == 0) //Haven't found edge yet
		{
			++bw[2];
			if (transition > 0) //dark to light
			{	
				if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(x,i),1,Colors::Aquamarine,true));
				k++;
			}
		}
		else if (k == 1) 
		{
			++bw[3];
			if (transition < 0) //dark to light
			{
				if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(x,i),1,Colors::SeaGreen,true));
				++k;
			}
		}
		else if (k== 2) //Found second sedge
		{
			++bw[4];
			if (transition > 0) //dark to light - end of pattern
			{
				if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(x,i),1,Colors::Purple,true));
				k++;
				break; //Found last edge, break loop
			}
		}
	}

	if (k != 3) //Exit because we didn't find enough edges
	{
		//LOGV(LOGTAG_QR,"Only found %d edges for top",k);
		return -1;
	}
	//Check bottom
	k = 0;
	for ( i = y+1; i < image.rows-2; i++)
	{		
		int transition = getTransitionVertical(image,x,i,threshold);

		if (k == 0) //Haven't found edge yet
		{
			++bw[2];
			if (transition > 0) /* Light->dark transistion */
			{				
				if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(x,i),1,Colors::Orange,true));
				k++;
			}
		}
		else if (k == 1) //Found first edge
		{
			++bw[1];
			if (transition < 0) //dark to light
			{
				if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(x,i),1,Colors::Crimson,true));
				++k;
			}
		}
		else if (k== 2) //Found second sedge
		{
			++bw[0];
			if (transition > 0) //light to dark
			{
				if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(x,i),1,Colors::Olive,true));
				k++;
				break; //Found last edge, break loop
			}
		}
	}

	if (k != 3) //Exit because we didn't find enough edges
	{
		//LOGV(LOGTAG_QR,"Only found %d edges for bottom",k);
		return -2;
	}

	
	if (CheckRatios(bw,fpbw))
	{
		//LOGV(LOGTAG_QR,"CheckRatios Vertical T: New= %d,%d,%d,%d,%d",bw[0],bw[1],bw[2],bw[3],bw[4]);
		return (i - bw[4] - bw[3]) - (bw[2] / 2);
	}
	else
	{
		//LOGV(LOGTAG_QR,"CheckRatios Vertical F: New= %d,%d,%d,%d,%d",bw[0],bw[1],bw[2],bw[3],bw[4]);
		return 0;
	}
}



/*
static int FindCenterVertical_Old(const Mat& image, int x, int y, int fpbw[])
{
	//LOGD("FindCenterVertical_Start");
	int bw[6] = { 0 };

	int i = y;

	// Calculate vertical finder pattern up from the center. 
	for (; i > 0 && image.at<unsigned char>(i, x) == 0; ++bw[2], --i)
		;

	if (i == 0)
	{
		return 0;
	}

	for (; i > 0 && image.at<unsigned char>(i, x) != 0 && bw[1] < fpbw[2]; ++bw[1], --i)
		;

	if (i == 0 || bw[1] >= fpbw[2])
	{
		return 0;
	}

	for (; i >= 0 && image.at<unsigned char>(i, x) == 0 && bw[0] < fpbw[2]; ++bw[0], --i)
		;

	if (bw[0] >= fpbw[2])
	{
		return 0;
	}

	// Calculate vertical finder pattern down from the center. 
	i = y + 1;
	for (; i < image.rows && image.at<unsigned char>(i, x) == 0; ++bw[2], ++i)
		;

	if (i == image.rows)
	{
		return 0;
	}

	for (; i < image.rows && image.at<unsigned char>(i, x) != 0 && bw[3] < fpbw[2]; ++bw[3], ++i)
		;
	if (i == image.rows || bw[3] >= fpbw[2])
	{
		return 0;
	}

	for (; i < image.rows && image.at<unsigned char>(i, x) == 0 && bw[4] < fpbw[2]; ++bw[4], ++i)
		;

	if (bw[4] >= fpbw[2])
	{
		return 0;
	}

	if (CheckRatios2(bw,fpbw) == false)
	{
		//LOGD("FinderPatternHelper","VertCenter CheckRatio2 Failed: %d,%d,%d,%d,%d",bw[0],bw[1],bw[2],bw[3],bw[4]);
		return 0;
	}

	//LOGD( "FindCenterVertical_End");
	return (i - bw[4] - bw[3]) - (bw[2] / 2);
}
*/


/*
int QRFinder::FindCenterHorizontal(const Mat& image, int x, int startY, int fpbw[], int yDelta)
{
	int bw[6] = { 0 };

	//LOGD( "FindCenterHorizontal_Start");
	int j = x;
	int y = startY;

	// Calculate horizontal finder pattern left from the center.
	for (; j > 0 && image.at<unsigned char>(y, j) == 0; ++bw[2], --j, y-= yDelta)
		;
	if (j == 0)
	{
		return 0;
	}

	for (; j > 0 && image.at<unsigned char>(y, j) != 0 && bw[1] < fpbw[2]; ++bw[1], --j, y-= yDelta)
		;
	if (j == 0 || bw[1] >= fpbw[2])
	{
		return 0;
	}

	for (; j >= 0 && image.at<unsigned char>(y, j) == 0 && bw[0] < fpbw[2]; ++bw[0], --j, y-= yDelta)
		;
	if (bw[0] >= fpbw[2])
	{
		return 0;
	}

	// Calculate horizontal finder pattern right from the center. 
	j = x + 1;
	y = startY + yDelta;

	for (; j < image.cols && image.at<unsigned char>(y, j) == 0; ++bw[2], ++j, y+= yDelta)
		;
	if (j == image.rows)
	{
		return 0;
	}

	for (; j < image.cols && image.at<unsigned char>(y, j) != 0 && bw[3] < fpbw[2]; ++bw[3], ++j, y+= yDelta)
		;
	if (j == image.cols || bw[3] >= fpbw[2])
	{
		return 0;
	}

	for (; j < image.cols && image.at<unsigned char>(y, j) == 0 && bw[4] < fpbw[2]; ++bw[4], ++j, y+= yDelta)
		;
	if (bw[4] >= fpbw[2])
	{
		return 0;
	}

	float scoreMod = 1.0f;
	if (yDelta != 0)
		scoreMod = 0.7f;

	if (CheckRatios(bw,NULL) == false)
	{
		return 0;
	}

	//LOGD("FindCenterHorizontal_End");
	return (j - bw[4] - bw[3]) - (bw[2] / 2);
}



bool QRFinder::CheckRatios2(int newBw[], int oldBw[])
{
	int newModuleSum = 0, oldModuleSum = 0;
	for (int i = 0; i < 5; ++i)
	{
		if (newBw[i] == 0)
		{
			return false; 
		}
		newModuleSum += newBw[i];
	}
	if (newModuleSum < 7)
	{
		return false; 
	}
	int newModuleSize = (int)round((float)newModuleSum / 7.0f);


	for (int i = 0; i < 5; ++i)
	{
		if (newBw[i] == 0)
		{
			return false; 
		}
		oldModuleSum += oldBw[i];
	}
		

	int variance	= newModuleSize >> 2;
	int variance2	= newModuleSize >> 1;

	int a = newBw[0];	int b = newBw[1];	int c = newBw[2];	int d = newBw[3];	int e = newBw[4];

	int symmetryScore = (abs(a - e) < variance2) && (abs(b - d) < variance2);
	int sizeScore = ((abs(newModuleSize - a) < variance2) + (abs(newModuleSize - b) < variance2) + (abs(newModuleSize - d) < variance2) + (abs(newModuleSize - e) < variance2)) + 
		(abs((3 * newModuleSize) - c) < (3 * variance2))*3; //Max 7

	int comparisonScore = (c > b && c > d && c > a && c > e);

	int oldNewCompareScore = ((abs(oldModuleSum-newModuleSum) < (oldModuleSum >> 2)) && abs((newBw[2] - oldBw[2]) < oldBw[2] >> 1));
	
	if ((symmetryScore && sizeScore && comparisonScore && oldNewCompareScore) &&
		(symmetryScore*3 + sizeScore + comparisonScore*3 + oldNewCompareScore*6) > 14)
	{		
		return true;
	}
	else
	{
		LOGD("FinderPatternHelper","CheckRatio2 F: New= %d,%d,%d,%d,%d",newBw[0],newBw[1],newBw[2],newBw[3],newBw[4]);
		LOGD("FinderPatternHelper","CheckRatio2 F: Old= %d,%d,%d,%d,%d",oldBw[0],oldBw[1],oldBw[2],oldBw[3],oldBw[4]);
		LOGD("FinderPatternHelper","Scores: Sym=%d,Size=%d,Compare=%d,OldNew=%d",symmetryScore,sizeScore,comparisonScore,oldNewCompareScore);
		return false;
	}
}
*/