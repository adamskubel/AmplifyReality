#include <LogDefinitions.h>
#include <ExceptionCodes.hpp>
#include <DebugSettings.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>

#ifndef DATACOLLECTOR_HPP_
#define DATACOLLECTOR_HPP_

using namespace cv;
using namespace std;


class ImageCollector
{
public:
	enum ImageType {GRAY, RGBA};
	void setCorrectionMatrices(Mat* cameraMatrix, Mat* distortionMatrix);
	void getImage(cv::Mat ** imageMatrix, ImageType type);
	ImageCollector(int width, int height);
	void newFrame();
	void teardown();


private:
	Mat *mrgba, *mgray, *mbin;
	Mat *cameraMatrix, *distortionMatrix;
	bool rgbaUpdated, grayUpdated, grayOptMode;
	int width, height;
	VideoCapture * myCapture;

	void undistortImage(Mat * inputImage, Mat* outputImage);
	void generateImage(ImageType type);
	bool canUndistort();
};

#endif
