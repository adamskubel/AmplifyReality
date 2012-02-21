#include "LogDefinitions.h"
#include "FindPattern.h"
#include <opencv2/core/core.hpp>
#include "model/Drawable.hpp"

#ifndef QRCODE_HPP_
#define QRCODE_HPP_

using namespace cv;

//Represents a fully or partially located QR code
class QRCode : public Drawable
{	
public:
	static int instanceCount;

	vector<FinderPattern*> finderPatterns;
	Point2i alignmentPattern;
	bool validCodeFound; 
	std::string TextValue;

	QRCode(vector<FinderPattern*> _finderPatterns, bool codeFound, Point2i _alignmentPattern = Point2i(0,0));
	~QRCode();

	vector<cv::Point2i> getPatternsAsPoints();	
	void Draw(Mat * rgbaImage);

	static int FPDistance(FinderPattern * a,  FinderPattern * b);
	static int FPAcuteAngleGeometry(FinderPattern* a, FinderPattern* b, FinderPattern* c);
	static QRCode * CreateFromFinderPatterns(vector<FinderPattern*> & finderPatterns);

};


#endif 