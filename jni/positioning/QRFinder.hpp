#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <vector>
#include <android/log.h>
#include "FindPattern.h"
#include "positioning/QRCode.hpp"

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
	static QRCode * locateQRCode(cv::Mat& M, vector<Point3i>&  debugVector);

private:
	static bool CheckRatios(int bw[]);
	static bool CheckRatios(int bw[], int bw2[]);
	static int FindCenterVertical(const Mat& image, int x, int y, int fpbw[]);
	static int FindCenterHorizontal(const Mat& image, int x, int y, int fpbw[]);
	static void FindFinderPatterns(cv::Mat& M, FinderPattern_vector& fpv, vector<Point3i>& debugVector);
	static void FindFinderPatterns_Symmetry(cv::Mat& M, FinderPattern_vector& fpv, vector<Point3i>& debugVector);
	static void TriangleOrder(const FinderPattern_vector& fpv, FinderPattern& bottom_left, FinderPattern& top_left, FinderPattern& top_right);
	static int SkipHeuristic(const FinderPattern_vector& fpv);
	static bool GetEdges(const Mat& image, Point2i start, int xDir, int yDir, int * Q);
	static float CentralSymmetry(int * x);
	static float SquareCharacteristic(int *x, int * y);
	static float RatioCharacteristic(int * x);
	static float SymmetryAlgorithm(int * X, int * Y, int * R, int * S);
	static bool CheckArea(const Mat& image, Point2i center, int bwLengths[], FinderPattern * result);
};

#endif /* QR_FINDER_HPP_ */
