#include "positioning/PositionSelector.hpp"


PositionSelector::PositionSelector()
{
	pastResults = new CircularList<PositioningResults*>(resultsToKeep);
}

PositionSelector::~PositionSelector()
{
	pastResults->clear();
	delete pastResults;
	LOGD(LOGTAG_POSITION,"Position Selector Deleted");
}

/* Set the rotation and translation matrices in the FrameItem to the
 * estimated position and orientation. 
 */
float PositionSelector::UpdatePosition(FrameItem * item)
{
	
	QRCode * qrCode = item->qrCode;
	
	PositioningResults * currentResults;

	if (qrCode == NULL)
	{
		
	}
	else
	{
		if (qrCode->validCodeFound)
		{
			LOGV(LOGTAG_POSITION,"Using position from code found this frame");

			//Just save the results
			currentResults = new PositioningResults();

			currentResults->Position = *(item->translationMatrix);
			currentResults->Rotation = *(item->rotationMatrix);
			currentResults->positioningMethod = PositioningMethods::QRCode;

			pastResults->add(currentResults);
			
			return 1.0f;
		}
		else //Partial or no code. Ignore partial case for now.
		{
			if (pastResults->size() > 0)
			{
				currentResults = new PositioningResults();
				PositioningResults * lastResults = pastResults->front();
				*(item->translationMatrix) = lastResults->Position;
				*(item->rotationMatrix) = lastResults->Rotation;

				currentResults->Position = lastResults->Position;
				currentResults->Rotation = lastResults->Rotation;
				currentResults->positioningMethod = PositioningMethods::Momentum;
				LowpassFilter(currentResults, pastResults->front());
				pastResults->add(currentResults);
		
				
				return 0.5f;
			}
			else //Just started and there are no past results. Position is completely unknown.
			{
				return 0.0f;
			}
		}
	}

}

void PositionSelector::LowpassFilter(PositioningResults * current, PositioningResults * previous)
{
	current->Position = current->Position * 0.9f + previous->Position * 0.1f;
}

void PositionSelector::FirstOrderPrediction(FrameItem * item)
{
	//vector<FrameItem *> validFrames = vector<FrameItem*>();
	////Collect all the frames that have positions found using QRCodes	
	//for (int i=0;i<previousItems.size();i++)
	//{
	//	if (pastResults.at(i)->positioningMethod == PositioningMethods::QRCode)
	//	{
	//		validFrames.push_back(previousItems.at(i));
	//	}
	//}
	////If enough frames were found, calculate the velocities between them
	//if (validFrames.size() > 2)
	//{
	//	vector<Mat> velocities;
	//	for (int i=0;i<validFrames.size() - 1;i++)
	//	{
	//		//velocities.push_back(CalculateVelocity(validFrames.at(i),validFrames.at(i+1)));
	//	}
	//	//Average velocities
	//	for (int i=0;i<velocities.size();i++)
	//	{

	//	}
	//
	//}
}

//Calculate the difference over time between two points
static Mat * CalculateVelocity(Mat * p1, Mat * p2, long timedelta)
{

}