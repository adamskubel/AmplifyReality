#include "model/FrameItem.hpp"
#include "datastructures/CircularList.hpp"

#ifndef POSITION_SELECTOR_HPP_
#define POSITION_SELECTOR_HPP_

namespace PositioningMethods
{
	enum PositioningMethod
	{
		QRCode, MotionSensors, Momentum
	};

}

/*
- How was the position determined?
- What was the position?
- What is the estimated accuracy of the position?
*/
class PositioningResults
{
public:
	PositioningMethods::PositioningMethod positioningMethod;
	cv::Mat Rotation, Position;
	
	//cv::Mat RotationError, PositionError;
};


class PositionSelector
{
public:
	PositionSelector();
	~PositionSelector();
	float UpdatePosition(FrameItem * item);

private:
	void FirstOrderPrediction(FrameItem * item);
	CircularList<PositioningResults*> * pastResults;
	static const int resultsToKeep = 20;


};

#endif