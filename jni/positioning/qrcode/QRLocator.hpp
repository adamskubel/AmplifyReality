#include "LogDefinitions.h"
#include "ExceptionCodes.hpp"
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2\calib3d\calib3d.hpp>
#include "QRCode.hpp"

#ifndef QR_LOCATPR_HPP_
#define QR_LOCATPR_HPP_

#undef LOG_TAG
#define LOG_TAG "QRLocator"

using namespace std;
using namespace cv;

class QRLocator
{
private:
	Mat * cameraMatrix, * distortionMatrix;
	//float qrCodeUnitDimension;

public:
	QRLocator(Mat  cameraMatrix, Mat distortionMatrix);
	QRLocator(Mat  cameraMatrix);
	~QRLocator();
	void transformPoints(QRCode * qrCode, Mat& rotationMatrix, Mat& translationMatrix);
};

#endif
