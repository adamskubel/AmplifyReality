#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <vector>
#include <android/log.h>
#include "FindPattern.h"
#include "positioning/QRCode.hpp"
#include "model/Drawable.hpp"
#include "model/DebugShape.hpp"
#include "QRDecoder.hpp"

#include "FinderPatternHelper.hpp"
#include "AlignmentPatternHelper.hpp"

#ifndef QR_FINDER_HPP_
#define QR_FINDER_HPP_

#undef LOG_TAG
#define LOG_TAG "QRFinder"

using namespace cv;
using namespace std;


struct QRFinder
{
public:
	static QRCode * LocateQRCodes(cv::Mat& M, vector<Drawable*> & debugVector);
	static int MinimumAlignmentPatternScore;

private:
	static void TriangleOrder(const FinderPattern_vector& fpv, FinderPattern& bottom_left, FinderPattern& top_left, FinderPattern& top_right);

	
};

#endif 
