#include "QRLocator.hpp"



QRLocator::QRLocator(Mat _cameraMatrix)
{
	cameraMatrix = new Mat(_cameraMatrix);
//	distortionMatrix = new Mat(_distortionMatrix);
}

void QRLocator::transformPoints(Point_<int> * pointArray, int numPoints, float qrSize, Mat& rotationMatrix, Mat& translationMatrix)
{
	double * pointData = new double[numPoints*2];

	vector<Point3f> qrVector = vector<Point3f>();
	qrVector.push_back(Point3f(0,0,0));
	qrVector.push_back(Point3f(qrSize,0,0));
	qrVector.push_back(Point3f(0,qrSize,0));
	qrVector.push_back(Point3f(qrSize,qrSize,0));


	
	for (int i=0;i<numPoints;i++)
	{
		LOGI("QRLocator","PointInput(%d)=[%lf,%lf]",i,(double) pointArray[i].x,(double) pointArray[i].x);
		pointData[i] = (double) pointArray[i].x;
		pointData[(i*2)+1] = (double) pointArray[i].y;
	}

	Mat pointMat = Mat(numPoints,1, CV_64FC2, pointData);
	Mat output;
	LOGD("QRLocator","Calling undistort");
	try
	{
		LOGD_Mat("QRLocator","Input Point",&pointMat);
		LOGD_Mat("QRLocator","Camera Matrix",cameraMatrix);
		solvePnP(qrVector,pointMat,*cameraMatrix,*distortionMatrix,rotationMatrix,translationMatrix,false);
		LOGD_Mat("QRLocator","Rotation",&rotationMatrix);
		LOGD_Mat("QRLocator","Translation",&translationMatrix);

	} catch (int e)
	{
		LOGE("QRLocator","Exception: %d", e);
	} catch (exception& e)
	{
		LOGE("QRLocator","Exception2: %s", e.what());
	}

	LOGD_Mat("QRLocator","UndistortResult",&output);
}

