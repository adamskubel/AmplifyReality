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
#include <map>


using namespace cv;
using namespace std;


class QRFinder
{
public:
	QRFinder(ARControllerDebugUI * debugUI);
	~QRFinder();
	QRCode * LocateQRCodes(cv::Mat& M, vector<Drawable*> & debugVector, bool decode);

private:
	//Finder patterns
	void FindFinderPatterns(cv::Mat& inputImg, vector<FinderPattern*> & fpv, vector<Drawable*> & debugVector);
	int CheckRatios(int * bw, int  * oldBw, float scoreModifier = 1.0f);
	int FindCenterVertical(const Mat& image, int x, int y, int fpbw[], vector<Drawable*> & debugVector, int * verticalPatternSize = NULL);
	int FindCenterHorizontal(const Mat& edgeArray, int x, int y, int fpbw[], int & xSize, vector<Drawable*> & debugVector);
	void FindEdgesVerticalClosed(const Mat & inputImg, int xPosition);
	bool validatePattern(FinderPattern * newPattern, vector<FinderPattern*> patternVector);
	//int SkipHeuristic(vector<FinderPattern*> & patternVector);
		
	//Alignment pattern
	void FindAlignmentPattern(Mat & inputImg, QRCode * newCode, vector<Drawable*>& debugVector);
	bool CheckAlignmentRatios(int * bw, int finderPatternSize);
	bool CheckAlignmentRatios(int * bw, int * previousBw, bool log = false);
	int FindAlignmentCenterVertical(const Mat & image, Rect searchRegion, int x, int y, int finderPatternSize, int * verticalWidths, int * horizontalBw, vector<Drawable*> & debugVector);
	int FindAlignmentCenterHorizontal(Rect searchRegion, int x, int y, int maxSize, int * verticalBw, int * horizontalBw, vector<Drawable*>& debugVector);
	bool CheckPoint(Mat & M, Point2i searchCenter, Size2i searchRange, int moduleSize);
	bool CheckAlignmentDiagonals(const Mat& image, Point2i center, int * verticalBw, int * horizontalBw, vector<Drawable*> & debugVector);
	
	//Fields
	ARControllerDebugUI * config;
	QRDecoder * qrDecoder;	

	//Stateful image values
	Size2i imgSize;
	Mat edgeArray;
	Mat verticalEdgeArray;
	map<int,bool> calculatedEdges;

	//Statistics
	double finderPatternTime, edgeTime;
	int numVerticalCalc;

	//Parameters
	int debugLevel, edgeThreshold, detectorSize, minimumFinderPatternScore, minimumAlignmentPatternScore, alignDebugLevel;
	bool nonMaxEnabled;
};

#endif 
