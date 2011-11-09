#include "QRLocator.hpp"

QRLocator::QRLocator(Mat& cameraMatrix,Mat& distortionMatrix)
{
	this->cameraMatrix = cameraMatrix;
	this->distortionMatrix = distortionMatrix;
	//this->cameraMatrix = new Mat();
	//invert(cameraMatrix, *(this->cameraMatrix), DECOMP_SVD);
	pixelWidth = 4.98f / 800;
	pixelHeight = 2.30f / 480;
}

void QRLocator::transformPoints(vector<Point_<int>* > pointVector, float qrSize, Mat& rotationMatrix, Mat& translationMatrix)
{
	unproject((float)pointVector.at(0)->x,(float)pointVector.at(0)->y);
}

Point2f QRLocator::unproject(float x, float y)
{
	LOGI("Undistorting points (%f,%f)",x,y);
	Mat pointMat = (Mat_<float>(1,3) << x * pixelWidth, y * pixelHeight, 1.0f);
	Mat output = Mat();
	undistortPoints(pointMat,output,*cameraMatrix,*distortionMatrix);
	LOGI("Undistort result (%f,%f)",output.at<float>(0,0),output.at<float>(1,0));
	Point2f result(output.at<float>(0,0),output.at<float>(1,0));
	return result;
}

