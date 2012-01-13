#include "model/Updateable.hpp"

#ifndef POSITION_PREDICTOR_HPP_
#define POSITION_PREDICTOR_HPP_


/*
Use data from past and current frames to predict the current location of the phone, 
and the location of the tracking code in the image
*/
using namespace cv;

class PositionPredictor
{
public:
	void Update(FrameItem * item);

private:
	void ExtractPosition(vector<FrameItem *> * previousItems , vector<Mat*> * positionVectors);
	void CalculateMotion(FrameItem * item, vector<Mat*> * positionVectors, vector<Mat*> * rotationVectors, Mat * predictedPosition, Mat * predictedRotation);
	void UnprojectCode(QRCode * qrCode, Mat * predictedPosition, Mat * predictedRotation, Rect * predictedCodeArea);
};

#endif