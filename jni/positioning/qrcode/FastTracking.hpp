#ifndef FAST_TRACKING_HPP_
#define FAST_TRACKING_HPP_

#include "LogDefinitions.h"
#include <opencv2/core/core.hpp>
#include "FindPattern.h"
#include "QRCode.hpp"
#include "model/DebugShape.hpp"
#include "model/Drawable.hpp"
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/objdetect/objdetect.hpp>

class FastTracking
{
public:
	static void DoFastTracking(Mat & img, QRCode * qrCode, vector<Drawable*> & debugVector);
private:
	static void FastWindow(Mat & img, vector<KeyPoint> & features, Rect window);
	static float GetPointDistance(Point2f p1, Point2f p2);
	//static void ConcentricSegmentation(vector<KeyPoint> & features, Point2i center, float innerMost, float middle, float outerMost, vector<KeyPoint> *& kpVecArray);// map<int,vector<KeyPoint> > & concentricMap);
};

#endif