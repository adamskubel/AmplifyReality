#include "QRFinder.hpp"

#define DRAW_FP_DEBUG_LAYER
#define FP_DEBUG_ENABLED true
#define DETECTOR_SIZE 2


class PatternSizeCompare
{
	public:
      bool operator()(const FinderPattern * fp0, const FinderPattern * fp1)
	  {
		  return (fp0->size < fp1->size);
	  }
};


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
void QRFinder::FindEdgesClosed(const Mat & inputImg, Rect regionOfInterest, Mat & edgeArray, short threshold, bool nonMaxSuppress, int detectorSize, int yResolution)
{
	//const short detectorSize = 3;
	short maxColumn = (regionOfInterest.height == 0) ? inputImg.cols : regionOfInterest.x + regionOfInterest.width;
	maxColumn -= detectorSize;	
	short maxRow = (regionOfInterest.height == 0) ? 0 : regionOfInterest.y + regionOfInterest.height;
	short xStart = (regionOfInterest.height == 0) ? 0 : regionOfInterest.x;
	short yStart = (regionOfInterest.height == 0) ? 0 : regionOfInterest.y;
	xStart += detectorSize;
		
	Mat tmpEdgeArray = Mat::zeros(3,inputImg.cols,CV_16S); //Located and scored edges. Always 3??
	
	const unsigned char * imgRowPtr;

	short * edgeRowPtr = tmpEdgeArray.ptr<short>(0);

	short * outputEdgeRowPtr;
	short * lastOutputRow = NULL;
	short * beforeLastOutputRow = NULL;
	

	for (int i=yStart;i < maxRow;i+= yResolution)
	{
		imgRowPtr = inputImg.ptr<unsigned char>(i);
		outputEdgeRowPtr = edgeArray.ptr<short>(i);
	
		for (int j=xStart;j < maxColumn;j++)
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

void QRFinder::FindFinderPatterns(cv::Mat& inputImg, Rect regionOfInterest, vector<FinderPattern*> & finderPatterns, vector<Drawable*> & debugVector)
{
	struct timespec start,end;
	SET_TIME(&start);	

	//Get parameters from config
	edgeThreshold = config->GetIntegerParameter("EdgeThreshold");
	debugLevel = config->GetIntegerParameter("QR Debug Level");
	int verticalResolution = config->GetIntegerParameter("YResolution");
	nonMaxEnabled = config->GetBooleanParameter("EdgeNonMax");
	minimumFinderPatternScore = config->GetIntegerParameter("MinimumFPScore");
	detectorSize = config->GetIntegerParameter("DetectorSize");

	int yBorder = detectorSize;
	vector<Point3i> exclusionZones;
	
	//Calculate limits
	int maxColumn = regionOfInterest.x + regionOfInterest.width;
	maxColumn -= detectorSize;	
	int maxRow = regionOfInterest.y + regionOfInterest.height;
	int xStart = regionOfInterest.x;
	int yStart = regionOfInterest.y;
	xStart += detectorSize;
	yStart += detectorSize;
	maxColumn -= detectorSize;
	maxRow -= detectorSize;

	
	if (debugLevel > 0)
		debugVector.push_back(new DebugRectangle(Rect(Point2i(xStart,yStart),Point2i(maxColumn,maxRow)),Colors::Aqua,1));

	
	//Find horizontal edges
	SET_TIME(&start);
	FindEdgesClosed(inputImg,regionOfInterest,edgeArray,edgeThreshold,nonMaxEnabled,detectorSize);
	SET_TIME(&end);
	double edgeTime_local = calc_time_double(start,end);
	edgeTime = (edgeTime + edgeTime_local)/2.0;
	config->SetLabelValue("EdgeTime",(float)edgeTime/1000.0f);
	
	//If debug level set, find all vertical edges and draw them
	if (debugLevel <= -2)
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
			}
		}	
	}
	//END
	
	
	int bw[6] = { 0 };
	int k = 0;
	//LOGV(LOGTAG_QR,"ImgSize=[%d,%d] EdgeSize=[%d,%d]",inputImg.rows,inputImg.cols,edgeArray.rows,edgeArray.cols);

	int yDirection = 1;
	int yCenter = yStart + (maxRow - yStart)/2;
	int y = yStart, absOffset = 0;

	LOGV(LOGTAG_QR,"Beginning search. y[%d->%d], Center=%d, x[%d->%d]",yStart,maxRow,yCenter,xStart,maxColumn);
	
	while (y < maxRow && y >= yStart)
	{	
		y = yCenter + absOffset * yDirection;

		if (yDirection == 1) absOffset += verticalResolution;	//Increment every other frame		
		yDirection = -yDirection;								//Change direction every frame

		k = 0;
		bw[0] = bw[1] = bw[2] = bw[3] = bw[4] = bw[5] = 0;

		const short * edgeRowPtr = edgeArray.ptr<short>(y);
		for (int x = xStart; x < maxColumn; x++)
		{
			if (isInRadius(exclusionZones,Point2i(x,y)))
				continue;

			int transition =  edgeRowPtr[x]; //getTransition(Mi,x,threshold);

			
			if (k == 0) //Haven't found edge yet
			{
				if (transition < 0) /* Light->dark transistion */
				{					
					k++;
				}
			}
			else //Found at least one edge
			{
				if ((k & 1) == 1) //Counting dark
				{
					if (transition > 0) //dark to light
					{					
						++k;
					}
				}
				else //Counting light
				{
					if (transition < 0) //light to dark
					{					
						++k;
					}
				}
			}

			if (k > 0) ++bw[k-1];

			if (FP_DEBUG_ENABLED && (debugLevel == -1 || debugLevel == -2))
			{				
				if (transition < 0)
					debugVector.push_back(new DebugCircle(Point2i(x,y),0,Colors::Lime,true));
				else if (transition > 0)
					debugVector.push_back(new DebugCircle(Point2i(x,y),0,Colors::Yellow,true));

				
			}	


			if (k == 6)
			{		
				int result = 0;
				result = CheckRatios(bw,NULL);

				if (result == 1)
				{	
					//LOGV(LOGTAG_QR,"Ratio check pass");
					//Center based on initial ratio					
					float patternCenterWidth = (float)bw[2];
					int tempXCenter = (x - bw[4] - bw[3]) - (int)round(patternCenterWidth/2.0f); 
					float xOffset = (patternCenterWidth/6.0f);
					
					//y coordinate of center. If check fails, returns 0.
					int tempYCenterArray[] = {0,0,0};

					int * verticalPatternSizes[3];

					for (int i = 0;i < 3;i++)
						verticalPatternSizes[i] = new int[5];

					tempYCenterArray[0] = FindCenterVertical(inputImg, tempXCenter, y, bw, debugVector,verticalPatternSizes[0]);
					tempYCenterArray[1] = FindCenterVertical(inputImg, tempXCenter - xOffset, y, bw, debugVector,verticalPatternSizes[1]);
					tempYCenterArray[2] = FindCenterVertical(inputImg, tempXCenter + xOffset, y, bw, debugVector,verticalPatternSizes[2]);
										
					int tempYCenter = 0;
					int passCount = 0;
					float avgYSize = 0;

					int averageVerticalSize[5] = {0,0,0,0,0};

					for (int yTest = 0; yTest < 3; yTest++)
					{
						if (tempYCenterArray[yTest] > 0)
						{
							passCount++;
							tempYCenter += tempYCenterArray[yTest];
							for (int i=0;i<5;i++)
							{
								averageVerticalSize[i] += (verticalPatternSizes[yTest])[i];
								avgYSize += (verticalPatternSizes[yTest])[i]; 
							}
						}						
					}

					if (passCount >= 2)
					{
						//LOGV(LOGTAG_QR,"Vertical test pass-1");
						
						tempYCenter = (int)round((float)tempYCenter / (float)passCount);
						avgYSize = (float)avgYSize / (float)passCount;

						int allowedVariance = (int)avgYSize >> 2;
						bool yVarianceTest = true;
						for (int yTest = 0; yTest < 3; yTest++)
						{
							if (tempYCenterArray[yTest] > 0)
							{
								if (abs(tempYCenterArray[yTest] - tempYCenter) > allowedVariance)
								{
									yVarianceTest = false;
									break;
								}
							}						
						}

						if (yVarianceTest)
						{
							//LOGV(LOGTAG_QR,"Vertical test pass-2. Passcount=%d",passCount);
							//Average the vertical pattern sizes
							for (int i=0;i<5;i++)
							{
								averageVerticalSize[i] = idiv(averageVerticalSize[i],passCount);
							}
							//LOGV(LOGTAG_QR,"Averaged sizes. Center=%d",averageVerticalSize[2]);

							int tempXCenterArray[] = {0,0,0};
							int xSizeArray[] = {0,0,0};
							int yOffset = idiv(averageVerticalSize[2],6.0f);

							//LOGV(LOGTAG_QR,"Yoffset=%d,yCenter=%d",yOffset,tempYCenter);
							tempXCenterArray[0] = FindCenterHorizontal(tempXCenter, tempYCenter-yOffset, bw, xSizeArray[0], debugVector); 
							tempXCenterArray[1] = FindCenterHorizontal(tempXCenter, tempYCenter, bw, xSizeArray[1], debugVector); 
							tempXCenterArray[2] = FindCenterHorizontal(tempXCenter, tempYCenter+yOffset, bw, xSizeArray[2], debugVector); 

							tempXCenter = 0;
							passCount = 0;
							float avgXSize = 0;

							for (int xTest = 0; xTest < 3; xTest++)
							{
								if (tempXCenterArray[xTest] > 0)
								{
									passCount++;
									tempXCenter += tempXCenterArray[xTest];
									avgXSize += xSizeArray[xTest];
								}						
							}

							if (passCount >= 2)
							{
								
								//LOGV(LOGTAG_QR,"Horizontal test pass");
								tempXCenter = (int)round((float)tempXCenter / (float)passCount);
								avgXSize = (float)avgXSize/(float)passCount;
								//allowedVariance = (int)round((float)avgYSize/1.5f);
								float aspectRatio = avgXSize/avgYSize;
								
								if (aspectRatio > 0.33f && aspectRatio < 3.0f)
								{
									
									//LOGV(LOGTAG_QR,"Size test pass");
									Point2i finderPatternCenter = Point2i(tempXCenter,tempYCenter); //Center of finder pattern

									int finderPatternSize =  MAX(avgXSize,avgYSize);
									int fpRadius = (int)round((float)finderPatternSize/2.0f);
									int fpRadiusExclude = ipow(finderPatternSize,2);

									//LOGD(LOGTAG_QR,"Creating new pattern[%d,%d]",avgXSize,avgYSize);
									//Create a new pattern
									FinderPattern * newPattern = new FinderPattern(finderPatternCenter,Size2i(avgXSize,avgYSize));
									Size2f patternSearchSize = Size2f(avgXSize,avgYSize);

									vector<Point2i> corners;
									struct timespec fastStart,fastEnd;
									SET_TIME(&fastStart);
									fastQRFinder->LocateFPCorners(inputImg,newPattern,corners,debugVector);
//									fastQRFinder->CheckAlignmentPattern(inputImg,finderPatternCenter,patternSearchSize,corners,debugVector);
									SET_TIME(&fastEnd);
									double fastFPTime_Local = calc_time_double(fastStart,fastEnd);
									fastFPTime += fastFPTime_Local;
									if (corners.size() == 4)
									{
										//if (validatePattern(newPattern,finderPatterns))
										//{
										newPattern->patternCorners = corners;
										exclusionZones.push_back(Point3i(finderPatternCenter.x,finderPatternCenter.y, fpRadiusExclude));
										finderPatterns.push_back(newPattern);
										if (FP_DEBUG_ENABLED && debugLevel > 0)
										{
											debugVector.push_back(new DebugCircle(finderPatternCenter,fpRadius,Colors::MediumSpringGreen,1,true));
											for (int i=0;i<corners.size();i++)
											{
												if (FP_DEBUG_ENABLED && debugLevel > 0)
													debugVector.push_back(new DebugCircle(corners[i],10,Colors::DodgerBlue,2));
											}
										}
										//}
										//else
										//{
										//	//LOGV(LOGTAG_QR,"Compare check failed");
										//	if (FP_DEBUG_ENABLED && debugLevel > 0)
										//		debugVector.push_back(new DebugCircle(finderPatternCenter,fpRadius,Colors::HotPink,2));
										//}
									}
									else
									{
										
										//LOGV(LOGTAG_QR,"FAST check failed");
										if (FP_DEBUG_ENABLED && debugLevel > 0)
											debugVector.push_back(new DebugCircle(finderPatternCenter,fpRadius,Colors::Red,2));
										delete newPattern;
									}
								}
								else
								{
									//LOGV(LOGTAG_QR,"Size check failed");
									//Size check failed
									if (FP_DEBUG_ENABLED && debugLevel > 1)
										debugVector.push_back(new DebugCircle(Point2i(tempXCenter,tempYCenter),13, Colors::HotPink,1));
								}
							}
							else
							{
								//LOGV(LOGTAG_QR,"Horizontal check failed");
								//Vertical check succeeded, but horizontal re-check failed
								if (FP_DEBUG_ENABLED && debugLevel > 1)
									debugVector.push_back(new DebugCircle(Point2i(tempXCenter,tempYCenter),12, Colors::OrangeRed,1));
							}
						}
						else
						{
							//LOGV(LOGTAG_QR,"Variance test failed. AllowedVariance = %d, yCenters = %d,%d,%d [avg=%d], AvgYSize=%d",allowedVariance,tempYCenterArray[0],tempYCenterArray[1],tempYCenterArray[2],tempYCenter,avgYSize);
							//Vertical variance test failed
							if (FP_DEBUG_ENABLED && debugLevel > 1)
								debugVector.push_back(new DebugCircle(Point2i(tempXCenter,tempYCenter),14, Colors::MediumSpringGreen,1));
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
					{
						if (result == 0)
							debugVector.push_back(new DebugCircle(Point2i(x,y),2,Colors::Yellow,-1));
						if (result == -1)
							debugVector.push_back(new DebugCircle(Point2i(x,y),2,Colors::DarkTurquoise,-1));
					}
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
	
	//LOGV(LOGTAG_QR,"Returning from FPSearch");
	SET_TIME(&end);
	double finderPatternTime_local = calc_time_double(start,end);
	finderPatternTime = (finderPatternTime + finderPatternTime_local)/2.0;
	config->SetLabelValue("FinderPatternTime",(float)(finderPatternTime/1000.0));

	config->SetLabelValue("FPCornerTime",fastFPTime/1000.0);
	fastFPTime = 0;

	numVerticalCalc = idiv(numVerticalCalc + calculatedEdges.size(),2);
	config->SetLabelValue("NumVerticalCalc",numVerticalCalc);
}

int QRFinder::CheckRatios(int * newBw, int * oldBw, float scoreModifier)
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
		//LOGV(LOGTAG_QR,"Failed comparison, widths= %d,%d,%d,%d,%d",newBw[0],newBw[1],newBw[2],newBw[3],newBw[4]);
		return -1;
	}
	
	int newModuleSize = (int)round((float)newModuleSum / 7.0f);
	int variance	= newModuleSize >> 2;
	int variance2	= newModuleSize >> 1;
	variance2 = MAX(2,variance2);
	int symmetryScore = (abs(a - e) < variance2)*50 + (abs(b - d) < variance2)*50;


	int previousSimilarScore = 20;
	//Compare with previous results, if exist
	//Exit if comparison fails
	if (useOld)
	{		
		bool compareResult = (abs(oldModuleSum-newModuleSum) < (oldModuleSum >> 1)) && (abs(newBw[2] - oldBw[2]) < oldBw[2] >> 1);
		if (!compareResult)
		{
			//LOGV(LOGTAG_QR,"Failed old-new comparison,New= %d,%d,%d,%d,%d,%d",newBw[0],newBw[1],newBw[2],newBw[3],newBw[4],newBw[5]);
			return -2;
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
		//if (useOld)
			//LOGV(LOGTAG_QR,"Found pattern! Scores were: Sym=%d,Size=%d,Compare=%d < MinimumFinderPatternScore=%d",symmetryScore,sizeScore,comparisonScore,minimumFinderPatternScore);
		return 1;
	}
	else
	{
		//if (useOld)
		//{
		//LOGV(LOGTAG_QR,"CheckRatios F: New= %d,%d,%d,%d,%d",newBw[0],newBw[1],newBw[2],newBw[3],newBw[4]);
		//LOGV(LOGTAG_QR,"CheckRatios F: Old= %d,%d,%d,%d,%d",oldBw[0],oldBw[1],oldBw[2],oldBw[3],oldBw[4]);
		//LOGV(LOGTAG_QR,"Scores: Sym=%d,Size=%d,Compare=%d < MinimumFinderPatternScore=%d",symmetryScore,sizeScore,comparisonScore,minimumFinderPatternScore);
		//}
		return 0;
	}
}

int QRFinder::FindCenterHorizontal(int x, int y, int fpbw[], int & xSize, vector<Drawable*> & debugVector)
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
				if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(j,y),debugCircleSize,Colors::Aquamarine,1));
				k++;
			}
		}
		else if (k == 1) 
		{
			++bw[3];
			if (transition < 0) //dark to light
			{
				if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(j,y),debugCircleSize,Colors::SeaGreen,1));
				k++;
			}
		}
		else if (k == 2) //Found second edge
		{
			++bw[4];
			if (transition > 0) //dark to light - end of pattern
			{
				if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(j,y),debugCircleSize,Colors::CornflowerBlue,1));
				k++;
				break; //Found last edge, break loop
			}
		}
	}

	if (k < 3) //Exit because we didn't find enough edges
	{
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
				if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(j,y),debugCircleSize,Colors::Yellow,1));
				k++;
			}
		}
		else if (k == 1) //Found first edge
		{
			++bw[1];
			if (transition < 0) //light to dark
			{
				if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(j,y),debugCircleSize,Colors::Orange,1));
				k++;
			}
		}
		else if (k== 2) //Found second sedge
		{
			++bw[0];
			if (transition > 0) //dark to light
			{
				if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(j,y),debugCircleSize,Colors::Red,1));
				k++;
				break; //Found last edge, break loop
			}
		}
	}

	if (k < 3) //Exit because we didn't find enough edges
	{
		return 0;
	}

	
	if (CheckRatios(bw,fpbw) == 1)
	{
		//LOGV(LOGTAG_QR,"CheckRatios Horizontal T: New= %d,%d,%d,%d,%d",bw[0],bw[1],bw[2],bw[3],bw[4]);
		xSize = bw[0]+bw[1]+bw[2]+bw[3]+bw[4];
		return (j - bw[4] - bw[3]) - (bw[2] / 2);
	}
	else
	{
		//LOGV(LOGTAG_QR,"CheckRatios Horizontal F: New= %d,%d,%d,%d,%d",bw[0],bw[1],bw[2],bw[3],bw[4]);
		return 0;
	}
}

bool QRFinder::validatePattern(FinderPattern * newPattern, vector<FinderPattern*> & patternVector)
{
	

	//Choose the best patterns. If the new is used, return true.
	if (patternVector.size() >= 2)
	{

		LOGD(LOGTAG_QR,"Validating %d patterns",patternVector.size());
		float minDeviance = config->GetParameter("MinFPDeviance");
		float maxDevianceRatio = config->GetParameter("MaxDevianceRatio");
		vector<FinderPattern*> tmpVector = patternVector;

		//std::sort(tmpVector.begin(),tmpVector.end(),PatternSizeCompare());
				
		float avgSize = 0;
		tmpVector.push_back(newPattern);
		for (int i=0;i<tmpVector.size();i++)
		{
			avgSize += tmpVector.at(i)->size;
		}
		avgSize = avgSize / (float)tmpVector.size();
		float avgDeviance = 0;

		for (int i=0;i<tmpVector.size();i++)
		{
			avgDeviance = abs(tmpVector[i]->size - avgSize);
		}

		float devRatio = avgDeviance/avgSize;


		//Can't determine pattern validity accurately, so return true
		if (devRatio > maxDevianceRatio)
		{
			LOGD(LOGTAG_QR,"Too much deviance, aborting");
			return true;
		}
		else
		{
			patternVector.clear();
			avgDeviance = MAX(minDeviance*avgSize,avgDeviance);
			//LOGD(LOGTAG_QR,"Allowed Deviance =%f",avgDeviance);
			bool result = false;
			for (int i=0;i<tmpVector.size();i++)
			{
				float deviance = abs(tmpVector[i]->size - avgSize);
				if (deviance <= avgDeviance)
				{					
					//Don't push back new pattern
					if (tmpVector.at(i) == newPattern)
					{
						//LOGD(LOGTAG_QR,"New pattern is ok!");
						result = true;
					}
					else
						patternVector.push_back(tmpVector.at(i));
				}
				else
				{
					LOGD(LOGTAG_QR,"Deleting pattern with pt=(%d,%d)",tmpVector[i]->pt.x,tmpVector[i]->pt.y);
					delete tmpVector.at(i);
				}
			}
			
			LOGD(LOGTAG_QR,"Validated %d patterns!",patternVector.size());
			return result;
		}
	}
	
	LOGD(LOGTAG_QR,"Only %d patterns, cannot validate",patternVector.size());
	return true;
}

int QRFinder::FindCenterVertical(const Mat& image, int x, int y, int fpbw[], vector<Drawable*> & debugVector, int * verticalPatternSizes)
{
	//Size2i debugCircleSize(50,0);
	
	int debugCircleSize = 3;
	int debugThickness = 1;

	int bw[5] = { 0 };
	int k = 0;
	int i = y;

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
				if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(x,i),debugCircleSize,Colors::Aquamarine,1));
				k++;
			}
		}
		else if (k == 1) 
		{
			++bw[3];
			if (transition < 0) //dark to light
			{
				if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(x,i),debugCircleSize,Colors::SeaGreen,1));
				++k;
			}
		}
		else if (k== 2) //Found second sedge
		{
			++bw[4];
			if (transition > 0) //dark to light - end of pattern
			{
				if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(x,i),debugCircleSize,Colors::CornflowerBlue,1));
				k++;
				break; //Found last edge, break loop
			}
		}
	}

	if (k != 3) //Exit because we didn't find enough edges
	{		
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
				if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(x,i),debugCircleSize,Colors::Yellow,1));
				k++;
			}
		}
		else if (k == 1) //Found first edge
		{
			++bw[1];
			if (transition < 0) //dark to light
			{
				if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(x,i),debugCircleSize,Colors::Orange,1));
				++k;
			}
		}
		else if (k== 2) //Found second sedge
		{
			++bw[0];
			if (transition > 0) //light to dark
			{
				if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(x,i),debugCircleSize,Colors::Red,1));
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
	
	//We counted twice, adjust center pattern
	--bw[2];

	int result = CheckRatios(bw,fpbw);
	if (result == 1)
	{
		if (verticalPatternSizes != NULL)
		{
			for (int i=0;i<5;i++)
			{
				verticalPatternSizes[i] = bw[i];
			}
		}
		//LOGV(LOGTAG_QR,"CheckRatios Vertical T: New= %d,%d,%d,%d,%d",bw[0],bw[1],bw[2],bw[3],bw[4]);
		return (i - bw[4] - bw[3]) - (bw[2] / 2);
	}
	else
	{
		//LOGV(LOGTAG_QR,"CheckRatios Vertical F: New= %d,%d,%d,%d,%d",bw[0],bw[1],bw[2],bw[3],bw[4]);
		if (result == -2)
		{
			if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(x,y),debugCircleSize*2,Colors::Teal,1));
		}
		else if (result == -1)
		{
			if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(x,y),debugCircleSize*2,Colors::Maroon,1));
		}
		else 
		{
			if (FP_DEBUG_ENABLED && debugLevel > 2)
					debugVector.push_back(new DebugCircle(Point2i(x,y),debugCircleSize*2,Colors::BlueViolet,1));
		}
		return result;
	}
}

