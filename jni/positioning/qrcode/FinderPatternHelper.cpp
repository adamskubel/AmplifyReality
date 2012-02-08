
#include "FinderPatternHelper.hpp"

int FinderPatternHelper::MinimumFinderPatternScore = 180;

//#define DRAW_FP_DEBUG_LAYER

void FinderPatternHelper::FindFinderPatterns(cv::Mat& M, FinderPattern_vector * fpv, vector<Drawable*> & debugVector)
{
	LOGD(LOGTAG_QR,"Enter FFP");
	bool skip = true;
	int bw[5] = { 0 };
	int k = 0;

	for (int y = 0; y < M.rows; y += 3)
	{

		k = 0;
		bw[0] = bw[1] = bw[2] = bw[3] = bw[4] = 0;

		const unsigned char* Mi = M.ptr<unsigned char>(y);
		for (int x = 0; x < M.cols; x++)
		{

			unsigned char px = Mi[x];
			if (px == 0) /* Black Pixel found. */
			{
				++bw[k];
				if ((k & 1) == 1) /* Were counting white pixels,
				 change state to count black pixels. */
				{
					++k;
				}
			} else if (bw[0] != 0) /* White Pixel found, but ensure there was a black pixel first. */
			{
				++bw[k];
				if ((k & 1) == 0) /* Were counting black pixels,
				 change state to count white pixels. */
				{
					++k;
				}
			}

			if (k == 5)
			{				
				bool result = false;
				if (fpv->size() > 0)
				{
					result = CheckRatios(bw,fpv->back()->patternWidths);
				}
				else
				{
					result = CheckRatios(bw,NULL);
				}

				if (result)
				{

					int tx = (x - bw[4] - bw[3]) - (bw[2] / 2); /* Calculate temporary horizontal center from end of pattern. */

					int ty = FindCenterVertical(M, tx, y, bw);

					if (ty != 0)
					{
						LOGD(LOGTAG_QR,"Found vertical");
						int t = FindCenterHorizontal(M, tx, ty, bw); /* Calculate it again. */
					
												
						if (t == 0)
						{
#ifdef DRAW_FP_DEBUG_LAYER
							debugVector.push_back(new DebugCircle(Point2i(tx,ty),10, Colors::Gold));
#endif
							return;
						}
						
						LOGD(LOGTAG_QR,"Found horizontal");

						//int hcenterDiag1 = FindCenterHorizontal(M, tx, ty, bw,1); //Calculate diagonal centers
						//int hcenterDiag2 = FindCenterHorizontal(M, tx, ty, bw,-1); 

						if (false)//hcenterDiag1 == 0)
						{
#ifdef DRAW_DP_DEBUG_LAYER
							debugVector.push_back(new DebugCircle(Point2i(tx,ty),10, Colors::Red));
#endif							
							return;
						}
						else
						{
#ifdef DRAW_FP_DEBUG_LAYER
							debugVector.push_back(new DebugCircle(Point2i(hcenterDiag1,ty),4, Colors::Red,true));
#endif
						}

						if (false)//if (hcenterDiag2 == 0)
						{
#ifdef DRAW_FP_DEBUG_LAYER	
							debugVector.push_back(new DebugCircle(Point2i(tx,ty),10,Colors::Lime));
#endif
							return;
						}
						else
						{
#ifdef DRAW_FP_DEBUG_LAYER								
							debugVector.push_back(new DebugCircle(Point2i(hcenterDiag2,ty),4,Colors::Lime,true));
#endif
						}

						tx = t;

						Point2i pt;
						pt.x = tx;
						pt.y = ty;

						
						LOGD(LOGTAG_QR,"Creating new pattern.");
						FinderPattern * fp = new FinderPattern();
						
						fp->hitCount = 1;
						fp->pt = pt;
						fp->size = bw[0] + bw[1] + bw[2] + bw[3] + bw[4];

						if (fpv->push_back_pattern(fp))
						{
							
							LOGD(LOGTAG_QR,"Pattern already exists, deleting new pattern.");
							//Pattern already found, so delete this one
							delete fp;
							LOGD(LOGTAG_QR,"Delete successful");
						}
						else
						{
							for (int arrayCopy=0;arrayCopy<5;arrayCopy++)
							{
								fp->patternWidths[arrayCopy] = bw[arrayCopy];
							}
							LOGD(LOGTAG_QR,"Copied size array");
						}
												

						if (skip)
						{
							int s = SkipHeuristic(fpv);

							if (s > 0)
							{
								y = y + s - bw[2] - 2;
								skip = false; /* Do not skip again! */

								if (y > M.rows) /* Something went wrong, back up. */
								{
									y = y - s + bw[2] + 2;
								}
							}
						}

					} else
					{
#ifdef DRAW_FP_DEBUG_LAYER
						debugVector.push_back(new DebugCircle(Point2i(tx,y),3,Colors::PeachPuff,true));
#endif
					}
				}

				k = 3;
				bw[0] = bw[2];
				bw[1] = bw[3];
				bw[2] = bw[4];
				bw[3] = 0;
				bw[4] = 0;
			}
		}
	}
	LOGV(LOGTAG_QR,"Exiting FFP");
}

bool FinderPatternHelper::CheckRatios(int * newBw, int * oldBw, float scoreModifier)
{
	int newModuleSum = 0, oldModuleSum = 0;
	bool useOld = oldBw != NULL;

	for (int i = 0; i < 5; ++i)
	{
		if (newBw[i] == 0)
		{
			return false; 
		}
		newModuleSum += newBw[i];
		if (useOld)
		{
			oldModuleSum += oldBw[i];
		}
	}
		
	if (newModuleSum < 7)
	{
		return false; 
	}
	int newModuleSize = (int)round((float)newModuleSum / 7.0f);
	

	int variance	= newModuleSize >> 2;
	int variance2	= newModuleSize >> 1;

	int a = newBw[0];	int b = newBw[1];	int c = newBw[2];	int d = newBw[3];	int e = newBw[4];

	int symmetryScore = (abs(a - e) < variance2)*50 + (abs(b - d) < variance2)*50;

	int sizeScore = 		
			(abs(newModuleSize - a) < variance2)*14 + 
			(abs(newModuleSize - b) < variance2)*14 + 
			(abs(newModuleSize - d) < variance2)*14 + 
			(abs(newModuleSize - e) < variance2)*14 + 
			(abs((3 * newModuleSize) - c) < (3 * variance2))*42; 

	bool comparisonScore = (c > b && c > d && c > a && c > e);

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
	
	if (score > ((int)(MinimumFinderPatternScore*scoreModifier)))
	{		
		//LOGV(LOGTAG_QR,"Found pattern! Scores were: Sym=%d,Size=%d,Compare=%d,OldNewScore=%d < MinimumFinderPatternScore=%d",symmetryScore,sizeScore,comparisonScore,oldNewCompareScore,MinimumFinderPatternScore);
		return true;
	}
	else
	{
		//LOGV(LOGTAG_QR,"CheckRatios F: New= %d,%d,%d,%d,%d",newBw[0],newBw[1],newBw[2],newBw[3],newBw[4]);
		//LOGV(LOGTAG_QR,"Scores: Sym=%d,Size=%d,Compare=%d,OldNewScore=%d < MinimumFinderPatternScore=%d",symmetryScore,sizeScore,comparisonScore,oldNewCompareScore,MinimumFinderPatternScore);
		return false;
	}
}

int FinderPatternHelper::SkipHeuristic(FinderPattern_vector * fpv)
{
	long skip = 0;

	if (fpv->HitConfidence() >= 2)
	{
		skip = abs(fpv->at(0)->pt.x - fpv->at(1)->pt.x) - abs(fpv->at(0)->pt.y - fpv->at(1)->pt.y);
		/* Get space between the
		 two "top" finder patterns. */
	}

	return skip;
}



int FinderPatternHelper::FindCenterVertical(const Mat& image, int x, int y, int fpbw[])
{
	//LOGD("FindCenterVertical_Start");
	int bw[5] = { 0 };

	int i = y;

	/* Calculate vertical finder pattern up from the center. */
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

	/* Calculate vertical finder pattern down from the center. */
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
int FinderPatternHelper::FindCenterHorizontal(const Mat& image, int x, int startY, int fpbw[], int yDelta)
{
	int bw[5] = { 0 };

	//LOGD( "FindCenterHorizontal_Start");
	int j = x;
	int y = startY;

	/* Calculate horizontal finder pattern left from the center. */
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

	/* Calculate horizontal finder pattern right from the center. */
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
bool FinderPatternHelper::CheckRatios2(int newBw[], int oldBw[])
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
		//LOGD("FinderPatternHelper","CheckRatio2 F: New= %d,%d,%d,%d,%d",newBw[0],newBw[1],newBw[2],newBw[3],newBw[4]);
		//LOGD("FinderPatternHelper","CheckRatio2 F: Old= %d,%d,%d,%d,%d",oldBw[0],oldBw[1],oldBw[2],oldBw[3],oldBw[4]);
		//LOGD("FinderPatternHelper","Scores: Sym=%d,Size=%d,Compare=%d,OldNew=%d",symmetryScore,sizeScore,comparisonScore,oldNewCompareScore);
		return false;
	}
}
