#ifndef FINDERPATTERN_HELPER_HPP_
#define FINDERPATTERN_HELPER_HPP_


#include "LogDefinitions.h"
#include <opencv2/core/core.hpp>
#include "FindPattern.h"
#include "QRCode.hpp"
#include "model/DebugShape.hpp"

class FinderPatternHelper
{
public:	
	static void FindFinderPatterns(cv::Mat& M, FinderPattern_vector * fpv, vector<Drawable*>& debugVector);	
	static void FindFinderPatterns_Symmetry(cv::Mat& M, FinderPattern_vector& fpv, vector<Point3i>& debugVector);
	
	static int MinimumFinderPatternScore;

private:
	static bool CheckRatios(int * bw, int  * oldBw, float scoreModifier = 1.0f);
	static bool CheckRatios2(int bw[], int bw2[]);
	static int FindCenterVertical(const Mat& image, int x, int y, int fpbw[]);
	static int FindCenterHorizontal(const Mat& image, int x, int y, int fpbw[], int yDelta = 0);

	static bool GetEdges(const Mat& image, Point2i start, int xDir, int yDir, int * Q);
	static float CentralSymmetry(int * x);
	static float SquareCharacteristic(int *x, int * y);
	static float RatioCharacteristic(int * x);
	static float SymmetryAlgorithm(int * X, int * Y, int * R, int * S);
	static bool CheckArea(const Mat& image, Point2i center, int bwLengths[], FinderPattern & result);
	
	static int SkipHeuristic(FinderPattern_vector * fpv);
};

#endif