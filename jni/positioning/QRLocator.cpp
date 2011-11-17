#include "QRLocator.hpp"

QRLocator::QRLocator(Mat _cameraMatrix, Mat _distortionMatrix)
{
//	cameraMatrix = new Mat(_cameraMatrix.size,_cameraMatrix.type());
	cameraMatrix = new Mat();
	_cameraMatrix.copyTo(*cameraMatrix);
	//cameraMatrix = _cameraMatrix;
	LOGI_Mat("CamMat2",cameraMatrix);
	distortionMatrix = new Mat(_distortionMatrix);
	pixelWidth = 4.98f / 800;
	pixelHeight = 2.30f / 480;
}

void QRLocator::transformPoints(vector<Point_<int>*> &pointVector, float qrSize, Mat& rotationMatrix, Mat& translationMatrix)
{
	LOGI_Mat("CamMat2",cameraMatrix);
	unproject((double) pointVector.at(0)->x, (double) pointVector.at(0)->y);
}

Point2f QRLocator::unproject(double x, double y)
{
	double pointData[2] = { x, y };
	Mat pointMat = Mat(1,1, CV_64FC2, pointData);
//	LOGI("Undistort input: [%lf,%lf]", pointMat.at<double>(0,0), pointMat.at<double>(0,1));
	Mat output;
	LOGD("Calling undistort");
	try
	{
		LOGI_Mat("Input Point",&pointMat);
		LOGI_Mat("DistortionMat",distortionMatrix);
		LOGI_Mat("CameraMat",cameraMatrix);
	//	Mat identity = Mat(3,3,CV_64FC2,new double[]{1,1,1,1,1,1,1,1,1});
		undistortPoints(pointMat, output, *cameraMatrix, *distortionMatrix);
	} catch (int e)
	{
		LOGE("Exception: %d", e);
		return Point2f(0, 0);
	} catch (exception& e)
	{
		LOGE("Exception2: %s", e.what());
		return Point2f(0, 0);
	}

	LOGI_Mat("UndistortResult",&output);
	Point2f result(output.at<double>(0, 0), output.at<double>(1, 0));
	return result;
}

