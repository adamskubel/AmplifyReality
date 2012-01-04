#include <LogDefinitions.h>
#include <ExceptionCodes.hpp>
#include <DebugSettings.hpp>
#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <vector>
#include "opencv2/highgui/highgui.hpp"

using namespace cv;
using namespace std;



#ifndef DATACOLLECTOR_HPP_
#define DATACOLLECTOR_HPP_

class ImageCollector
{
public:
	enum ImageType {GRAY, RGBA, OTSU};
	void setCorrectionMatrices(Mat* cameraMatrix, Mat* distortionMatrix);
	void getImage(cv::Mat ** imageMatrix, ImageType type);
	ImageCollector(int width, int height);
	void newFrame();
	void teardown();


private:
	Mat *mrgba, *mgray, *mbin;
	Mat *cameraMatrix, *distortionMatrix;
	bool rgbaUpdated, grayUpdated, otsuUpdated, grayOptMode;
	int width, height;
	VideoCapture * myCapture;

	void undistortImage(Mat * inputImage, Mat* outputImage);
	void generateImage(ImageType type);
	bool canUndistort();
	void qrBinarization(Mat * inputImage, Mat* outputImage); 
	void localThresholding(Mat& inputImage, Mat& outputImage, int windowWidth, int windowHeight);


};

#endif
