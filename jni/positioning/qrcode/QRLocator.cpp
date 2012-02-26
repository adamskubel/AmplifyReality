#include "QRLocator.hpp"


QRLocator::QRLocator(Mat  _cameraMatrix, Mat  _distortionMatrix)
{
	cameraMatrix = new Mat();
	_cameraMatrix.copyTo(*cameraMatrix);

	distortionMatrix = new Mat();
	_distortionMatrix.copyTo(*distortionMatrix);

	LOGD_Mat(LOGTAG_QR,"Instantiated with camera matrix:",cameraMatrix);
	LOGD_Mat(LOGTAG_QR,"and with distortion matrix:",distortionMatrix);
}

QRLocator::QRLocator(Mat _cameraMatrix)
{
	cameraMatrix = new Mat();
	_cameraMatrix.copyTo(*cameraMatrix);

	distortionMatrix = new Mat(Mat::zeros(1,5,CV_64F));
	LOGD_Mat(LOGTAG_QR,"Instantiated with only camera matrix:",cameraMatrix);
}

QRLocator::~QRLocator()
{
	delete cameraMatrix;
	delete distortionMatrix;
	LOGD(LOGTAG_QR,"QRLocator Deleted Successfully");
}

//Transform a set of points from camera space to reality space 
void QRLocator::transformPoints(QRCode * qrCode, Mat& rotationMatrix, Mat& translationMatrix)
{
	float qrSize = 41.25;
	float apSize = 36;

	struct timespec start,end;
	SET_TIME(&start);

	vector<Point3f> qrVector = vector<Point3f>();
	qrVector.push_back(Point3f(0,0,0));
	qrVector.push_back(Point3f(qrSize,0,0));
	qrVector.push_back(Point3f(apSize,apSize,0)); //alignment pattern
	qrVector.push_back(Point3f(0,qrSize,0));
		
	vector<Point2f> imagePointVector;/*
	imagePointVector.push_back(Point2f(qrCode->finderPatterns.at(0)->pt.x,qrCode->finderPatterns.at(0)->pt.y));
	imagePointVector.push_back(Point2f(qrCode->finderPatterns.at(1)->pt.x,qrCode->finderPatterns.at(1)->pt.y));
	imagePointVector.push_back(Point2f(qrCode->alignmentPattern.x, qrCode->alignmentPattern.y));
	imagePointVector.push_back(Point2f(qrCode->finderPatterns.at(2)->pt.x,qrCode->finderPatterns.at(2)->pt.y));*/

	LOGV(LOGTAG_QR,"Retreiving tracking points");
	qrCode->getTrackingPoints(imagePointVector);

	/*LOG_Vector(ANDROID_LOG_DEBUG,LOGTAG_QR,"ImagePoints",&imagePointVector);
	LOG_Vector(ANDROID_LOG_DEBUG,LOGTAG_QR,"QR-Points",&qrVector);*/

	LOGV(LOGTAG_QR,"Calling solvePnP");
	try
	{
		solvePnP(qrVector,imagePointVector,*cameraMatrix,*distortionMatrix,rotationMatrix,translationMatrix,false);
	} catch (exception& e)
	{
		LOGE("Exception from solvePnP: %s", e.what());
	}

	SET_TIME(&end);
	LOG_TIME("Solve PnP",start,end);
}





