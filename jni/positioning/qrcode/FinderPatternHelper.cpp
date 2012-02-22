#include "QRFinder.hpp"

int QRFinder::MinimumFinderPatternScore = 220;

#define DRAW_FP_DEBUG_LAYER
#define FP_DEBUG_ENABLED true
#define DETECTOR_SIZE 2


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

//Finds horizontal edges and stores in edgeArray
static void FindEdges(const Mat & inputImg, Mat & edgeArray, short threshold, bool nonMaxSuppress, int detectorSize = 3)
{
	//const short detectorSize = 3;
	const short maxColumn = inputImg.cols - detectorSize;
		
	Mat tmpEdgeArray = Mat::zeros(1,inputImg.cols,CV_16S); 
	
	const unsigned char * imgRowPtr;

	short * edgeRowPtr = tmpEdgeArray.ptr<short>(0);

	short * outputEdgeRowPtr;
	
	for (int i=0;i < inputImg.rows;i++)
	{
		imgRowPtr = inputImg.ptr<unsigned char>(i);
		outputEdgeRowPtr = edgeArray.ptr<short>(i);
	
		for (int j=detectorSize;j < maxColumn;j++)
		{
			if (detectorSize == 3)
				edgeRowPtr[j] = -((short)imgRowPtr[j+1] + (short)imgRowPtr[j+2] + (short)imgRowPtr[j+3]) + 
										(short)imgRowPtr[j-1] + (short)imgRowPtr[j-2] + (short)imgRowPtr[j-3];
			else if (detectorSize == 4)
				edgeRowPtr[j] = -((short)imgRowPtr[j+1] + (short)imgRowPtr[j+2] + (short)imgRowPtr[j+3] + (short)imgRowPtr[j+4]) + 
										(short)imgRowPtr[j-1] + (short)imgRowPtr[j-2] + (short)imgRowPtr[j-3] + (short)imgRowPtr[j-4];
			else			
				edgeRowPtr[j] = -((short)imgRowPtr[j+1] + (short)imgRowPtr[j+2]) + 
										(short)imgRowPtr[j-1] + (short)imgRowPtr[j-2];


			if (j > (detectorSize+1) && edgeRowPtr[j-1] != 0 && abs(edgeRowPtr[j-1]) > threshold && 
					(	
						!nonMaxSuppress || 
						(
							abs(edgeRowPtr[j-1]) > abs(edgeRowPtr[j-2]) &&
							abs(edgeRowPtr[j-1]) > abs(edgeRowPtr[j])
						)
					)
				)
			{
				outputEdgeRowPtr[j-1] = (edgeRowPtr[j-1] < 0) ? 1 : -1; 
			}
			else
			{
				outputEdgeRowPtr[j-1] = 0;				
			}
		}
	
	}
}

//Finds horizontal edges and stores in edgeArray
static void FindEdgesClosed(const Mat & inputImg, Mat & edgeArray, short threshold, bool nonMaxSuppress, int detectorSize = 3)
{
	//const short detectorSize = 3;
	const short maxColumn = inputImg.cols - detectorSize;
		
	Mat tmpEdgeArray = Mat::zeros(3,inputImg.cols,CV_16S); //Located and scored edges. Always 3??
	
	const unsigned char * imgRowPtr;

	short * edgeRowPtr = tmpEdgeArray.ptr<short>(0);

	short * outputEdgeRowPtr;
	short * lastOutputRow = NULL;
	short * beforeLastOutputRow = NULL;
	
	for (int i=0;i < inputImg.rows;i++)
	{
		imgRowPtr = inputImg.ptr<unsigned char>(i);
		outputEdgeRowPtr = edgeArray.ptr<short>(i);
	
		for (int j=detectorSize;j < maxColumn;j++)
		{
			if (detectorSize == 3)
				edgeRowPtr[j] = -((short)imgRowPtr[j+1] + (short)imgRowPtr[j+2] + (short)imgRowPtr[j+3]) + 
										(short)imgRowPtr[j-1] + (short)imgRowPtr[j-2] + (short)imgRowPtr[j-3];
			else if (detectorSize == 4)
				edgeRowPtr[j] = -((short)imgRowPtr[j+1] + (short)imgRowPtr[j+2] + (short)imgRowPtr[j+3] + (short)imgRowPtr[j+4]) + 
										(short)imgRowPtr[j-1] + (short)imgRowPtr[j-2] + (short)imgRowPtr[j-3] + (short)imgRowPtr[j-4];
			else			
				edgeRowPtr[j] = -((short)imgRowPtr[j+1] + (short)imgRowPtr[j+2]) + 
										(short)imgRowPtr[j-1] + (short)imgRowPtr[j-2];


			if (j > (detectorSize+1) && edgeRowPtr[j-1] != 0 && abs(edgeRowPtr[j-1]) > threshold && 
					(	
						!nonMaxSuppress || 
						(
							abs(edgeRowPtr[j-1]) > abs(edgeRowPtr[j-2]) &&
							abs(edgeRowPtr[j-1]) > abs(edgeRowPtr[j])
						)
					)
				)
			{
				outputEdgeRowPtr[j-1] = (edgeRowPtr[j-1] < 0) ? 1 : -1; 
			}
			else
			{
				outputEdgeRowPtr[j-1] = 0;				
			}
			

			if ((lastOutputRow != NULL && beforeLastOutputRow != NULL) && 
					lastOutputRow[j-1] == 0 && 
					(outputEdgeRowPtr[j-2] != 0 || outputEdgeRowPtr[j-1] != 0 || outputEdgeRowPtr[j] != 0) &&
					(beforeLastOutputRow[j-2] != 0 || beforeLastOutputRow[j-1] != 0 || beforeLastOutputRow[j] != 0))
				lastOutputRow[j-1] = outputEdgeRowPtr[j-1] * 2;

		}
		beforeLastOutputRow = lastOutputRow;	
		lastOutputRow = outputEdgeRowPtr;	
	}
}


//Finds vertical edges and stores in edgeArray
static void FindEdgesVertical(const Mat & inputImg, Mat & edgeArray, int xPosition, short threshold, bool nonMaxSuppress, int detectorSize)
{
	//const short detectorSize = 3;
	const short maxColumn = inputImg.rows - detectorSize;
	
	struct timespec start,end;
	SET_TIME(&start);

	//Copy the column into a row for faster access
	Mat rowBasedImg(1,inputImg.rows,CV_8U);
	for (int i=0;i<inputImg.rows;i++)
	{
		rowBasedImg.at<unsigned char>(0,i) = inputImg.at<unsigned char>(i,xPosition);
	}
	const unsigned char * imgRowPtr = rowBasedImg.ptr<unsigned char>(0);
	
	Mat tmpEdgeArray = Mat::zeros(1,inputImg.rows,CV_16S); //Located and scored edges
	short * edgeRowPtr = tmpEdgeArray.ptr<short>(0);
	edgeArray = Mat::zeros(1,inputImg.rows,CV_16S);
	short * outputEdgeRowPtr = edgeArray.ptr<short>(0);	
		
	for (int j=detectorSize;j < maxColumn;j++)
	{
		if (detectorSize == 3)
			edgeRowPtr[j] = -((short)(imgRowPtr[j+1] + (short)imgRowPtr[j+2] + (short)imgRowPtr[j+3])) + (short)imgRowPtr[j-1] + (short)imgRowPtr[j-2] + (short)imgRowPtr[j-3];
		else if (detectorSize == 4)
			edgeRowPtr[j] = -((short)imgRowPtr[j+1] + (short)imgRowPtr[j+2] + (short)imgRowPtr[j+3] + (short)imgRowPtr[j+4]) + (short)imgRowPtr[j-1] + (short)imgRowPtr[j-2] + (short)imgRowPtr[j-3] + (short)imgRowPtr[j-4];
		else			
			edgeRowPtr[j] = -((short)imgRowPtr[j+1] + (short)imgRowPtr[j+2]) + (short)imgRowPtr[j-1] + (short)imgRowPtr[j-2];

		if (j > (detectorSize + 1) && edgeRowPtr[j-1] != 0 && abs(edgeRowPtr[j-1]) > threshold && 
				(	
					!nonMaxSuppress || 
					(
						abs(edgeRowPtr[j-1]) > abs(edgeRowPtr[j-2]) && // || (abs(edgeRowPtr[j-1]) == abs(edgeRowPtr[j-2])  && abs(edgeRowPtr[j-1]) > abs(edgeRowPtr[j-3]))) &&
						abs(edgeRowPtr[j-1]) > abs(edgeRowPtr[j])
					)
				)
			)
		{
			outputEdgeRowPtr[j-1] = (edgeRowPtr[j-1] < 0) ? 1 : -1; 
		}
		/*else
		{
			outputEdgeRowPtr[j-1] = 0;
		}*/
	}

	SET_TIME(&end);
	LOG_TIME_PRECISE("VerticalEdgeTest",start,end);	
}

//Finds vertical edges and stores in edgeArray
//Vertical edge array is the size of the image, transposed, and contains vertical edge transition features
void QRFinder::FindEdgesVerticalClosed(const Mat & inputImg, int xPosition)
{
	const short maxRow = inputImg.rows - detectorSize;
	if (xPosition < 1 || xPosition >= inputImg.cols - 1)
		return;
	if (calculatedEdges.count(xPosition) != 0 && calculatedEdges.at(xPosition)) //This column has already been completely calculated
		return;
		
	struct timespec start,end;
	SET_TIME(&start);

	//Copy the relevant columns into rows for faster access
	/*Mat rowBasedImg(3,inputImg.rows,CV_8U);
	int rowCount = 0;
	for (int i = xPosition-1;i <= xPosition+1;i++)
	{
		rowCount++;
		if (calculatedEdges.count(i) != 0)
			continue;

		for (int j=0;j<inputImg.rows;j++)
		{
			rowBasedImg.at<unsigned char>(rowCount,j) = inputImg.at<unsigned char>(j,i);
		}
	}*/

	//const unsigned char * imgRowPtr = rowBasedImg.ptr<unsigned char>(0);
	

	Mat tmpEdgeArray = Mat::zeros(1,inputImg.rows,CV_16S); //Located and scored edges
	short * edgeRowPtr = tmpEdgeArray.ptr<short>(0);
	short * outputEdgeRowPtr;
	
	for (int i=xPosition-1;i <= xPosition+1;i++)
	{
		if (calculatedEdges.count(i) != 0)
			continue;

		outputEdgeRowPtr = verticalEdgeArray.ptr<short>(i);	
		for (int j=detectorSize;j < maxRow;j++)
		{
			edgeRowPtr[j] = -((short)inputImg.at<unsigned char>((j-1),i) + (short)inputImg.at<unsigned char>((j-2),i)) + (short)inputImg.at<unsigned char>((j+1),i) + (short)inputImg.at<unsigned char>((j+2),i);

			if (j > (detectorSize + 1) && edgeRowPtr[j-1] != 0 && abs(edgeRowPtr[j-1]) > edgeThreshold 
				&& 
					(	
						!nonMaxEnabled || 
						(
							abs(edgeRowPtr[j-1]) > abs(edgeRowPtr[j-2]) && 
							abs(edgeRowPtr[j-1]) > abs(edgeRowPtr[j])
						)
					)
				)
			{
				outputEdgeRowPtr[j-1] = (edgeRowPtr[j-1] < 0) ? -1 : 1; 
			}
			else
			{
				outputEdgeRowPtr[j-1] = 0;
			}
		}
		calculatedEdges[i] = false;
	}
		
	outputEdgeRowPtr = verticalEdgeArray.ptr<short>(xPosition+1);	
	short * lastOutputRow = verticalEdgeArray.ptr<short>(xPosition);
	short * beforeLastOutputRow = verticalEdgeArray.ptr<short>(xPosition-1);
	for (int j=detectorSize;j < maxRow;j++)
	{
		if (lastOutputRow[j-1] == 0 && 
			(outputEdgeRowPtr[j-2] != 0 || outputEdgeRowPtr[j-1] != 0 || outputEdgeRowPtr[j] != 0) &&
			(beforeLastOutputRow[j-2] != 0 || beforeLastOutputRow[j-1] != 0 || beforeLastOutputRow[j] != 0))

			lastOutputRow[j-1] = outputEdgeRowPtr[j-1] * 2;
	}

	calculatedEdges[xPosition] = true;

	SET_TIME(&end);
	//LOG_TIME_PRECISE("VerticalEdgeTest",start,end);	
}

void QRFinder::FindFinderPatterns(cv::Mat& inputImg, vector<FinderPattern*> & finderPatterns, vector<Drawable*> & debugVector)
{
	struct timespec start,end;
	SET_TIME(&start);
	//LOGD(LOGTAG_QR,"Enter FFP");
	bool skip = true;
	int bw[6] = { 0 };
	int k = 0;

	//Clear vertical edge map
	calculatedEdges.clear();

	//Get parameters from config
	edgeThreshold = config->GetIntegerParameter("EdgeThreshold");
	debugLevel = config->GetIntegerParameter("DebugLevel");
	int verticalResolution = config->GetIntegerParameter("YResolution");
	nonMaxEnabled = config->GetBooleanParameter("EdgeNonMax");
	minimumFinderPatternScore = config->GetIntegerParameter("MinimumFPScore");
	detectorSize = config->GetIntegerParameter("DetectorSize");

	if (verticalResolution < 1)
		verticalResolution = 1;
	int yBorder = detectorSize;

	vector<Point3i> fpVector;

	//If image size has changed, reallocate the matrix
	if (imgSize.width != inputImg.cols && imgSize.height != inputImg.rows)
	{
		imgSize = Size2i(inputImg.cols,inputImg.rows);
		LOGD(LOGTAG_QR,"Initializing new matrix, size=(%d,%d)",imgSize.width,imgSize.height);
		edgeArray = Mat::zeros(inputImg.rows,inputImg.cols,CV_16S); //Maximized edges
		verticalEdgeArray = Mat::zeros(inputImg.cols,inputImg.rows,CV_16S); //Maximized vertical edges
	}
	
	//Find horizontal edges
	SET_TIME(&start);
	FindEdgesClosed(inputImg,edgeArray,edgeThreshold,nonMaxEnabled,detectorSize);
	SET_TIME(&end);
	double edgeTime_local = calc_time_double(start,end);
	edgeTime = (edgeTime + edgeTime_local)/2.0;
	config->SetLabelValue("EdgeTime",(float)edgeTime/1000.0f);


	//Testing Matrix based edge detection	
	/*

	for (int y = 0; y < edgeArray.rows; y ++)
	{	
		const short * edgePtr = edgeArray.ptr<short>(y);
		for (int x = 0; x < edgeArray.cols; x ++)
		{
			int transition = edgePtr[x];
			if (transition < 0)
			{
				debugVector.push_back(new DebugCircle(Point2i(x,y),1,Colors::Blue,true));
			}
			else if (transition > 0)
			{
				debugVector.push_back(new DebugCircle(Point2i(x,y),1,Colors::Green,true));
			}
		}
	}
	return;
	
	*/
	//TESTING

	if (debugLevel > 4 || debugLevel <= -2)
	{
		for (int x = 1; x < verticalEdgeArray.rows-1; x ++)
		{
			FindEdgesVerticalClosed(inputImg,x);		
			const short * verticalEdgePtr = verticalEdgeArray.ptr<short>(x);
			for (int y = 0; y < (verticalEdgeArray.cols);y++)
			{
				short transition = verticalEdgePtr[y];
				if (transition < 0)
				{
					debugVector.push_back(new DebugCircle(Point2i(x,y),0,Colors::Fuchsia,true));
				}
				else if (transition > 0)
				{
					debugVector.push_back(new DebugCircle(Point2i(x,y),0,Colors::Cyan,true));
				}

				//if (abs(transition) == 2)
				//{
				//	debugVector.push_back(new DebugCircle(Point2i(x,y),3,Colors::OrangeRed,1));
				//}
			}
		}	
	}
	//END
	
	for (int y = yBorder; y < edgeArray.rows - yBorder; y += verticalResolution)
	{	
		k = 0;
		bw[0] = bw[1] = bw[2] = bw[3] = bw[4] = bw[5] = 0;

		const short * edgeRowPtr = edgeArray.ptr<short>(y);
		for (int x = 2; x < (edgeArray.cols-2); x++)
		{
			if (isInRadius(fpVector,Point2i(x,y)))
				continue;

			int transition =  edgeRowPtr[x]; //getTransition(Mi,x,threshold);

			
			if (k == 0) //Haven't found edge yet
			{
				if (transition < 0) /* Light->dark transistion */
				{
					/*if (FP_DEBUG_ENABLED && debugLevel > 3)
					{
						debugVector.push_back(new DebugCircle(Point2i(x,y),1,Colors::OrangeRed,true));
					}	*/
					k++;
				}
			}
			else //Found at least one edge
			{
				if ((k & 1) == 1) //Counting dark
				{
					if (transition > 0) //dark to light
					{
						/*if (FP_DEBUG_ENABLED && debugLevel > 3)
						{
							debugVector.push_back(new DebugCircle(Point2i(x,y),0,Colors::Blue,true));
						}*/						
						++k;
					}
				}
				else //Counting light
				{
					if (transition < 0) //light to dark
					{
						/*if (FP_DEBUG_ENABLED && debugLevel > 3)
						{
							debugVector.push_back(new DebugCircle(Point2i(x,y),0,Colors::Lime,true));
						}	*/				
						++k;
					}
				}
			}

			if (k > 0) ++bw[k-1];

			if (FP_DEBUG_ENABLED && debugLevel > 3 || debugLevel == -1 || debugLevel == -2)
			{				
				if (transition < 0)
					debugVector.push_back(new DebugCircle(Point2i(x,y),0,Colors::Lime,true));
				else if (transition > 0)
					debugVector.push_back(new DebugCircle(Point2i(x,y),0,Colors::Yellow,true));

				
			}	


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
					int tempYCenter = FindCenterVertical(inputImg, tempXCenter, y, bw, debugVector);

					if (tempYCenter > 0)
					{
						int tempXCenter_2 = FindCenterHorizontal(edgeArray, tempXCenter, tempYCenter, bw, debugVector); /* Calculate it again. */

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

							//		if (y > inputImg.rows) /* Something went wrong, back up. */
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

	if (!comparisonScore)
	{
		LOGV(LOGTAG_QR,"Failed comparison, widths= %d,%d,%d,%d,%d,%d",newBw[0],newBw[1],newBw[2],newBw[3],newBw[4],newBw[5]);
		return false; //If comparison fails, exit immediately. 
	}
	
	int newModuleSize = (int)round((float)newModuleSum / 7.0f);
	int variance	= newModuleSize >> 2;
	int variance2	= newModuleSize >> 1;
	variance2 = MAX(2,variance2);
	int symmetryScore = (abs(a - e) < variance2)*50 + (abs(b - d) < variance2)*50;


	int previousSimilarScore = 20;
	//Compare with previous results, if exist
	//Exit if comparison fails
	//NOPE: Points for difference between modules less than 25% of last module size
	//NOPE: Points for center width being less than 25% of previous center size
	if (useOld)
	{		
		bool compareResult = (abs(oldModuleSum-newModuleSum) < (oldModuleSum >> 2)) && (abs(newBw[2] - oldBw[2]) < oldBw[2] >> 1);
		if (!compareResult)
		{
			LOGV(LOGTAG_QR,"Failed old-new comparison,New= %d,%d,%d,%d,%d,%d",newBw[0],newBw[1],newBw[2],newBw[3],newBw[4],newBw[5]);
			return false;
		}
		previousSimilarScore += 40;
	}


	int sizeScore = 		
			(abs(newModuleSize - a) < variance2)*14 + 
			(abs(newModuleSize - b) < variance2)*14 + 
			(abs(newModuleSize - d) < variance2)*14 + 
			(abs(newModuleSize - e) < variance2)*14 + 
			(abs((3 * newModuleSize) - c) < (3 * variance2))*42; 

	int score = comparisonScore * (sizeScore + symmetryScore + previousSimilarScore);	
	
	if (score > (minimumFinderPatternScore))
	{		
		if (useOld)
			LOGV(LOGTAG_QR,"Found pattern! Scores were: Sym=%d,Size=%d,Compare=%d < MinimumFinderPatternScore=%d",symmetryScore,sizeScore,comparisonScore,minimumFinderPatternScore);
		return true;
	}
	else
	{
		if (useOld)
		{
			LOGV(LOGTAG_QR,"CheckRatios F: New= %d,%d,%d,%d,%d",newBw[0],newBw[1],newBw[2],newBw[3],newBw[4]);
			LOGV(LOGTAG_QR,"CheckRatios F: Old= %d,%d,%d,%d,%d",oldBw[0],oldBw[1],oldBw[2],oldBw[3],oldBw[4]);
			LOGV(LOGTAG_QR,"Scores: Sym=%d,Size=%d,Compare=%d < MinimumFinderPatternScore=%d",symmetryScore,sizeScore,comparisonScore,minimumFinderPatternScore);
		}
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

int QRFinder::FindCenterHorizontal(const Mat& edgeArray, int x, int y, int fpbw[], vector<Drawable*> & debugVector)
{
	//Size2i debugCircleSize(0,50);
	int debugCircleSize = 3;
	int debugThickness = 1;

	int bw[5] = { 0 };
	int k = 0;

	const short * rowPtr = edgeArray.ptr<short>(y);
	
	int j = x;
	for (; j >= 2; j --)
	{		
		short transition = -rowPtr[j]; //Invert because going left

		if (k == 0) //Haven't found edge yet
		{
			++bw[2];
			if (transition > 0) //dark to light
			{	
				if (FP_DEBUG_ENABLED && debugLevel > 1)
					debugVector.push_back(new DebugCircle(Point2i(j,y),debugCircleSize,Colors::Aquamarine,1));
				k++;
			}
		}
		else if (k == 1) 
		{
			++bw[3];
			if (transition < 0) //dark to light
			{
				if (FP_DEBUG_ENABLED && debugLevel > 1)
					debugVector.push_back(new DebugLine(Point2i(j,y),debugCircleSize,Colors::SeaGreen,1));
				k++;
			}
		}
		else if (k == 2) //Found second edge
		{
			++bw[4];
			if (transition > 0) //dark to light - end of pattern
			{
				if (FP_DEBUG_ENABLED && debugLevel > 1)
					debugVector.push_back(new DebugLine(Point2i(j,y),debugCircleSize,Colors::CornflowerBlue,1));
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
	for ( j = x+1; j < edgeArray.cols-2; j++)
	{		
		short transition = rowPtr[j];

		if (k == 0) //Haven't found edge yet
		{
			++bw[2];
			if (transition > 0)//dark to light
			{		
				if (FP_DEBUG_ENABLED && debugLevel > 1)
					debugVector.push_back(new DebugLine(Point2i(j,y),debugCircleSize,Colors::Yellow,1));
				k++;
			}
		}
		else if (k == 1) //Found first edge
		{
			++bw[1];
			if (transition < 0) //light to dark
			{
				if (FP_DEBUG_ENABLED && debugLevel > 1)
					debugVector.push_back(new DebugLine(Point2i(j,y),debugCircleSize,Colors::Orange,1));
				k++;
			}
		}
		else if (k== 2) //Found second sedge
		{
			++bw[0];
			if (transition > 0) //dark to light
			{
				if (FP_DEBUG_ENABLED && debugLevel > 1)
					debugVector.push_back(new DebugLine(Point2i(j,y),debugCircleSize,Colors::Red,1));
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
		LOGV(LOGTAG_QR,"CheckRatios Horizontal T: New= %d,%d,%d,%d,%d",bw[0],bw[1],bw[2],bw[3],bw[4]);
		return (j - bw[4] - bw[3]) - (bw[2] / 2);
	}
	else
	{
		LOGV(LOGTAG_QR,"CheckRatios Horizontal F: New= %d,%d,%d,%d,%d",bw[0],bw[1],bw[2],bw[3],bw[4]);
		return 0;
	}
}

static void DrawVerticalEdges(Mat & edges, int x, vector<Drawable*> & debugVector, Scalar negative = Colors::Blue, Scalar positive = Colors::Lime, int radius = 2)
{
	int lineWidth = 60;
	const short * rowPtr = edges.ptr<short>(0);
	for (int q = 0;q < edges.cols;q++)
	{
		short transition = rowPtr[q];
		if (transition < 0)
		{
			//debugVector.push_back(new DebugLine(Point2i(x-lineWidth,q),Point2i(x+lineWidth,q),negative,1));
			debugVector.push_back(new DebugLine(Point2i(x,q),lineWidth,negative,1));
		}
		else if (transition > 0)
		{
			//debugVector.push_back(new DebugLine(Point2i(x-lineWidth,q),Point2i(x+lineWidth,q),positive,1));
			debugVector.push_back(new DebugLine(Point2i(x,q),lineWidth,positive,1));
		}
	}
}

int QRFinder::FindCenterVertical(const Mat& image, int x, int y, int fpbw[], vector<Drawable*> & debugVector)
{
	//Size2i debugCircleSize(50,0);
	
	int debugCircleSize = 3;
	int debugThickness = 1;

	int bw[5] = { 0 };
	int k = 0;
	int i = y;

	//FindEdgesVertical(image,edges,x,edgeThreshold,nonMaxEnabled,detectorSize);
	FindEdgesVerticalClosed(image,x);
	const short * rowPtr = verticalEdgeArray.ptr<short>(x);

	for (; i >= 2; i --)
	{		
		int transition = -rowPtr[i]; //invert because we are moving up

		if (k == 0) //Haven't found edge yet
		{
			++bw[2];
			if (transition > 0) //dark to light
			{	
				if (FP_DEBUG_ENABLED && debugLevel > 1)
					debugVector.push_back(new DebugCircle(Point2i(x,i),debugCircleSize,Colors::Aquamarine,1));
				k++;
			}
		}
		else if (k == 1) 
		{
			++bw[3];
			if (transition < 0) //dark to light
			{
				if (FP_DEBUG_ENABLED && debugLevel > 1)
					debugVector.push_back(new DebugCircle(Point2i(x,i),debugCircleSize,Colors::SeaGreen,1));
				++k;
			}
		}
		else if (k== 2) //Found second sedge
		{
			++bw[4];
			if (transition > 0) //dark to light - end of pattern
			{
				if (FP_DEBUG_ENABLED && debugLevel > 1)
					debugVector.push_back(new DebugCircle(Point2i(x,i),debugCircleSize,Colors::CornflowerBlue,1));
				k++;
				break; //Found last edge, break loop
			}
		}
	}

	if (k != 3) //Exit because we didn't find enough edges
	{
		//LOGV(LOGTAG_QR,"Only found %d edges for top",k);
		/*if (FP_DEBUG_ENABLED && debugLevel > 1)
		{
			DrawVerticalEdges(edges,x,debugVector);
		}*/
		return -1;
	}
	//Check bottom
	k = 0;
	for ( i = y; i < image.rows-2; i++)
	{		
		int transition = rowPtr[i];// getTransitionVertical(image,x,i,threshold);

		if (k == 0) //Haven't found edge yet
		{
			++bw[2];
			if (transition > 0) /* Light->dark transistion */
			{				
				if (FP_DEBUG_ENABLED && debugLevel > 1)
					debugVector.push_back(new DebugCircle(Point2i(x,i),debugCircleSize,Colors::Yellow,1));
				k++;
			}
		}
		else if (k == 1) //Found first edge
		{
			++bw[1];
			if (transition < 0) //dark to light
			{
				if (FP_DEBUG_ENABLED && debugLevel > 1)
					debugVector.push_back(new DebugCircle(Point2i(x,i),debugCircleSize,Colors::Orange,1));
				++k;
			}
		}
		else if (k== 2) //Found second sedge
		{
			++bw[0];
			if (transition > 0) //light to dark
			{
				if (FP_DEBUG_ENABLED && debugLevel > 1)
					debugVector.push_back(new DebugCircle(Point2i(x,i),debugCircleSize,Colors::Red,1));
				k++;
				break; //Found last edge, break loop
			}
		}
	}

	if (k != 3) //Exit because we didn't find enough edges
	{
	/*	if (FP_DEBUG_ENABLED && debugLevel > 2)
		{			
			DrawVerticalEdges(edges,x,debugVector);
		}*/
		//LOGV(LOGTAG_QR,"Only found %d edges for bottom",k);
		return -2;
	}
	
	//We counted twice, adjust center pattern
	--bw[2];
	if (CheckRatios(bw,fpbw))
	{
		//LOGV(LOGTAG_QR,"CheckRatios Vertical T: New= %d,%d,%d,%d,%d",bw[0],bw[1],bw[2],bw[3],bw[4]);
		return (i - bw[4] - bw[3]) - (bw[2] / 2);
	}
	else
	{
		/*if (FP_DEBUG_ENABLED && debugLevel > 2)
		{			
			DrawVerticalEdges(edges,x,debugVector);
		}*/
		//LOGV(LOGTAG_QR,"CheckRatios Vertical F: New= %d,%d,%d,%d,%d",bw[0],bw[1],bw[2],bw[3],bw[4]);
		return 0;
	}
}



//Old way
/*

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
*/