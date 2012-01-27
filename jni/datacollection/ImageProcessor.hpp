#include <LogDefinitions.h>
#include <ExceptionCodes.hpp>
#include <AmplifyRealityGlobals.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <model/FrameItem.hpp>
#include "positioning/qrcode/QRCode.hpp"


#ifndef IMGPROCESSOR_HPP_
#define IMGPROCESSOR_HPP_

using namespace cv;
using namespace std;

class ImageProcessor
{
public:
	static void SimpleThreshold(Mat * grayImage, Mat * binaryImage);
	static void FeedbackBinarization(FrameItem * item);
	static void LocalizedThreshold(FrameItem * item);

private:
	static void localThresholding(Mat & inputImg, Mat & outputImg, int windowWidth, int windowHeight);
	static void calculateWindow(FrameItem item, int * windowWidth, int * windowHeight);
	static void WindowedThreshold(Mat & inputImg, Mat & outputImg, Rect window);
	static Rect createWindow(QRCode * qrCode);
	static void GetPointAttributes(vector<Point2i> points, Point2i & centroid, int & distance);

};
#endif