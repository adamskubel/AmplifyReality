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
	struct timespec start,end;
	SET_TIME(&start);

	vector<Point3f> qrVector = vector<Point3f>();
	vector<Point2f> imagePointVector;

	LOGV(LOGTAG_QR,"Retreiving tracking points");
	qrCode->getTrackingPoints(imagePointVector,qrVector);

	LOG_Vector(ANDROID_LOG_DEBUG,LOGTAG_POSITION,"ImagePoints",&imagePointVector);
	LOG_Vector(ANDROID_LOG_DEBUG,LOGTAG_POSITION,"QR-Points",&qrVector);

	try
	{
		LOGV(LOGTAG_QR,"Calling solvePnP");
		solvePnP(qrVector,imagePointVector,*cameraMatrix,*distortionMatrix,rotationMatrix,translationMatrix,false);
	} catch (exception& e)
	{
		LOGE("Exception from solvePnP: %s", e.what());
	}

	SET_TIME(&end);
	LOG_TIME("Solve PnP",start,end);
}





