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
#include <random>


using namespace cv;
using namespace std;


class IDetectableContour
{
public:
	virtual int NumPoints();
	virtual float ChooseNextPoint(vector<Point2f> & options);
	virtual float ScoreNextPoint(Point2f possiblePoint);
	virtual float AddNextPoint(Point2f selectedPoint);

};

class DetectableSquare : public IDetectableContour
{
public:
	DetectableSquare()
	{
		currentPoints = 0;
	}

	int NumPoints()
	{
		return 4;
	};
	float ChooseNextPoint(vector<Point2f> & options)
	{
		vector<Point2f>::iterator pointIterator = options.begin();
		
		//Mat xCoords(1,options.size(),CV_32FC1);
		//Mat yCoords(1,options.size(),CV_32FC1);
		//Mat distances;
		//xCoords.push_back((*pointIterator).x);
		//yCoords.push_back((*pointIterator).y);

		for (;pointIterator != options.end(); pointIterator++)
		{
			
		}

		return 0.0f;
	}
	
	float ScoreNextPoint(Point2f possiblePoint)
	{

	}

	float AddNextPoint(Point2f selectedPoint)
	{
		pointVector.push_back(selectedPoint);
	}

private:
	vector<Point2f> pointVector;
	int currentPoints;
	

};


class FastQRFinder
{

public: 
	FastQRFinder(ARControllerDebugUI * config);
	QRCode * FindQRCodes(Mat & img, vector<Drawable*> & debugVector);

private:
	bool PointInRegion(Point2f point, Point2i cellPosition, Size2i regionSize);
	static Point2i GetRegion(Point2f point, Size2i regionSize);
	ARControllerDebugUI * config;
};

#endif 