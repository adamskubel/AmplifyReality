#include "QRFinder.hpp"


QRFinder::QRFinder(ARControllerDebugUI * _debugUI, FastQRFinder * _qrFinder)
{
	LOGD(LOGTAG_QR,"QRFinder initialized");
	qrDecoder = new QRDecoder();
	config = _debugUI;
	qrFinder = _qrFinder;

	finderPatternTime = 0;
	edgeTime = 0;
	debugLevel = 1;
	nonMaxEnabled = true;
	edgeThreshold = 10;
	detectorSize = 2;
	minimumFinderPatternScore = 180;
	minimumAlignmentPatternScore = 180;
	alignDebugLevel = 0;
		
	config->AddNewParameter("APFastScale",1.4f,0.1f,1.0f,100,"%2.1f","QR");
	config->AddNewParameter("EdgeThreshold",edgeThreshold,1,0,255,"%3.0f","QR");
	config->AddNewParameter("MinimumFPScore",minimumFinderPatternScore,10,0,400,"%3.0f","QR");
	config->AddNewParameter("MinimumAPScore",minimumAlignmentPatternScore,10,0,400,"%3.0f","QR");
	config->AddNewParameter("DetectorSize",detectorSize,1,1,5,"%1.0f","QR");
	config->AddNewParameter("QR Debug Level",debugLevel,1,-3,5,"%3.0f","Debug");
	config->AddNewParameter("AlignDebug",debugLevel,1,-3,5,"%3.0f","Debug");
	config->AddNewParameter("YResolution",2,1,1,50,"%2.0f","QR");
	config->AddNewParameter("EdgeNonMax",(float)nonMaxEnabled,1,0,1,"%1.0f","QR");
	
	config->AddNewLabel("FinderPatternTime"," ms");
	config->AddNewLabel("EdgeTime"," ms");
	config->AddNewLabel("NumVerticalCalc","");

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
		SET_TIME(&start);		
		FindAlignmentPattern(M, newCode,debugVector);				
		SET_TIME(&end);
		LOG_TIME("Alignment Search", start, end);

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





