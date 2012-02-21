#ifndef QR_FINDER_HPP_
#define QR_FINDER_HPP_

#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <vector>
#include "LogDefinitions.h"
#include "FindPattern.h"
#include "QRCode.hpp"
#include "model/Drawable.hpp"
#include "model/DebugShape.hpp"
#include "QRDecoder.hpp"
#include "AlignmentPatternHelper.hpp"
#include "positioning/fast/FastTracking.hpp"
#include "controllers/ARControllerDebugUI.hpp"


using namespace cv;
using namespace std;


class QRFinder
{
public:
	QRFinder(ARControllerDebugUI * debugUI);
	~QRFinder();
	QRCode * LocateQRCodes(cv::Mat& M, vector<Drawable*> & debugVector, bool decode);
	int MinimumAlignmentPatternScore;
	static int MinimumFinderPatternScore;

private:
	//void FindFinderPatterns(cv::Mat& M, FinderPattern_vector * fpv, vector<Drawable*> & debugVector);
	void FindFinderPatterns(cv::Mat& M, vector<FinderPattern*> & fpv, vector<Drawable*> & debugVector);
	void DoFastDetection(Mat & img, vector<Drawable*> & debugVector);
	static bool CheckRatios(int * bw, int  * oldBw, float scoreModifier = 1.0f);
	static bool CheckRatios2(int bw[], int bw2[]);
	static int FindCenterVertical(const Mat& image, int x, int y, int fpbw[], vector<Drawable*> & debugVector, int debugLevel);
	static int FindCenterHorizontal(const Mat& image, int x, int y, int fpbw[], int yDelta = 0);
	static bool GetEdges(const Mat& image, Point2i start, int xDir, int yDir, int * Q);
	//static int SkipHeuristic(FinderPattern_vector * fpv);

	double finderPatternTime;
	ARControllerDebugUI * config;
	QRDecoder * qrDecoder;	
	
};

#endif 
