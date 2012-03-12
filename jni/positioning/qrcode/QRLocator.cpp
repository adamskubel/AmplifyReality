#include "QRLocator.hpp"


QRLocator::QRLocator(Mat  _cameraMatrix, Mat  _distortionMatrix, ARControllerDebugUI * _config)
{
	cameraMatrix = new Mat();
	_cameraMatrix.copyTo(*cameraMatrix);

	distortionMatrix = new Mat();
	_distortionMatrix.copyTo(*distortionMatrix);

	LOGD_Mat(LOGTAG_QR,"Instantiated with camera matrix:",cameraMatrix);
	LOGD_Mat(LOGTAG_QR,"and with distortion matrix:",distortionMatrix);

	config = _config;
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
	
	bool extraPoints = false;
	if (config != NULL)
		 extraPoints = config->GetBooleanParameter("UseExtraPoints");

	vector<Point3f> qrVector;
	vector<Point2f> imagePointVector;

	LOGV(LOGTAG_QR,"Retreiving tracking points. Extra=%d",extraPoints);
	qrCode->getTrackingPoints(imagePointVector,qrVector,extraPoints);

	/*LOG_Vector(ANDROID_LOG_DEBUG,LOGTAG_POSITION,"ImagePoints",&imagePointVector);
	LOG_Vector(ANDROID_LOG_DEBUG,LOGTAG_POSITION,"QR-Points",&qrVector);*/
	Mat imagePointMatrix(imagePointVector.size(),2,CV_32F,imagePointVector.data());
	LOGD_Mat(LOGTAG_POSITION,"ImagePoints",&imagePointMatrix);
	Mat qrPointMatrix(qrVector.size(),3,CV_32F,qrVector.data());
	LOGD_Mat(LOGTAG_POSITION,"QRPoints",&qrPointMatrix);

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





