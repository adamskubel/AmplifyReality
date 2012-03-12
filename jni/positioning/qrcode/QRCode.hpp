#include "LogDefinitions.h"
#include "FinderPattern.hpp"
#include <opencv2/core/core.hpp>
#include "model/Drawable.hpp"
#include "model/DebugShape.hpp"
#include "zxing/common/PerspectiveTransform.h"

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
	std::string TextValue;

	QRCode(vector<FinderPattern*> _finderPatterns);
	~QRCode();

	void getTrackingPoints(vector<cv::Point2f> & points, vector<Point3f> & qrVector, bool extraPoints = false);
	zxing::Ref<zxing::PerspectiveTransform> getPerspectiveTransform();//vector<Point2i> & imagePoints, vector<Point2i> & codePoints);
	Rect getBoundaryRectangle();

	void Draw(Mat * rgbaImage);
	void sortCorners();
	bool isValidCode();
	bool isDecoded();

	int getCodeDimension();
	float getModuleSize();
	void setModuleSize(float moduleSize);

	float getAvgPatternSize();

	bool isCodeValidSuccessor(QRCode * nextCode);
	void SetAlignmentCorners(vector<Point2i> & alignmentCorners);	
	bool GuessAlignmentPosition(Point2i & result,Rect & searchRegion);

	static int FPDistance(FinderPattern * a,  FinderPattern * b);
	static int FPAcuteAngleGeometry(FinderPattern* a, FinderPattern* b, FinderPattern* c);
	static QRCode * CreateFromFinderPatterns(vector<FinderPattern*> & finderPatterns);

	void SetDrawingLevel(int level);
	float QRCodeDimension;
	
	Point2i codeCenter;
private:
	vector<Point2i> trackingCorners;
	int debugDrawingLevel;

};


#endif 