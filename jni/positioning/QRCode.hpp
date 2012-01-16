#include "LogDefinitions.h"
#include "positioning/FindPattern.h"
#include <opencv2/core/core.hpp>

#ifndef QRCODE_HPP_
#define QRCODE_HPP_

//Represents a fully or partially located QR code
class QRCode
{
public:
	//If a valid code is found, patterns are stored in clockwise order starting with the top-left
	//Otherwise, effectively random
	vector<FinderPattern*> * finderPatterns;
	bool validCodeFound; 

	QRCode(vector<FinderPattern*> *_finderPatterns, bool codeFound) { finderPatterns = _finderPatterns, validCodeFound = codeFound;}

	std::vector<cv::Point2i> getPatternsAsPoints()
	{
		std::vector<cv::Point2i> pVec; 
		for (int i=0;i<finderPatterns->size();i++)
		{
			pVec.push_back(finderPatterns->at(i)->pt);
		}
		return pVec;
	}

	~QRCode() 
	{		
		if (finderPatterns != NULL)
		{
			while (!finderPatterns->empty())
			{
				delete finderPatterns->back();
				finderPatterns->pop_back();
			}
			delete finderPatterns;	
			finderPatterns = NULL;
		}			
	}

};

#endif 