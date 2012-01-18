#include "positioning/PositionPredictor.hpp"

/*
void PositionPredictor::Update(FrameItem * item)
{	
	vector<Mat*> positionVectors = vector<Mat*>();
	Mat predictedPosition = Mat(), predictedRotation = Mat();
}

static void FirstOrderPrediction(FrameItem * item)
{
	vector<FrameItem *> previousItems = item->getLastFrames();

	vector<FrameItem *> validFrames = vector<FrameItem*>();
	//Collect all the frames that have positions found using QRCodes	
	for (int i=0;i<previousItems.size();i++)
	{
		if (previousItems.at(i)->positioningResults->positioningMethod == PositioningMethods::QRCode)
		{
			validFrames.push_back(previousItems.at(i));
		}
	}
	//If enough frames were found, calculate the velocities between them
	if (validFrames.size() > 2)
	{
		vector<Mat> velocities;
		for (int i=0;i<validFrames.size() - 1;i++)
		{
			velocities.push_back(CalculateVelocity(validFrames.at(i),validFrames.at(i+1)));
		}
		//Average velocities
		for (int i=0;i<velocities.size();i++)
		{

		}
	
	}
}

static Mat CalculateVelocity(FrameItem * item1, FrameItem * item2)
{

}

void PositionPredictor::CalculateMotion(FrameItem * item, vector<Mat*> * positionVectors, vector<Mat*> * rotationVectors, Mat * predictedPosition, Mat * predictedRotation)
{
	////Calculate difference between the two most recent positions
	//if (positionVectors->size() > 1)
	//{
	//	Mat * p1 = positionVectors->at(0);
	//	Mat * p2 = positionVectors->at(1);

	//	Mat diff = (*p1 - *p2);

	//	*predictedPosition = *(item->translationMatrix) + diff;
	//	LOGD_Mat(LOGTAG_POSITION, "Prediction position",predictedPosition);

	//	//double length = sqrt(diff.dot(diff));
	//}


}


void PositionPredictor::UnprojectCode(QRCode * qrCode, Mat * predictedPosition, Mat * predictedRotation, Rect * predictedCodeArea)
{
	//
}




/*
Returns a vector of 3x1 matrices indicating all the past positions
*
void PositionPredictor::ExtractPosition(vector<FrameItem *> * previousItems , vector<Mat*> * positionVectors)
{
	for (int i = 0;i < previousItems->size(); i++)
	{
		positionVectors->push_back(previousItems->at(i)->translationMatrix);
	}
}


*/