#include "QRFinder.hpp"


QRFinder::QRFinder(ARControllerDebugUI * _debugUI)
{
	LOGD(LOGTAG_QR,"QRFinder initialized");
	config = _debugUI;
	fastQRFinder = new FastQRFinder(config);

	finderPatternTime = 0;
	edgeTime = 0;
	fastFPTime = 0;
	debugLevel = 1;
	nonMaxEnabled = true;
	edgeThreshold = 20;
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
	config->AddNewParameter("Finder Pattern","QR Debug Level",debugLevel,1,-3,5,"%3.0f","Debug");
	config->AddNewParameter("Alignment Pattern","AlignDebug",debugLevel,1,-3,5,"%3.0f","Debug");
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
	if (patternVector.size() < 3)
		return;
	LOGV(LOGTAG_QR,"Sorting %d patterns",patternVector.size());

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

void QRFinder::prepareMatrices(Mat & inputImg)
{
	calculatedEdges.clear();	
	numVerticalCalc = 0;

	//If image size has changed, reallocate the matrix
	if (imgSize.width != inputImg.cols || imgSize.height != inputImg.rows)
	{
		imgSize = Size2i(inputImg.cols,inputImg.rows);
		LOGD(LOGTAG_QR,"Initializing new matrix, size=(%d,%d)",imgSize.width,imgSize.height);
		edgeArray = Mat::zeros(inputImg.rows,inputImg.cols,CV_16S); //Maximized edges
		verticalEdgeArray = Mat::zeros(inputImg.cols,inputImg.rows,CV_16S); //Maximized vertical edges
	}
}

QRCode * QRFinder::LocateQRCodes(cv::Mat& inputImg, vector<Drawable*>& debugVector, QRCode * lastCode) 
{
	

	prepareMatrices(inputImg);

	float regionAdjuster = 1.5f;
	vector<FinderPattern*> finderPatterns;
		
	struct timespec start,end;
	SET_TIME(&start);
	if (lastCode != NULL && lastCode->isValidCode())
	{		
		LOGV(LOGTAG_QR,"Tracking %d patterns",finderPatterns.size());
		for (int i=0;i<lastCode->finderPatterns.size();i++)
		{
			FinderPattern * fp = lastCode->finderPatterns.at(i);
			Size2i regionSize = fp->patternSize;
			regionSize = Size2i((float)regionSize.width * regionAdjuster, (float)regionSize.height * regionAdjuster);
			Rect fpRegion = Rect(fp->pt.x - regionSize.width,fp->pt.y - regionSize.height,2 * regionSize.width, 2 * regionSize.height);
			ConstrainRectangle(inputImg,fpRegion);
			
			FindFinderPatterns(inputImg,fpRegion,finderPatterns,debugVector);
		}
	}
	else
	{
		LOGV(LOGTAG_QR,"No tracking");
		FindFinderPatterns(inputImg,Rect(0,0,inputImg.cols,inputImg.rows), finderPatterns, debugVector);
	}
	SET_TIME(&end);
	LOG_TIME("FinderPatternSearch", start, end);	

	chooseBestPatterns(finderPatterns);	

	//If 3 patterns are found, we've probably found a QR code. Generate it and return.
	if (finderPatterns.size() == 3)
	{		
		LOGD(LOGTAG_QR,"Creating new code.");
		QRCode * newCode = QRCode::CreateFromFinderPatterns(finderPatterns);
		
		//Determine alignment pattern position
		struct timespec aStart,aEnd;
		SET_TIME(&aStart);		
		FindAlignmentPattern(inputImg, newCode,debugVector);		
	/*	if (GetSquaredDistance(newCode->alignmentPattern,lastCode->alignmentPattern) > lastCode->getAvgPatternSize() * 3.0f)
			newCode->alignmentPattern = lastCode->alignmentPattern;*/
		SET_TIME(&aEnd);
		LOG_TIME("Alignment Search", aStart, aEnd);

		return newCode;
	} 	

	QRCode * newCode = new QRCode(finderPatterns);	
	return newCode;
}





