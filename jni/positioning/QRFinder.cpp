#include "QRFinder.hpp"
#include "LogDefinitions.h"
#include "QRCode.hpp"
//#include <PerspectiveTransform.h>



QRCode * QRFinder::locateQRCode(cv::Mat& M, vector<Point3i>& debugVector) 
{	
	struct timespec start,end;
	SET_TIME(&start);

	long height = M.rows;
	long width = M.cols;

	FinderPattern_vector pattern_store;

	FindFinderPatterns(M, pattern_store, debugVector);

	//Less or more than three patterns were found, so just return a vector of all the finder patterns 


	//If 3 patterns are found, we've probably found a QR code. Generate it and return.
	if (pattern_store.size() == 3)
	{
		FinderPattern  top_left, top_right, bottom_left, bottom_right;
		TriangleOrder(pattern_store, bottom_left, top_left, top_right);

		bottom_right.pt.x = (top_right.pt.x - top_left.pt.x) + bottom_left.pt.x;
		bottom_right.pt.y = (top_right.pt.y - top_left.pt.y) + bottom_left.pt.y;
		bottom_right.hitCount = -1;
		bottom_right.size = 0;

	/*	Point_<int>* points = new Point_<int> [4];
		points[0].x = (int) top_left.pt.x;
		points[0].y = (int) top_left.pt.y;

		points[1].x = (int) top_right.pt.x;
		points[1].y = (int) top_right.pt.y;

		points[2].x = (int) bottom_right.pt.x;
		points[2].y = (int) bottom_right.pt.y;

		points[3].x = (int) bottom_left.pt.x;
		points[3].y = (int) bottom_left.pt.y;*/

		
		SET_TIME(&end);
		LOG_TIME("QR Search(Found)", start, end);
		vector<FinderPattern*> * patternVector = new vector<FinderPattern*>();

		patternVector->push_back(new FinderPattern(top_left));
		patternVector->push_back(new FinderPattern(top_right));
		patternVector->push_back(new FinderPattern(bottom_right));
		patternVector->push_back(new FinderPattern(bottom_left));

		return new QRCode(patternVector,true);

	} 
	
	vector<FinderPattern*> * patternVector = new vector<FinderPattern*>();
	for (int i=0;i<pattern_store.size();i++)
	{		
		patternVector->push_back(new FinderPattern(pattern_store.at(i)));
	}

	
	SET_TIME(&end);
	LOG_TIME("QR Search(NotFound)", start, end);
	return new QRCode(patternVector,false);
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
				if (CheckRatios(bw))
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

bool QR_vector::SetDimension(long d)
{
	bool good = true;

	if (d < 21 || (d & 0x03) != 1)
	{
		good = false;
	} else
	{
		dimension = d;
	}

	return good;
}

bool QRFinder::CheckRatios(int bw[])
{

	int modules_sum = 0;
	for (int i = 0; i < 5; ++i)
	{
		if (bw[i] == 0)
		{
			return false; /* Oops, one module size was zero. */
		}

		modules_sum += bw[i];
	}

	if (modules_sum < 7)
	{
		return false; /* Minimum size check failed. */
	}

	int module_size = (int)round((float)modules_sum / 7.0f);

	/* Allow less than 25% variance from 1-1-3-1-1 ratios. */
	int variance	= module_size >> 2;
	int variance2	= module_size >> 1;

	int a = bw[0];
	int b = bw[1];
	int c = bw[2];
	int d = bw[3];
	int e = bw[4];

	

	if(
		(abs(module_size - a) < variance2) && 
		(abs(module_size - b) < variance2) && 
		(abs(module_size - d) < variance2) && 
		(abs(module_size - e) < variance2) && 
		(abs(a - e) < variance2) &&
		(abs(b - d) < variance2) &&
		(c > b && c > d && c > a && c > e) && 
		(abs((3 * module_size) - c) < (3 * variance2)))
	{
		//LOGD("QRFinder","CheckRatios Success: a=%d,b=%d,c=%d,d=%d,e=%d,msize=%d,var1=%d,var2=%d",a,b,c,d,e,module_size,variance,variance2);
		return true;
	}
	else
	{
		//LOGD("QRFinder","CheckRatios Failed: a=%d,b=%d,c=%d,d=%d,e=%d,msize=%d,var1=%d,var2=%d",a,b,c,d,e,module_size,variance,variance2);
	}
	return false;
	//return 
	//	abs(module_size - a) < variance && 
	//	abs(module_size - b) < variance && 
	//	abs((3 * module_size) - c) < (3 * variance) && 
	//	abs(module_size - d) < variance && 
	//	abs(module_size - e) < variance;
}

bool QRFinder::CheckRatios(int newBw[], int oldBw[])
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

	if (CheckRatios(bw,fpbw) == false)
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

	if (CheckRatios(bw) == false)
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

