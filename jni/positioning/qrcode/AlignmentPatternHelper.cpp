#include "AlignmentPatternHelper.hpp"


int AlignmentPatternHelper::MinimumAlignmentPatternScore = 180;

//#define DRAW_AP_DEBUG_LAYER

void AlignmentPatternHelper::FindAlignmentPattern(Mat & M, QRCode * newCode, vector<Drawable*>& debugVector)
{
	Point2i searchCenter = newCode->alignmentPattern;
	float finderPatternSize = newCode->finderPatterns.at(0)->size;
	
	int startX = searchCenter.x - finderPatternSize, endX = searchCenter.x + finderPatternSize;
	if (startX < 0)
		startX = 0;
	if (endX >= M.cols)
		endX = M.cols -1;

	int startY = searchCenter.y- finderPatternSize, endY = searchCenter.y + finderPatternSize;
	if (startY < 0)
		startY = 0;
	if (endY >= M.rows)
		endY = M.rows -1;
#ifdef DRAW_AP_DEBUG_LAYER
	debugVector.push_back(new DebugRectangle(Rect(Point2i(startX,startY),Point2i(endX,endY)),Colors::CornflowerBlue));
#endif
	for (int y = startY;y<endY;y+=2)
	{
		int bw[3] = { 0 };
		int k = 0;
		bw[0] = bw[1] = bw[2] = 0;
		const unsigned char * rowPointer = M.ptr<unsigned char>(y);
		for (int x = startX;x<endX;x++)
		{
			unsigned char px = rowPointer[x];

			if (px != 0) /* White Pixel found*/
			{
				++bw[k];
				if (k == 1) /* Were counting black pixels, change state to count white pixels. */
				{
					++k;
				}
			}
			else if (bw[0] != 0) /* Black Pixel found. Ensure white pixel was first */
			{
				++bw[k];
				if (k != 1) /* Were counting white pixels, change state to count black pixels. */
				{
					++k;
				}
			}

			if (k == 3)
			{
				if (CheckAlignmentRatios(bw,finderPatternSize))
				{
					int tx = (x - bw[2]) - (bw[1] / 2); /* Calculate temporary horizontal center from end of pattern. */
					int verticalBw[3] = {0,0,0};
					int ty = FindAlignmentCenterVertical(M, tx, y,finderPatternSize,verticalBw,bw);

					if (ty != 0)
					{
						int horizontalBw[3] = {0,0,0};
						int t = FindAlignmentCenterHorizontal(M, tx, ty, finderPatternSize, verticalBw, horizontalBw); /* Calculate it again. */

						if (t!= 0)
						{								
							tx = t;					
							if (CheckAlignmentDiagonals(M,Point2i(tx,ty),verticalBw,horizontalBw,debugVector))
							{									
								int aPSize =  horizontalBw[2] + horizontalBw[1] + horizontalBw[0];
#ifdef DRAW_AP_DEBUG_LAYER
								debugVector.push_back(new DebugCircle(Point2i(tx,ty),20, Colors::Lime));
#endif
								newCode->alignmentPattern = Point2i(tx,ty);
								return;
							}	
							else
							{			
#ifdef DRAW_AP_DEBUG_LAYER
								debugVector.push_back(new DebugCircle(Point2i(tx,ty),20, Colors::PeachPuff));
#endif		
							}
						}
						else
						{
#ifdef DRAW_AP_DEBUG_LAYER
							debugVector.push_back(new DebugCircle(Point2i(tx,ty),10, Colors::Gold));
#endif		
						}
					}
					else
					{
#ifdef DRAW_AP_DEBUG_LAYER
						debugVector.push_back(new DebugCircle(Point2i(tx,y),3,Colors::Red,-1));
#endif					
					}

				}
				k = 1;
				bw[0] = bw[2];
				bw[1] = 0;
				bw[2] = 0;
			}
		}
	}
}

int AlignmentPatternHelper::FindAlignmentCenterVertical(const Mat& image, int x, int y, int finderPatternSize, int * bw, int * horizontalBw)
{
	//LOGD("FindCenterVertical_Start");
	//int bw[3] = { 0 };

	int i = y;

	/* Calculate vertical finder pattern up from the center. */
	for (; i > 0 && image.at<unsigned char>(i, x) == 0 && bw[1] < finderPatternSize; ++bw[1], --i)
		;

	if (i == 0 || bw[1] >= finderPatternSize)
	{
		return 0;
	}

	for (; i > 0 && image.at<unsigned char>(i, x) != 0 && bw[0] < finderPatternSize; ++bw[0], --i)
		;

	if (i == 0 || bw[0] >= finderPatternSize)
	{
		return 0;
	}

	/* Calculate vertical finder pattern down from the center. */
	i = y + 1;
	for (; i < image.rows && image.at<unsigned char>(i, x) == 0 && bw[1] < finderPatternSize; ++bw[1], ++i)
		;

	if (i == image.rows || bw[1] >= finderPatternSize)
	{
		return 0;
	}

	for (; i < image.rows && image.at<unsigned char>(i, x) != 0 && bw[2] < finderPatternSize; ++bw[2], ++i)
		;
	if (i == image.rows || bw[2] >= finderPatternSize)
	{
		return 0;
	}


	if (CheckAlignmentRatios(bw, horizontalBw) == false)
	{
		//LOGV(LOGTAG_QR,"FindAlignmentCenterVertical F: BW= %d,%d,%d",bw[0],bw[1],bw[2]);
		//LOGV(LOGTAG_QR,"Failed ratio check for finding vertical center, FPSize=%d",finderPatternSize);
		return 0;
	}

	return (i - bw[2]) - (bw[1] / 2);
}

bool AlignmentPatternHelper::CheckAlignmentDiagonals(const Mat& image, Point2i center, int * verticalBw, int * horizontalBw, vector<Drawable*> & debugVector)
{
	int xMin = center.x - (horizontalBw[0] + horizontalBw[1])/2;

	int xMax = center.x + (horizontalBw[2] + horizontalBw[1])/2;
	

	int  yMin = center.y - (verticalBw[0] + verticalBw[1])/2;
	int  yMax = center.y + (verticalBw[2] + verticalBw[1])/2;
#ifdef DRAW_AP_DEBUG_LAYER
	debugVector.push_back(new DebugRectangle(Rect(Point2i(xMin,yMin), Point2i(xMax,yMax)),Colors::Red));
#endif
	int j = xMin;
	for (; j < xMax && image.at<unsigned char>(yMin,j) != 0; j++)
		;

	if (j < xMax)
		return false;

	j = xMin;
	for (; j < xMax && image.at<unsigned char>(yMax,j) != 0; j++)
		;

	if (j < xMax)
		return false;

	return true;
}

int AlignmentPatternHelper::FindAlignmentCenterHorizontal(const Mat& image, int x, int y, int finderPatternSize, int * verticalBw, int * bw)
{
	int j = x;
	
	/* Calculate horizontal finder pattern left from the center. */
	for (; j > 0 && image.at<unsigned char>(y, j) == 0 && bw[1] < finderPatternSize; ++bw[1], --j)
		;

	if (j == 0 || bw[1] >= finderPatternSize)
	{
		//LOGD(LOGTAG_QR,"Pattern exceed boundaries :0 - j=%d",j);
		return 0;
	}

	for (; j > 0 && image.at<unsigned char>(y, j) != 0 && bw[0] < finderPatternSize; ++bw[0], --j)
		;

	if (j == 0 || bw[0] >= finderPatternSize)
	{
		//LOGD(LOGTAG_QR,"Pattern exceed boundaries :1 - j=%d",j);
		return 0;
	}

	/* Calculate horizontal finder pattern right from the center. */
	j = x + 1;
	for (; j < image.cols && image.at<unsigned char>(y, j) == 0 && bw[1] < finderPatternSize; ++bw[1], ++j)
		;

	if (j == image.cols || bw[1] >= finderPatternSize)
	{
		//LOGD(LOGTAG_QR,"Pattern exceed boundaries :2 - j=%d",j);
		return 0;
	}

	for (; j < image.cols && image.at<unsigned char>(y, j) != 0 && bw[2] < finderPatternSize; ++bw[2], ++j)
		;
	if (j == image.cols || bw[2] >= finderPatternSize)
	{
		//LOGD(LOGTAG_QR,"Pattern exceed boundaries :3 - j=%d",j);
		return 0;
	}


	if (CheckAlignmentRatios(bw,verticalBw,false) == false)
	{
		//LOGD(LOGTAG_QR,"AlignmentPattern Ratio Check Failed. Fpsize=%d, Bw=%d,%d,%d", finderPatternSize, bw[0],bw[1],bw[2]);
		return 0;
	}
		
	return (j - bw[2]) - (bw[1] / 2);
}

bool AlignmentPatternHelper::CheckAlignmentRatios(int * newBw, int finderPatternSize)
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
	
	if (score > MinimumAlignmentPatternScore)
	{				
		//LOGV(LOGTAG_QR,"CheckAlignRatios T: New= %d,%d,%d",newBw[0],newBw[1],newBw[2]);
		//LOGV(LOGTAG_QR,"Found alignment pattern! Scores were: Sym=%d,Size=%d,OldNewScore=%d < MinScore=%d",symmetryScore,sizeScore,oldNewCompareScore,QRFinder::MinimumAlignmentPatternScore);
		return true;
	}
	else
	{
		//LOGV(LOGTAG_QR,"CheckAlignRatios F: New= %d,%d,%d",newBw[0],newBw[1],newBw[2]);
		//LOGV(LOGTAG_QR,"Scores: Sym=%d,Size=%d,OldNewScore=%d < MinScore=%d",symmetryScore,sizeScore,oldNewCompareScore,QRFinder::MinimumAlignmentPatternScore);
		return false;
	}

}

bool AlignmentPatternHelper::CheckAlignmentRatios(int * newBw, int * previousBw, bool log)
{
	int alignmentPatternSum = 0, horizontalSum = 0;
	
	for (int i = 0; i < 3; ++i)
	{
		if (newBw[i] == 0)
		{
			if (log)
			{
				LOGD(LOGTAG_QR,"Zero for width #%d",i);
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
			LOGD(LOGTAG_QR,"Alignment Pattern too small: %d < 3",alignmentPatternSum);
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
	
	if (score > MinimumAlignmentPatternScore)
	{		
		if (log)
		{
			LOGV(LOGTAG_QR,"CheckAlignRatios T: New= %d,%d,%d - Old = %d,%d,%d",newBw[0],newBw[1],newBw[2],previousBw[0],previousBw[1],previousBw[2]);
			LOGV(LOGTAG_QR,"Found alignment pattern! Scores were: Sym=%d,Size=%d,OldNewScore=%d < MinScore=%d",symmetryScore,sizeScore,oldNewCompareScore,MinimumAlignmentPatternScore);
		}
		return true;
	}
	else
	{
		if (log)
		{
			LOGV(LOGTAG_QR,"CheckAlignRatios F: New= %d,%d,%d - Old = %d,%d,%d",newBw[0],newBw[1],newBw[2],previousBw[0],previousBw[1],previousBw[2]);
			LOGV(LOGTAG_QR,"Scores: Sym=%d,Size=%d,OldNewScore=%d < MinScore=%d",symmetryScore,sizeScore,oldNewCompareScore,MinimumAlignmentPatternScore);
		}
		return false;
	}

}
