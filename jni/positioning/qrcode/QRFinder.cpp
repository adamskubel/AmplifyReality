#include "QRFinder.hpp"


QRFinder::QRFinder(ARControllerDebugUI * _debugUI)
{
	LOGD(LOGTAG_QR,"QRFinder initialized");
	config = _debugUI;
	fastQRFinder = new FastQRFinder(config);

	finderPatternTime = 0;
	edgeTime = 0;
	fastFPTime = 0;
	//fastAPTime = 0;
	debugLevel = 1;
	nonMaxEnabled = true;
	edgeThreshold = 10;
	detectorSize = 2;
	minimumFinderPatternScore = 180;
	minimumAlignmentPatternScore = 180;
	alignDebugLevel = 0;
	
	config->AddNewParameter("MinFPDeviance",0.1f,0.1f,0.0f,3,"%2.1f","QR");
	config->AddNewParameter("MaxDevianceRatio",0.4f,0.1f,1.0f,3,"%2.1f","QR");
			
	config->AddNewParameter("APFastScale",1.4f,0.1f,1.0f,100,"%2.1f","QR");
	config->AddNewParameter("EdgeThreshold",edgeThreshold,1,0,255,"%3.0f","QR");
	config->AddNewParameter("MinimumFPScore",minimumFinderPatternScore,10,0,400,"%3.0f","QR");
	config->AddNewParameter("MinimumAPScore",minimumAlignmentPatternScore,10,0,400,"%3.0f","QR");
	config->AddNewParameter("DetectorSize",detectorSize,1,1,5,"%1.0f","QR",true);
	config->AddNewParameter("QR Debug Level",debugLevel,1,-3,5,"%3.0f","Debug");
	config->AddNewParameter("AlignDebug",debugLevel,1,-3,5,"%3.0f","Debug");
	config->AddNewParameter("YResolution",2,1,1,50,"%2.0f","QR");
	config->AddNewParameter("EdgeNonMax",(float)nonMaxEnabled,1,0,1,"%1.0f","QR",true);
	
	config->AddNewLabel("FinderPatternTime"," ms");
	config->AddNewLabel("EdgeTime"," ms");
	config->AddNewLabel("NumVerticalCalc","");
	config->AddNewLabel("FPCornerTime"," ms ");

	//Initialize matrix size variables
	imgSize = Size2i(0,0);
}

QRFinder::~QRFinder()
{
	LOGD(LOGTAG_QR,"QRFinder deleted successfully.");
}

class PatternDistCompare
{
public:
	PatternDistCompare(Point2i _center)
	{
		center = _center;
	}
	bool operator()(const FinderPattern * fp0, const FinderPattern * fp1)
	{
		return GetSquaredDistance(fp0->pt,center) > GetSquaredDistance(fp1->pt,center);
	}
private:
	Point2i center;
};


static void chooseBestPatterns(vector<FinderPattern*> & patternVector)
{
	if (patternVector.size() <= 3)
		return;

	Point2i centroid(0,0);
	for (int i =0;i<patternVector.size();i++)
	{
		centroid += patternVector[i]->pt;
	}
	centroid = Point2i(idiv(centroid.x,patternVector.size()),idiv(centroid.y,patternVector.size()));

	std::sort(patternVector.begin(),patternVector.end(),PatternDistCompare(centroid));
	
	while (patternVector.size() > 3)
	{
		delete patternVector.back();
		patternVector.pop_back();
	}
}


QRCode * QRFinder::LocateQRCodes(cv::Mat& M, vector<Drawable*>& debugVector) 
{
	struct timespec start,end;
	SET_TIME(&start);

	vector<FinderPattern*> finderPatterns;

	FindFinderPatterns(M, finderPatterns, debugVector);
	
	chooseBestPatterns(finderPatterns);

	//If 3 patterns are found, we've probably found a QR code. Generate it and return.
	if (finderPatterns.size() == 3)
	{		
		LOGD(LOGTAG_QR,"Creating new code.");
		QRCode * newCode = QRCode::CreateFromFinderPatterns(finderPatterns);
		SET_TIME(&end);
		LOG_TIME("QR Search(Found)", start, end);		
		
		//Determine alignment pattern position
		struct timespec aStart,aEnd;
		SET_TIME(&aStart);		
		FindAlignmentPattern(M, newCode,debugVector);				
		SET_TIME(&aEnd);
		LOG_TIME("Alignment Search", aStart, aEnd);

		return newCode;
	} 
		
	SET_TIME(&end);
	LOG_TIME("QR Search(NotFound)", start, end);

	QRCode * newCode = new QRCode(finderPatterns);	
	return newCode;
}





