#include "QRFinder.hpp"
#include "LogDefinitions.h"
#include "QRCode.hpp"
//#include <PerspectiveTransform.h




QRCode * QRFinder::LocateQRCodes(cv::Mat& M, vector<Drawable*>& debugVector) 
{	
	struct timespec start,end;
	SET_TIME(&start);


	FinderPattern_vector patternStore;

	//FindFinderPatterns_Symmetry(M,patternStore,debugVector);
	FinderPatternHelper::FindFinderPatterns(M, patternStore, debugVector);
	
	//If 3 patterns are found, we've probably found a QR code. Generate it and return.
	if (patternStore.size() == 3)
	{		
		QRCode * newCode = new QRCode(QRCode::CreateFromFinderPatterns(patternStore));
		SET_TIME(&end);
		LOG_TIME("QR Search(Found)", start, end);

		//Determine alignment pattern position
		SET_TIME(&start);		

		AlignmentPatternHelper::FindAlignmentPattern(M, newCode,debugVector);				
		
		//Temp disable of alignment debug
		debugVector.clear();


		SET_TIME(&end);
		LOG_TIME("Alignment Search", start, end);

		//LOGD(LOGTAG_QR,"QR Search results: %d FP's found, AP = (%d,%d)",newCode->finderPatterns->size(),newCode->alignmentPattern.x,newCode->alignmentPattern.y);

		//Decode the QRCode
		SET_TIME(&start);
		QRDecoder * qrDecoder = new QRDecoder();
		qrDecoder->DecodeQRCode(M,newCode,debugVector);
		delete qrDecoder;

		SET_TIME(&end);
		LOG_TIME("QR decode", start, end);

		return newCode;
	} 
	
	//Less or more than three patterns were found, so just return a vector of all the finder patterns 
	vector<FinderPattern*> * patternVector = new vector<FinderPattern*>();
	for (int i=0;i<patternStore.size();i++)
	{		
		patternVector->push_back(patternStore.at(i));
	}
	
	SET_TIME(&end);
	LOG_TIME("QR Search(NotFound)", start, end);
	return new QRCode(patternVector,false);
}




