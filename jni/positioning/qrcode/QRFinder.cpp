#include "QRFinder.hpp"
#include "LogDefinitions.h"
#include "QRCode.hpp"



QRFinder::QRFinder()
{
	LOGD(LOGTAG_QR,"QRFinder initialized");
	qrDecoder = new QRDecoder();
	fastThreshold = 10;
}

QRFinder::~QRFinder()
{
	delete qrDecoder;
	LOGD(LOGTAG_QR,"QRFinder deleted successfully.");
}

QRCode * QRFinder::LocateQRCodes(cv::Mat& M, vector<Drawable*>& debugVector, bool decode) 
{
	LOGD(LOGTAG_QR,"Enter QRFinder");
	struct timespec start,end;
	SET_TIME(&start);


	FinderPattern_vector * patternStore = new FinderPattern_vector();

	FinderPatternHelper::FindFinderPatterns(M, patternStore, debugVector);

	LOGD(LOGTAG_QR,"Found %d codes",patternStore->size());
	
	//If 3 patterns are found, we've probably found a QR code. Generate it and return.
	if (patternStore->size() == 3)
	{		
		LOGD(LOGTAG_QR,"Creating new code.");
		QRCode * newCode = new QRCode(QRCode::CreateFromFinderPatterns(patternStore));
		SET_TIME(&end);
		LOG_TIME("QR Search(Found)", start, end);
		
		//FastTracking::DoFastTracking(M,newCode,debugVector);
		
		//Determine alignment pattern position
		SET_TIME(&start);		
		AlignmentPatternHelper::FindAlignmentPattern(M, newCode,debugVector);				
		SET_TIME(&end);
		LOG_TIME("Alignment Search", start, end);

		//Decode the QRCode
		if (decode)
		{
			SET_TIME(&start);
			qrDecoder->DecodeQRCode(M,newCode,debugVector);
			SET_TIME(&end);
			LOG_TIME("QR decode", start, end);
		}
		return newCode;
	} 
	
	LOGD(LOGTAG_QR,"Not enough codes");
	//Less or more than three patterns were found, so just return a vector of all the finder patterns 
	
	vector<FinderPattern*> * patternVector = new vector<FinderPattern*>();
	for (int i=0;i<patternStore->size();i++)
	{		
		LOGD(LOGTAG_QR,"Pushing back FP(%d)",i);
		patternVector->push_back(patternStore->at(i));
	}		

	delete patternStore;

	SET_TIME(&end);
	LOG_TIME("QR Search(NotFound)", start, end);

	QRCode * newCode = new QRCode(patternVector,false);	
	return newCode;
}


void QRFinder::DoFastDetection(Mat & img, vector<Drawable*> & debugVector)
{
	vector<KeyPoint> kpVec;
	LOGD(LOGTAG_QR,"Calling FAST");
	struct timespec start,end;
	SET_TIME(&start);
	cv::FAST(img,kpVec,fastThreshold,true);
	SET_TIME(&end);
	LOG_TIME("FAST", start, end);
	for (int i=0;i<kpVec.size();i++)
	{
		debugVector.push_back(new DebugCircle(Point2i(kpVec.at(i).pt.x,kpVec.at(i).pt.y),5,Colors::Lime,false));
	}
}



