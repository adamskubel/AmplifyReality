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
	static void DoSquareTracking(Mat & image, QRCode * qrCode, vector<Drawable*> & debugVector);
	static void DoFastTracking(Mat & img, QRCode * qrCode, vector<Drawable*> & debugVector);
private:
	static void findSquares( const Mat& image, vector<vector<Point> >& squares );
	static void FastWindow(Mat & img, vector<KeyPoint> & features, Rect window);
	static float GetPointDistance(Point2f p1, Point2f p2);
	static float getPolyArea(Point2f a, Point2f b, Point2f c);
	static float getArea(vector<Point2f> & testPoints);
	static float getCosine(vector<Point2f> & testPoints);
	static float angle(Point2i p1, Point2i p2, Point2i p0);

	//static void ConcentricSegmentation(vector<KeyPoint> & features, Point2i center, float innerMost, float middle, float outerMost, vector<KeyPoint> *& kpVecArray);// map<int,vector<KeyPoint> > & concentricMap);
};

#endif