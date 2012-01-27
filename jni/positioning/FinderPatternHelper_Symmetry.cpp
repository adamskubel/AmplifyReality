#include "FinderPatternHelper.hpp"

static void LogArray(int * Array)
{
	LOGD(LOGTAG_QR,"Array=[%d,%d,%d,%d,%d,%d,%d]",Array[0],Array[1],Array[2],Array[3],Array[4],Array[5],Array[6]);
}

//Modified symmetry-based QR finding
/*
1. Check every N rows for Ratio and Symmetry matches. Score must exceed a given value.
2. If score check is successful, then check the local area for the pixel with the greatest score.
The local area is a rectangle centered on the center of the ratio/symmetry match, with length and width given by size of black center.
3. Proceed.
*/
void FinderPatternHelper::FindFinderPatterns_Symmetry(cv::Mat& M, FinderPattern_vector& fpv, vector<Point3i>& debugVector)
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
bool FinderPatternHelper::CheckArea(const Mat& image, Point2i center, int bwLengths[], FinderPattern & result)
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
bool FinderPatternHelper::GetEdges(const Mat& image, Point2i start, int xDir, int yDir, int * Q)
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
float FinderPatternHelper::SymmetryAlgorithm(int * X, int * Y, int * R, int * S)
{
	float totalSymmetry = (CentralSymmetry(X) + CentralSymmetry(R) + CentralSymmetry(Y) + CentralSymmetry(S))/4.0f;
	float totalSquare = (SquareCharacteristic(X,Y) + SquareCharacteristic(R,S))/2.0f;
	float totalRatio = (RatioCharacteristic(X) + RatioCharacteristic(Y) + RatioCharacteristic(R) + RatioCharacteristic(S))/4.0f;

	return (totalSymmetry + totalSquare + totalRatio)/3.0f;
}
float FinderPatternHelper::CentralSymmetry(int * x)
{
	return
		(
			((abs(x[4] - x[0]) - abs(x[0] - x[3])) / abs(x[4]-x[4])) +
			((abs(x[5] - x[0]) + abs(x[0]- x[2])) / abs(x[5] - x[2])) +
			((abs(x[6] - x[0]) - abs(x[0] - x[1])) / abs(x[6]-x[1]))
		) / 3.0f;

}
float FinderPatternHelper::SquareCharacteristic(int *x, int * y)
{
	return
		(
			(2 * ((abs(x[4] - x[1]) - abs(y[4]-y[1]))/(abs(x[4]-x[1]) + abs(y[4]-y[1])))) +
			(2 * ((abs(x[5] - x[2]) - abs(y[5]-y[2]))/(abs(x[5]-x[2]) + abs(y[5]-y[2])))) +
			(2 * ((abs(x[6] - x[3]) - abs(y[6]-y[3]))/(abs(x[6]-x[3]) + abs(y[6]-y[3])))) +
			(2 * ((abs(x[6] - x[1]) - abs(y[6]-y[1]))/(abs(x[6]-x[1]) + abs(y[6]-y[1]))))
		) / 4.0f;
}
float FinderPatternHelper::RatioCharacteristic(int * x)
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
