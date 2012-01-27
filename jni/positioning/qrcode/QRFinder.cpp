#include "QRFinder.hpp"
#include "LogDefinitions.h"
#include "QRCode.hpp"
//#include <PerspectiveTransform.h




QRCode * QRFinder::LocateQRCodes(cv::Mat& M, vector<Drawable*>& debugVector) 
{	
	struct timespec start,end;
	SET_TIME(&start);


	FinderPattern_vector pattern_store;

	//FindFinderPatterns_Symmetry(M,pattern_store,debugVector);
	FinderPatternHelper::FindFinderPatterns(M, pattern_store, debugVector);
	
	//If 3 patterns are found, we've probably found a QR code. Generate it and return.
	if (pattern_store.size() == 3)
	{
		FinderPattern  top_left, top_right, bottom_left;
		TriangleOrder(pattern_store, bottom_left, top_left, top_right);
		
		vector<FinderPattern*> * patternVector = new vector<FinderPattern*>();
		patternVector->push_back(new FinderPattern(top_left));
		patternVector->push_back(new FinderPattern(top_right));
		patternVector->push_back(new FinderPattern(bottom_left));
		
		QRCode * newCode = new QRCode(patternVector,true);
		SET_TIME(&end);
		LOG_TIME("QR Search(Found)", start, end);

		//Determine alignment pattern position
		SET_TIME(&start);

		int fpSize = (int)round(top_left.size/2.0f);

		Point2i alignmentGuess;
		alignmentGuess.x = (top_right.pt.x - top_left.pt.x) + bottom_left.pt.x - fpSize;
		alignmentGuess.y = (top_right.pt.y - top_left.pt.y) + bottom_left.pt.y - fpSize;								
						
		newCode->alignmentPattern = alignmentGuess;

		AlignmentPatternHelper::FindAlignmentPattern(M, newCode,debugVector);
				
		debugVector.clear();
		SET_TIME(&end);
		LOG_TIME("Alignment Search", start, end);

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
	for (int i=0;i<pattern_store.size();i++)
	{		
		patternVector->push_back(new FinderPattern(pattern_store.at(i)));
	}
	
	SET_TIME(&end);
	LOG_TIME("QR Search(NotFound)", start, end);
	return new QRCode(patternVector,false);
}


//Use cross product to arrange finder patterns into correct order
void QRFinder::TriangleOrder(const FinderPattern_vector& fpv, FinderPattern& bottom_left, FinderPattern& top_left, FinderPattern& top_right)
{
	long d0to1 = FinderPattern::Distance(fpv[0], fpv[1]);
	long d1to2 = FinderPattern::Distance(fpv[1], fpv[2]);
	long d0to2 = FinderPattern::Distance(fpv[0], fpv[2]);
	
	if (d1to2 > d0to1 && d1to2 > d0to2)
	{
		top_left = fpv[0];
		bottom_left = fpv[1];
		top_right = fpv[2];
	} else if (d0to1 > d1to2 && d0to1 > d0to2)
	{
		top_left = fpv[2];
		bottom_left = fpv[1];
		top_right = fpv[0];
	}  else if (d0to2 > d1to2 && d0to2 > d0to1)
	{
		top_left = fpv[1];
		bottom_left = fpv[0];
		top_right = fpv[2];
	}
	else
	{
		LOGW("QRFinder","Unable to resolve finder pattern order by distance");
		//no clear winner, probably some significant perspective. 
		if (FinderPattern::AcuteAngleGeometry(fpv[0], fpv[1], fpv[2]) >= 0)
		{
			top_left = fpv[0];
			top_right = fpv[1];
			bottom_left = fpv[2];
		}
		else if (FinderPattern::AcuteAngleGeometry(fpv[1], fpv[0], fpv[2]) >= 0)
		{
			top_left = fpv[1];
			top_right = fpv[0];
			bottom_left = fpv[2];
		}
		else if (FinderPattern::AcuteAngleGeometry(fpv[2], fpv[0], fpv[1]) >= 0)
		{
			top_left = fpv[2];
			top_right = fpv[0];
			bottom_left = fpv[1];
		}
		else
		{
			LOGE("Error determining finder pattern order. This shouldn't happen.");
			top_left = fpv[0];
			bottom_left = fpv[1];
			top_right = fpv[2];
		}
	}

	if (FinderPattern::AcuteAngleGeometry(bottom_left, top_left, top_right) < 0)
	{
		FinderPattern t = bottom_left;
		bottom_left = top_right;
		top_right = t;
	}

	return;
}


