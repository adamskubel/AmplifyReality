#ifndef FAST_QR_FINDER_HPP_
#define FAST_QR_FINDER_HPP_

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <vector>
#include "LogDefinitions.h"
#include "positioning/qrcode/FindPattern.h"
#include "positioning/qrcode/QRCode.hpp"
#include "model/Drawable.hpp"
#include "model/DebugShape.hpp"
#include "positioning/qrcode/QRDecoder.hpp"
#include "controllers/ARControllerDebugUI.hpp"
#include "AmplifyRealityGlobals.hpp"
#include "FastTracking.hpp"
#include <stdlib.h>
#include <set>

using namespace cv;
using namespace std;


class IDetectableContour : public Drawable
{
public:
	virtual int NumPoints()
	{
		return 0;
	}
	virtual float ChooseNextPoint(vector<Point2i> & options, Point2i & selectedPoint, map<int,multimap<int,Point2i>*> & regionMap)
	{
		return 0;
	}

	virtual float AddNextPoint(Point2i selectedPoint)
	{
		return 0;
	}
	virtual bool IsComplete()
	{
		return false;
	}

	virtual void Draw(cv::Mat * rgbaImage)
	{
	}
};

class DetectableSquare : public IDetectableContour
{
public:
	DetectableSquare();

	int NumPoints();
	float ChooseNextPoint(vector<Point2i> & options, Point2i & selectedPoint, map<int,multimap<int,Point2i>*> & regionMap);
	float AddNextPoint(Point2i selectedPoint);
	bool IsComplete();
	void Draw(cv::Mat * rgbaImage);

private:
	vector<Point2i> pointVector;
	float AngleDirectionTest(Point2i testPoint);
	bool DistanceTest(Point2i p0, Point2i p1);
	bool isClockwise;
	float avgDistance, distanceError;
	bool validateSquare(map<int,multimap<int,Point2i>*> & regionMap);
	const static float minPointSpacing = 10.0f;
	const static float maxCosine = 0.5f;
	

};


class FastQRFinder
{

public: 
	FastQRFinder(ARControllerDebugUI * config);
	QRCode * FindQRCodes(Mat & img, Mat & binaryImg, vector<Drawable*> & debugVector);

private:
	bool PointInRegion(Point2i point, Point2i cellPosition, Size2i regionSize);
	void FindPoint(Point2i regionPoint, IDetectableContour * contour, map<int,multimap<int,Point2i>*> & regionMap);
	void SegmentByQuantity(map<int,multimap<int,Point2i>*> & regionMap, map<int,multimap<int,Point2i>*> & quantityMap);
	void GetRandomPoint(map<int,map<int,Point2i>*> & regionMap, Point2i & randomPoint, Point2i & randomPointKey);
	float getBestEdgeSize(int detectorRadius, Mat & img, Point2i imgPoint, int recurseCount);
	static Point2i GetRegion(Point2i point, Size2i regionSize);
	bool IsEdge(int detectorRadius, Mat & img, Point2i imgPoint, Point2i & exitPoint);
	ARControllerDebugUI * config;
	int GetCornerType(Point2i point, unsigned char threshold, Mat & img);

	void FindContours(Mat & img, Point2i startPoint, vector<vector<Point2i> > & contourPoints, map<int,map<int,Point2i>*> & regionMap);//, map<float, set<float> > & foundPoints);
};

#endif 