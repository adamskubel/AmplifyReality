#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"

#ifndef QR_LOCATPR_HPP_
#define QR_LOCATPR_HPP_

using namespace std;
using namespace cv;

class QRLocator
{

	Mat * cameraMatrix, * distortionMatrix;
	float pixelWidth, pixelHeight;

public:
	QRLocator(Mat cameraMatrix, Mat distortionMatrix);
	void transformPoints(vector<Point_<int>* > &pointVector, float qrSize, Mat& rotationMatrix, Mat& translationMatrix);

private:
	Point2f unproject(double x, double y);




};

#endif
