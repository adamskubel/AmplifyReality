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
#include "FinderPatternHelper.hpp"
#include "AlignmentPatternHelper.hpp"
#include "positioning/fast/FastTracking.hpp"


using namespace cv;
using namespace std;


class QRFinder
{
public:
	QRFinder();
	~QRFinder();
	QRCode * LocateQRCodes(cv::Mat& M, vector<Drawable*> & debugVector, bool decode);
	int MinimumAlignmentPatternScore;
	int fastThreshold;

private:
	void DoFastDetection(Mat & img, vector<Drawable*> & debugVector);
	static void TriangleOrder(const FinderPattern_vector& fpv, FinderPattern& bottom_left, FinderPattern& top_left, FinderPattern& top_right);
	QRDecoder * qrDecoder;
	
};

#endif 
