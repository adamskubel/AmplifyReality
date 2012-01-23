#include "QRFinder.hpp"
#include "LogDefinitions.h"
#include "QRCode.hpp"
//#include <PerspectiveTransform.h>


int QRFinder::minScore = 141;

QRCode * QRFinder::locateQRCode(cv::Mat& M, vector<Point3i>& debugVector, int _minScore) 
{	
	minScore = _minScore;
	struct timespec start,end;
	SET_TIME(&start);

	long height = M.rows;
	long width = M.cols;

	FinderPattern_vector pattern_store;

	//FindFinderPatterns_Symmetry(M,pattern_store,debugVector);
	FindFinderPatterns(M, pattern_store, debugVector);



	//If 3 patterns are found, we've probably found a QR code. Generate it and return.
	if (pattern_store.size() == 3)
	{
		FinderPattern  top_left, top_right, bottom_left, bottom_right;
		TriangleOrder(pattern_store, bottom_left, top_left, top_right);

		bottom_right.pt.x = (top_right.pt.x - top_left.pt.x) + bottom_left.pt.x;
		bottom_right.pt.y = (top_right.pt.y - top_left.pt.y) + bottom_left.pt.y;
		bottom_right.hitCount = -1;
		bottom_right.size = 0;
				
		SET_TIME(&end);
		LOG_TIME("QR Search(Found)", start, end);
		vector<FinderPattern*> * patternVector = new vector<FinderPattern*>();

		patternVector->push_back(new FinderPattern(top_left));
		patternVector->push_back(new FinderPattern(top_right));
		patternVector->push_back(new FinderPattern(bottom_right));
		patternVector->push_back(new FinderPattern(bottom_left));

		return new QRCode(patternVector,true);

	} 
	
	//Less or more than three patterns were found, so just return a vector of all the finder patterns 

	vector<FinderPattern*> * patternVector = new vector<FinderPattern*>();
	for (int i=0;i<pattern_store.size();i++)
	{		
		patternVector->push_back(new FinderPattern(pattern_store.at(i)));
	}

	
	SET_TIME(&end);
	LOG_TIME("QR Search(NotFound)", start, end);
	return new QRCode(patternVector,false);
}

//Modified symmetry-based QR finding
/*
1. Check every N rows for Ratio and Symmetry matches. Score must exceed a given value. 
2. If score check is successful, then check the local area for the pixel with the greatest score. 
The local area is a rectangle centered on the center of the ratio/symmetry match, with length and width given by size of black center.
3. Proceed.
*/
void QRFinder::FindFinderPatterns_Symmetry(cv::Mat& M, FinderPattern_vector& fpv, vector<Point3i>& debugVector)
{

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
				if (CheckRatios(bw,NULL))
				{

					int tx = (x - bw[4] - bw[3]) - (bw[2] / 2); /* Calculate temporary horizontal center from end of pattern. */
					
					FinderPattern result;
					bool foundPattern = CheckArea(M,Point2i(tx,y),bw, result);
					

					if (foundPattern)
					{
						LOGD(LOGTAG_QR,"Found");
						LOGD(LOGTAG_QR,"Found pattern at (%d,%d), size=%ld",result.pt.x,result.pt.y,result.size);
						fpv.push_back_pattern(result);

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

					} else
					{
						debugVector.push_back(Point3i(x, y, 5));
					}

					/*if (result != NULL)
					{
						LOGD(LOGTAG_QR,"Deleting result");
						delete result;
					}*/
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
}
static void LogArray(int * Array)
{
	LOGD(LOGTAG_QR,"Array=[%d,%d,%d,%d,%d,%d,%d]",Array[0],Array[1],Array[2],Array[3],Array[4],Array[5],Array[6]);
}




bool QRFinder::CheckRatios(int * newBw, int * oldBw)
{
	int newModuleSum = 0, oldModuleSum = 0;
	bool useOld = oldBw != NULL;
	//if (useOld)
	//	LOGD(LOGTAG_QR,"Using previous finder pattern data");
	//else
	//	LOGD(LOGTAG_QR,"No previous finder patterns found");

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
		//int oldModuleSize = (int)round((float)oldModuleSum / 7.0f);
		oldNewCompareScore =
			(
				(abs(oldModuleSum-newModuleSum) < (oldModuleSum >> 2))*20 +
				(abs(newBw[2] - oldBw[2]) < oldBw[2] >> 1)*20
			);
	
	}

	int score = comparisonScore * (sizeScore + symmetryScore + oldNewCompareScore);	
	
	if (score > minScore)
	{		
		LOGV(LOGTAG_QR,"Found pattern! Scores were: Sym=%d,Size=%d,Compare=%d,OldNewScore=%d < MinScore=%d",symmetryScore,sizeScore,comparisonScore,oldNewCompareScore,minScore);
		return true;
	}
	else
	{
		LOGV(LOGTAG_QR,"CheckRatios F: New= %d,%d,%d,%d,%d",newBw[0],newBw[1],newBw[2],newBw[3],newBw[4]);
		LOGV(LOGTAG_QR,"Scores: Sym=%d,Size=%d,Compare=%d,OldNewScore=%d < MinScore=%d",symmetryScore,sizeScore,comparisonScore,oldNewCompareScore,minScore);
		return false;
	}
}

void QRFinder::FindFinderPatterns(cv::Mat& M, FinderPattern_vector& fpv, vector<Point3i>& debugVector)
{
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
				if (fpv.size() > 0)
				{
					result = CheckRatios(bw,fpv.back().patternWidths);
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
						int t = FindCenterHorizontal(M, tx, ty, bw); /* Calculate it again. */

						if (t != 0)
						{
							tx = t;
						}

						Point2i pt;
						pt.x = tx;
						pt.y = ty;

						FinderPattern fp;
						for (int arrayCopy=0;arrayCopy<5;arrayCopy++)
						{
							fp.patternWidths[arrayCopy] = bw[arrayCopy];
						}
						fp.hitCount = 1;
						fp.pt = pt;
						fp.size = bw[0] + bw[1] + bw[2] + bw[3] + bw[4];
						fpv.push_back_pattern(fp);

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
						debugVector.push_back(Point3i(x, y, 5));
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
}

int QRFinder::SkipHeuristic(const FinderPattern_vector& fpv)
{
	long skip = 0;

	if (fpv.HitConfidence() >= 2)
	{
		skip = abs(fpv[0].pt.x - fpv[1].pt.x) - abs(fpv[0].pt.y - fpv[1].pt.y);
		/* Get space between the
		 two "top" finder patterns. */
	}

	return skip;
}
//
//bool QR_vector::SetDimension(long d)
//{
//	bool good = true;
//
//	if (d < 21 || (d & 0x03) != 1)
//	{
//		good = false;
//	} else
//	{
//		dimension = d;
//	}
//
//	return good;
//}

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
		LOGD("QRFinder","CheckRatio2 F: New= %d,%d,%d,%d,%d",newBw[0],newBw[1],newBw[2],newBw[3],newBw[4]);
		LOGD("QRFinder","CheckRatio2 F: Old= %d,%d,%d,%d,%d",oldBw[0],oldBw[1],oldBw[2],oldBw[3],oldBw[4]);
		LOGD("QRFinder","Scores: Sym=%d,Size=%d,Compare=%d,OldNew=%d",symmetryScore,sizeScore,comparisonScore,oldNewCompareScore);
		return false;
	}
}

int QRFinder::FindCenterVertical(const Mat& image, int x, int y, int fpbw[])
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
		LOGD("QRFinder","VertCenter CheckRatio2 Failed: %d,%d,%d,%d,%d",bw[0],bw[1],bw[2],bw[3],bw[4]);
		return 0;
	}

	//LOGD( "FindCenterVertical_End");
	return (i - bw[4] - bw[3]) - (bw[2] / 2);
}

int QRFinder::FindCenterHorizontal(const Mat& image, int x, int y, int fpbw[])
{
	int bw[5] = { 0 };

	//LOGD( "FindCenterHorizontal_Start");
	int j = x;

	/* Calculate horizontal finder pattern left from the center. */
	for (; j > 0 && image.at<unsigned char>(y, j) == 0; ++bw[2], --j)
		;
	if (j == 0)
	{
		return 0;
	}

	for (; j > 0 && image.at<unsigned char>(y, j) != 0 && bw[1] < fpbw[2]; ++bw[1], --j)
		;
	if (j == 0 || bw[1] >= fpbw[2])
	{
		return 0;
	}

	for (; j >= 0 && image.at<unsigned char>(y, j) == 0 && bw[0] < fpbw[2]; ++bw[0], --j)
		;
	if (bw[0] >= fpbw[2])
	{
		return 0;
	}

	/* Calculate horizontal finder pattern right from the center. */
	j = x + 1;
	for (; j < image.cols && image.at<unsigned char>(y, j) == 0; ++bw[2], ++j)
		;
	if (j == image.rows)
	{
		return 0;
	}

	for (; j < image.cols && image.at<unsigned char>(y, j) != 0 && bw[3] < fpbw[2]; ++bw[3], ++j)
		;
	if (j == image.cols || bw[3] >= fpbw[2])
	{
		return 0;
	}

	for (; j < image.cols && image.at<unsigned char>(y, j) == 0 && bw[4] < fpbw[2]; ++bw[4], ++j)
		;
	if (bw[4] >= fpbw[2])
	{
		return 0;
	}

	if (CheckRatios(bw,NULL) == false)
	{
		return 0;
	}

	//LOGD("FindCenterHorizontal_End");
	return (j - bw[4] - bw[3]) - (bw[2] / 2);
}

void QRFinder::TriangleOrder(const FinderPattern_vector& fpv, FinderPattern& bottom_left, FinderPattern& top_left, FinderPattern& top_right)
{
	long d0to1 = FinderPattern::Distance(fpv[0], fpv[1]);
	long d1to2 = FinderPattern::Distance(fpv[1], fpv[2]);
	long d0to2 = FinderPattern::Distance(fpv[0], fpv[2]);



	if (d1to2 > d0to1 && d1to2 > d0to2)
	{
		top_left = fpv[0];
		bottom_left = fpv[1];
		top_right = fpv[2];
	} else if (d0to1 > d1to2 && d0to1 > d0to2)
	{
		top_left = fpv[2];
		bottom_left = fpv[1];
		top_right = fpv[0];
	}  else if (d0to2 > d1to2 && d0to2 > d0to1)
	{
		top_left = fpv[1];
		bottom_left = fpv[0];
		top_right = fpv[2];
	}
	else
	{
		LOGW("QRFinder","Unable to resolve finder pattern order by distance");
		//no clear winner, probably some significant perspective. 
		if (FinderPattern::AcuteAngleGeometry(fpv[0], fpv[1], fpv[2]) >= 0)
		{
			top_left = fpv[0];
			top_right = fpv[1];
			bottom_left = fpv[2];
		}
		else if (FinderPattern::AcuteAngleGeometry(fpv[1], fpv[0], fpv[2]) >= 0)
		{
			top_left = fpv[1];
			top_right = fpv[0];
			bottom_left = fpv[2];
		}
		else if (FinderPattern::AcuteAngleGeometry(fpv[2], fpv[0], fpv[1]) >= 0)
		{
			top_left = fpv[2];
			top_right = fpv[0];
			bottom_left = fpv[1];
		}
		else
		{
			LOGE("Error determining finder pattern order. This shouldn't happen.");
			top_left = fpv[0];
			bottom_left = fpv[1];
			top_right = fpv[2];
		}
	}

	if (FinderPattern::AcuteAngleGeometry(bottom_left, top_left, top_right) < 0)
	{
		FinderPattern t = bottom_left;
		bottom_left = top_right;
		top_right = t;
	}

	return;
}


bool QRFinder::CheckArea(const Mat& image, Point2i center, int bwLengths[], FinderPattern & result)
{
	int xRange = (int)(bwLengths[2] * 0.4f);
	int yRange = bwLengths[2];

	Rect area = Rect(center.x - xRange/2,center.y - yRange/2, xRange, yRange);
	LOGD(LOGTAG_QR,"Checking area at (%d,%d), w=%d,h=%d",area.x,area.y,area.width,area.height);
	//Check bounds
	if (area.x < 0) area.x = 0;
	if (area.y < 0) area.y = 0;
	if (area.x + area.width >= image.cols) area.width = (image.cols-area.x)-1;
	if (area.y + area.height >= image.rows) area.height = (image.rows-area.y)-1;
	
	int X[7] = {0,0,0,0,0,0,0};
	int R[7] = {0,0,0,0,0,0,0};
	int Y[7] = {0,0,0,0,0,0,0};
	int S[7] = {0,0,0,0,0,0,0};


	Point2i minPoint;
	float minScore = 1000;
	float minSize = 0;


	bool next = false;
	for (int y=area.y;y<(area.y+area.height) && !next;y++)
	{
		for (int x=area.x;x<(area.x+area.width);x++)
		{
			bool success = GetEdges(image,Point2i(x,y),1,0,X);
			success = success && GetEdges(image,Point2i(x,y),0,1,Y);			
			success = success && GetEdges(image,Point2i(x,y),1,1,R);
			success = success && GetEdges(image,Point2i(x,y),1,-1,S);
					
			if (!success)
				continue;
			
			LogArray(X);			
			LogArray(Y);			
			LogArray(R);			
			LogArray(S);

			float score = SymmetryAlgorithm(X,Y,R,S);
			LOGD(LOGTAG_QR,"Score is %f",score);
			if (score < minScore)
			{
				minScore = score;
				minPoint = Point2i(x,y);
				minSize = abs(X[1]-X[6]); 
				if (score == 0)
				{
					next = true;
					break;
				}
			}
		}
	}

	if (minScore < 0.8f)
	{
		result.size = (long)minSize;
		result.pt = minPoint;
		LOGD(LOGTAG_QR,"X:Found pattern at (%d,%d), size=%ld",result.pt.x,result.pt.y,result.size);
		return true;
	}
	return false;
}

bool QRFinder::GetEdges(const Mat& image, Point2i start, int xDir, int yDir, int * Q)
{	
	if (image.at<unsigned char>(start.y,start.x) != 0)
	{
		return false;
	}
	int maxCount = 100;

	

	int edgeCount = 4, totalCount = 0;
	bool next = false, yMode = xDir == 0;	

	if (yMode)
		Q[0] = start.y;
	else
		Q[0] = start.x;

	for (int y = start.y, x = start.x; y<image.rows && x<image.cols && x >= 0 && y>= 0 && !next; y+=yDir, x+=xDir)
	{
		//const unsigned char * Mi = image.ptr<unsigned char>(y);
		unsigned char px = image.at<unsigned char>(y,x);
		totalCount++;
		switch (px)
		{

		case 0:
			if (edgeCount == 5)
			{
				//LOGD(LOGTAG_QR,"Detected edge %d at x=%d",edgeCount,x);
				edgeCount ++;
				Q[5] = (yMode) ? y : x;
			}
			break;					
		case 255:
			switch (edgeCount)
			{
			case 4:		
				Q[4] = (yMode) ? y : x;
				//LOGD(LOGTAG_QR,"Detected edge %d at x=%d",edgeCount,x);
				edgeCount++;
				break;
			case 6:
				Q[6] = (yMode) ? y : x;
				//LOGD(LOGTAG_QR,"Detected edge %d at x=%d",edgeCount,x);
				edgeCount++;
				next = true;
				break;
			}

			if (totalCount > maxCount)
				return false;
		}
	}

	//LOGD(LOGTAG_QR,"Doing negative direction");

	edgeCount = 3, totalCount = 0;
	for (int y = start.y, x = start.x; y<image.rows && x<image.cols && x >= 0 && y>= 0; y-=yDir, x-=xDir)
	{
		//const unsigned char * Mi = image.ptr<unsigned char>(y);
		unsigned char px = image.at<unsigned char>(y,x);
		totalCount++;
		switch (px)
		{

		case 0:
			if (edgeCount ==2)
			{
				//LOGD(LOGTAG_QR,"Detected edge %d at x=%d",edgeCount,x);
				edgeCount --;
				Q[2] = (yMode) ? y : x;
			}
			break;					
		case 255:
			switch (edgeCount)
			{
			case 3:		
				Q[3] = (yMode) ? y : x;
				//LOGD(LOGTAG_QR,"Detected edge %d at x=%d",edgeCount,x);
				edgeCount--;
				break;
			case 1:
				Q[1] = (yMode) ? y : x;
				//LOGD(LOGTAG_QR,"Detected edge %d at x=%d",edgeCount,x);
				//edgeCount--;
				return true;
				break;
			}

			if (totalCount > maxCount)
				return false;
		}
	}
	return false;
}

float QRFinder::SymmetryAlgorithm(int * X, int * Y, int * R, int * S)
{	
	float totalSymmetry = (CentralSymmetry(X) + CentralSymmetry(R) + CentralSymmetry(Y) + CentralSymmetry(S))/4.0f;
	float totalSquare = (SquareCharacteristic(X,Y) + SquareCharacteristic(R,S))/2.0f;
	float totalRatio = (RatioCharacteristic(X) + RatioCharacteristic(Y) + RatioCharacteristic(R) + RatioCharacteristic(S))/4.0f;

	return (totalSymmetry + totalSquare + totalRatio)/3.0f;
}

float QRFinder::CentralSymmetry(int * x)
{
	return 		
		(
			((abs(x[4] - x[0]) - abs(x[0] - x[3])) / abs(x[4]-x[4])) + 
			((abs(x[5] - x[0]) + abs(x[0]- x[2])) / abs(x[5] - x[2])) + 
			((abs(x[6] - x[0]) - abs(x[0] - x[1])) / abs(x[6]-x[1]))
		) / 3.0f;

}

float QRFinder::SquareCharacteristic(int *x, int * y)
{
	return 
		(
			(2 * ((abs(x[4] - x[1]) - abs(y[4]-y[1]))/(abs(x[4]-x[1]) + abs(y[4]-y[1])))) + 
			(2 * ((abs(x[5] - x[2]) - abs(y[5]-y[2]))/(abs(x[5]-x[2]) + abs(y[5]-y[2])))) + 
			(2 * ((abs(x[6] - x[3]) - abs(y[6]-y[3]))/(abs(x[6]-x[3]) + abs(y[6]-y[3])))) + 
			(2 * ((abs(x[6] - x[1]) - abs(y[6]-y[1]))/(abs(x[6]-x[1]) + abs(y[6]-y[1])))) 
		) / 4.0f;
}

float QRFinder::RatioCharacteristic(int * x)
{
	return
		(
			abs((x[5]-x[1])/(x[6]-x[2]) - 1) + 
			abs((x[4]-x[1])/(x[6]-x[3]) - 1) + 
			abs((x[3]-x[1])/(x[6]-x[4]) - 1) + 
			abs((x[5]-x[3])/(x[6]-x[4]) - 2) + 
			abs((x[4]-x[2])/(x[3]-x[1]) - 2) + 
			abs((x[3]-x[1])/(x[5]-x[4]) - 1) +
			abs((x[2]-x[1])/(x[6]-x[5]) - 1)  
		)/7.0f;
}


//long module_size = ((pattern_store[0].size + pattern_store[1].size + pattern_store[2].size) / 3) / 7;

//long dab = FinderPattern::Distance(top_left, top_right) / module_size;
//long dac = FinderPattern::Distance(top_left, bottom_left) / module_size;

//		if (!qr_store.SetDimension(((dab + dac) / 2) + 7))
//		{
//			if (!qr_store.SetDimension(((dab + dac) / 2) + 8))
//			{
//				if (!qr_store.SetDimension(dab + 7))
//				{
//					if (!qr_store.SetDimension(dac + 7))
//					{
//						success = false;
//					}
//				}
//			}
//		}
//
//		if (success != false)
//		{
//			long dimension = qr_store.GetDimension();
//
//			/* Setup a square that has the computed locations of the finder patterns
//			 for the coorasponding detected finder pattern locations. */
//			POINT math_top_left, math_top_right, math_bottom_left, math_bottom_right;
//
//			math_top_left.x = math_top_left.y = (module_size * 7) / 2;
//
//			math_top_right.x = (module_size * dimension) - ((module_size * 7) / 2);
//			math_top_right.y = math_top_left.x;
//
//			math_bottom_left.x = math_top_left.x;
//			math_bottom_left.y = math_top_right.x;
//
//			math_bottom_right.x = math_bottom_right.y = math_top_right.x;

//			PerspectiveTransform transform = PerspectiveTransform::QuadrilateralToQuadrilateral(math_top_left, math_top_right, math_bottom_right, math_bottom_left, top_left.pt,
//					top_right.pt, bottom_right.pt, bottom_left.pt);

//			/* Read the bits. */
//			{
//				long half_module_size = module_size >> 1;
//				for (long i = 0; i < dimension; ++i)
//				{
//					for (long j = 0; j < dimension; ++j)
//					{
//						POINT pt;
//						pt.x = (j * module_size) + half_module_size;
//						pt.y = (i * module_size) + half_module_size;
//
//						if (!transform.TransformPoint(pt))
//						{
//							success = false;
//							/* Emergency breaking will lead to memory
//							 leaking of the image data */
//							// return false;
//						}
//
//						qr_store.push_back(M.at<unsigned char>(pt.y,pt.x) == 0);
//					}
//				}
//			}
//		}

