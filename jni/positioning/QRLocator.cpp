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
	float qrSize = 37;

	struct timespec start,end;
	SET_TIME(&start);

	vector<Point3f> qrVector = vector<Point3f>();
	qrVector.push_back(Point3f(0,0,0));
	qrVector.push_back(Point3f(qrSize,0,0));
	qrVector.push_back(Point3f(qrSize,qrSize,0));
	qrVector.push_back(Point3f(0,qrSize,0));
		
	vector<Point2f> imagePointVector = vector<Point2f>();
	for (int i=0;i<qrCode->finderPatterns->size();i++)
	{
		imagePointVector.push_back(Point2f(qrCode->finderPatterns->at(i)->pt.x,qrCode->finderPatterns->at(i)->pt.y));
		//Point2f((float)pointArray[i].x,(float)pointArray[i].y));
	}
	//LOG_Vector(ANDROID_LOG_DEBUG,LOGTAG_QR,"Input Points",&imagePointVector);

	LOGV(LOGTAG_QR,"Calling solvePnP");
	try
	{

	/*	LOGD_Mat(LOGTAG_QR,"Camera Matrix",cameraMatrix);
		LOGD_Mat(LOGTAG_QR,"Distortion Matrix",distortionMatrix);*/
		solvePnP(qrVector,imagePointVector,*cameraMatrix,*distortionMatrix,rotationMatrix,translationMatrix,false);
	//	LOGD_Mat(LOGTAG_QR,"Rotation",&rotationMatrix);
	/*	LOGD_Mat(LOGTAG_QR,"Translation",&translationMatrix);*/

	} catch (exception& e)
	{
		LOGE("Exception from solvePnP: %s", e.what());
	}

	SET_TIME(&end);
	LOG_TIME("Solve PnP",start,end);
}





