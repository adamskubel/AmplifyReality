#include "QRLocator.hpp"

QRLocator::QRLocator(Mat cameraMatrix, Mat distortionMatrix)
{
	this->cameraMatrix = new Mat(cameraMatrix); //->clone();
	this->distortionMatrix = new Mat(distortionMatrix); //->clone();
	//this->cameraMatrix = new Mat();
	//invert(cameraMatrix, *(this->cameraMatrix), DECOMP_SVD);
	pixelWidth = 4.98f / 800;
	pixelHeight = 2.30f / 480;
}

void QRLocator::transformPoints(vector<Point_<int>*> &pointVector, float qrSize, Mat& rotationMatrix, Mat& translationMatrix)
{
	unproject((double) pointVector.at(0)->x, (double) pointVector.at(0)->y);
}

Point2f QRLocator::unproject(double x, double y)
{
	LOGI("Undistorting points (%f,%f)", x, y);
	double pointData[2] = { x, y };
	Mat pointMat = Mat(1, 2, CV_64F, pointData);
	Mat output = Mat(1, 3, CV_64F);
	LOGD("Calling undistort");
	try
	{
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

	LOGI("Undistort result: [%lf,%lf,1]t", output.at<double>(0,0), output.at<double>(1,0));
	Point2f result(output.at<double>(0, 0), output.at<double>(1, 0));
	return result;
}

