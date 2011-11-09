#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"

#ifndef QR_LOCATPR_HPP_
#define QR_LOCATPR_HPP_

using namespace std;
using namespace cv;

class QRLocator
{

	Mat * cameraMatrix, * distortionMatrix;
	float pixelWidth, pixelHeight;

public:
	QRLocator(Mat& cameraMatrix, Mat& distortionMatrix);
	void transformPoints(vector<Point_<int>* > pointVector, float qrSize, Mat& rotationMatrix, Mat& translationMatrix);

private:
	Point2f unproject(float x, float y);




};

#endif
