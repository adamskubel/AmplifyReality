#ifndef FAST_QR_FINDER_HPP_
#define FAST_QR_FINDER_HPP_

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
//#include <opencv2/flann/flann.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <vector>
#include "LogDefinitions.h"
#include "positioning/qrcode/FinderPattern.hpp"
#include "positioning/qrcode/QRCode.hpp"
#include "model/Drawable.hpp"
#include "model/DebugShape.hpp"
#include "positioning/qrcode/QRDecoder.hpp"
#include "controllers/ARControllerDebugUI.hpp"
#include "AmplifyRealityGlobals.hpp"
#include "FastTracking.hpp"
#include <stdlib.h>
#include <set>
#include "display/Colors.hpp"

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

struct QRCorner
{
public:
	Point2i Center;
	Point2i Outer;
	Point2i Inner;

	int Size;

	QRCorner()
	{

	}

	QRCorner(Point2i _Center, Point2i _Outer, Point2i _Inner, int _Size)
	{
		Center = _Center;
		Outer = _Outer;
		Inner = _Inner;
		Size = _Size;
	}
};

namespace FastQR
{
	struct Node
	{
	public:
		Node(KeyPoint kp)
		{
			nodePoint = Mat(1,2,CV_32F);
			nodePoint.at<float>(0,0) = (int)kp.pt.x;
			nodePoint.at<float>(0,1) = (int)kp.pt.y;
			visited = false;
			clusterIndex = -1;
		}

		Node(Point2f pt)
		{
			nodePoint = Mat(1,2,CV_32F);
			nodePoint.at<float>(0,0) = (int)pt.x;
			nodePoint.at<float>(0,1) = (int)pt.y;
			visited = false;
			clusterIndex = -1;
		}

		Mat nodePoint;
		bool visited;
		int clusterIndex;
	};

	struct ThreePointLine
	{
	public:
		ThreePointLine(Point2i _pt0, Point2i _pt1, Point2i _pt2, int _length, float _cosine)
		{
			pt0 = _pt0;
			pt1 = _pt1;
			pt2 = _pt2;
			length = _length;
			cosine = _cosine;
		}

		void DrawDebug(vector<Drawable*> & debugVector);

		Point2i pt0,pt1,pt2;
		int length;
		float cosine;

	};
}


class FastQRFinder
{
	

public: 
	FastQRFinder(ARControllerDebugUI * config);
	void LocateFPCorners(Mat & img, FinderPattern * pattern,  vector<Point2i> & corners, vector<Drawable*> & debugVector);
	void EnhanceQRCodes(Mat & img, QRCode * qrCode, vector<Drawable*> & debugVector);
	void CheckAlignmentPattern(Mat & img, Point2i center, Size2f patternSize, vector<Point2i> & patternPoints, vector<Drawable*> & debugVector);

private:
	void FindPoint(Point2i regionPoint, IDetectableContour * contour, map<int,multimap<int,Point2i>*> & regionMap);
	void GetRandomPoint(map<int,map<int,Point2i>*> & regionMap, Point2i & randomPoint, Point2i & randomPointKey);
	float getBestEdgeSize(int detectorRadius, Mat & img, Point2i imgPoint, int recurseCount);
	ARControllerDebugUI * config;
	double flannTime, pointTime, avgPerPointTime, maxThreshTime, fastTime, clusterTime;
	//double patternTimes[4];

	//Utility methods
	static int GetSquaredDistance(int dx, int dy);
	static int GetDistanceFast(int dx, int dy);
	static int GetSquaredDistance(Point2i pt0, Point2i pt1);
	static int GetDistanceFast(Point2i pt0, Point2i pt1);
	
	//Clustering
	static bool GetNodesInRadius(FastQR::Node* pt, double dblRadius, int nMinPts, int maxPts, vector<FastQR::Node*>& rgpNodesFound, flann::Index * kdIndex, vector<FastQR::Node*> & vecNodes);
	static void ExpandCluster(vector<FastQR::Node*>& rgp, int nCluster, double dblEpsilon, int nMinPts, int maxPts, flann::Index * kdIndex,vector<FastQR::Node*> & vecNodes);
	static void ExpandCluster_Recursive(FastQR::Node* node, int nCluster, double dblEpsilon, int nMinPts, int maxPts, flann::Index * kdIndex,vector<FastQR::Node*> & vecNodes);
	static int RunDBScan(vector<FastQR::Node*> & vecNodes, flann::Index * kdIndex, double flannRadius, int nMinPts,int maxPts);
};

#endif 