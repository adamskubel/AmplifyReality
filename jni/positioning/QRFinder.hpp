#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <vector>
#include <android/log.h>
#include "FindPattern.h"

#ifndef QR_FINDER_HPP_
#define QR_FINDER_HPP_

#undef LOG_TAG
#define LOG_TAG "QRFinder"

using namespace cv;
using namespace std;

class QR_vector: public vector<bool>
{
public:
	inline bool SetDimension(long d);
	inline long GetDimension()
	{
		return dimension;
	}

private:
	long dimension;
};

struct QRFinder
{
public:
	static bool locateQRCode(cv::Mat& M, vector<Point_<int>*>& ptList, vector<Point3i>&  debugVector);

private:
	static bool CheckRatios(int bw[]);
	static bool CheckRatios(int bw[], int bw2[]);
	static int FindCenterVertical(const Mat& image, int x, int y, int fpbw[]);
	static int FindCenterHorizontal(const Mat& image, int x, int y, int fpbw[]);
	static void FindFinderPatterns(cv::Mat& M, FINDPATTERN_vector& fpv, vector<Point3i>& debugVector);
	static void TriangleOrder(const FINDPATTERN_vector& fpv, FINDPATTERN& bottom_left, FINDPATTERN& top_left, FINDPATTERN& top_right);
	static int SkipHeuristic(const FINDPATTERN_vector& fpv);
};

#endif /* QR_FINDER_HPP_ */
