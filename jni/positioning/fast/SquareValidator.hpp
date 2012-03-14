#ifndef SQUARE_VALIDATOR_HPP_
#define SQUARE_VALIDATOR_HPP_

#include <opencv2/core/core.hpp>

#include <vector>
#include "LogDefinitions.h"
#include "model/Drawable.hpp"
#include "model/DebugShape.hpp"
#include "FastTracking.hpp"
#include <stdlib.h>
#include <set>
#include "display/Colors.hpp"

class SquareValidator
{
public:
	SquareValidator();
	bool ValidateSquare(vector<Point2i> inputPoints, vector<Point2i> & squarePoints, int _startIndex, Point2i patternCenter, Size2i patternSize, vector<Drawable*> * _debugVector);
	void SetParameters(int debugLevel, float maxAngle = 0.15f);

private:
	bool addNextVertex(int currentIndex, int lastIndex, int lastSign, int depth, Scalar lineColor);

	//Params set once per validation
	vector<Drawable*> * debugVector;
	vector<Point2i> inputPoints;
	set<int> chosenIndices;
	int startIndex, numIterations, debugLevel;
	int minDistance, maxDistance;
	float maxAngle;
};

#endif