#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2\calib3d\calib3d.hpp>

#ifndef QR_LOCATPR_HPP_
#define QR_LOCATPR_HPP_

#undef LOG_TAG
#define LOG_TAG "QRLocator"

using namespace std;
using namespace cv;

class QRLocator
{

	Mat * cameraMatrix, * distortionMatrix;

public:
	QRLocator(Mat  cameraMatrix);
	void transformPoints(Point_<int>* points, int numPoints, float qrSize, Mat& rotationMatrix, Mat& translationMatrix);


};

#endif
