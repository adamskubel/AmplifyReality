
#ifndef ALIGN_PATTERN_HELPER_HPP_
#define ALIGN_PATTERN_HELPER_HPP_

#include "LogDefinitions.h"
#include <opencv2/core/core.hpp>
#include "FindPattern.h"
#include "QRCode.hpp"
#include "model/DebugShape.hpp"

using namespace cv;

class AlignmentPatternHelper
{
public:	
	static void FindAlignmentPattern(Mat & M, QRCode * newCode, vector<Drawable*>& debugVector);

	static int MinimumAlignmentPatternScore;

private:
	static bool CheckAlignmentRatios(int * bw, int finderPatternSize);
	static bool CheckAlignmentRatios(int * bw, int * previousBw, bool log = false);
	static int FindAlignmentCenterVertical(const Mat& image, int x, int y, int maxSize, int * bw, int * horizontalBw);
	static int FindAlignmentCenterHorizontal(const Mat& image, int x, int y, int maxSize, int * verticalBw, int * horizontalBw);
	static bool CheckPoint(Mat & M, Point2i searchCenter, Size2i searchRange, int moduleSize);
	static bool CheckAlignmentDiagonals(const Mat& image, Point2i center, int * verticalBw, int * horizontalBw, vector<Drawable*> & debugVector);
};

#endif