#include "QRFinder.hpp"
#include "LogDefinitions.h"
#include "QRCode.hpp"



QRFinder::QRFinder(ARControllerDebugUI * _debugUI)
{
	LOGD(LOGTAG_QR,"QRFinder initialized");
	qrDecoder = new QRDecoder();
	config = _debugUI;

	finderPatternTime = 0;
	edgeTime = 0;
	debugLevel = 1;
	nonMaxEnabled = true;
	edgeThreshold = 10;
	detectorSize = 2;
	minimumFinderPatternScore = 180;

	config->AddNewParameter("EdgeThreshold",edgeThreshold,1,0,255,"%3.0f",2);
	config->AddNewParameter("MinimumFPScore",minimumFinderPatternScore,10,0,400,"%3.0f",2);
	config->AddNewParameter("DetectorSize",detectorSize,1,1,5,"%1.0f",2);
	config->AddNewParameter("DebugLevel",debugLevel,1,-3,5,"%3.0f",2);
	config->AddNewParameter("YResolution",2,1,1,50,"%2.0f",2);
	config->AddNewParameter("EdgeNonMax",(float)nonMaxEnabled,1,0,1,"%1.0f",2);
	
	config->AddNewLabel("FinderPatternTime"," ms",1);
	config->AddNewLabel("EdgeTime"," ms",1);
	//config->AddNewParameter("LogDebugLvl",0,1,0,3,"%3.0f",2);

	//Initialize matrix size variables
	imgSize = Size2i(0,0);
}

QRFinder::~QRFinder()
{
	delete qrDecoder;
	LOGD(LOGTAG_QR,"QRFinder deleted successfully.");
}

QRCode * QRFinder::LocateQRCodes(cv::Mat& M, vector<Drawable*>& debugVector, bool decode) 
{
	struct timespec start,end;
	SET_TIME(&start);

	vector<FinderPattern*> finderPatterns;

	FindFinderPatterns(M, finderPatterns, debugVector);

	//If 3 patterns are found, we've probably found a QR code. Generate it and return.
	if (finderPatterns.size() == 3)
	{		
		LOGD(LOGTAG_QR,"Creating new code.");
		QRCode * newCode = QRCode::CreateFromFinderPatterns(finderPatterns);
		SET_TIME(&end);
		LOG_TIME("QR Search(Found)", start, end);
		
		
		//Determine alignment pattern position
		/*SET_TIME(&start);		
		AlignmentPatternHelper::FindAlignmentPattern(M, newCode,debugVector);				
		SET_TIME(&end);
		LOG_TIME("Alignment Search", start, end);*/

		//Decode the QRCode
		if (false && decode)
		{
			SET_TIME(&start);
			qrDecoder->DecodeQRCode(M,newCode,debugVector);
			SET_TIME(&end);
			LOG_TIME("QR decode", start, end);
		}
		newCode->validCodeFound = false;
		return newCode;
	} 
		
	SET_TIME(&end);
	LOG_TIME("QR Search(NotFound)", start, end);

	QRCode * newCode = new QRCode(finderPatterns,false);	
	return newCode;
}





