#include "positioning/PositionSelector.hpp"


PositionSelector::PositionSelector(ARControllerDebugUI * _config)
{
	config = _config;
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
		if (qrCode->isValidCode())
		{
			LOGV(LOGTAG_POSITION,"Using position from code found this frame");

			//Just save the results
			currentResults = new PositioningResults();

			currentResults->Position = *(item->translationMatrix);
			currentResults->Rotation = *(item->rotationMatrix);
			currentResults->positioningMethod = PositioningMethods::QRCode;
			currentResults->PositionCertainty = 1.0f;

			if (!pastResults->empty())
			{
				LowpassFilter(currentResults, pastResults->front());
			}
			pastResults->add(currentResults);
			
			return currentResults->PositionCertainty;
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
				currentResults->PositionCertainty = lastResults->PositionCertainty * 0.9f;
				LowpassFilter(currentResults, pastResults->front());
				pastResults->add(currentResults);
		
				
				return currentResults->PositionCertainty;
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
	float alpha = config->GetParameter("T-Alpha");
	current->Position = current->Position * alpha + previous->Position * (1.0f-alpha);
		
	alpha = config->GetParameter("R-Alpha");
	current->Rotation = current->Rotation * alpha + previous->Rotation * (1-alpha);
}

void PositionSelector::FirstOrderPrediction(FrameItem * item)
{
	
}

//Calculate the difference over time between two points
static Mat * CalculateVelocity(Mat * p1, Mat * p2, long timedelta)
{

}