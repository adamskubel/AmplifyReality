#include <LogDefinitions.h>
#include <ExceptionCodes.hpp>
#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <vector>
#include "opencv2/highgui/highgui.hpp"

using namespace cv;



#ifndef DATACOLLECTOR_HPP_
#define DATACOLLECTOR_HPP_

class ImageCollector
{
public:
	enum ImageType {GRAY, RGBA, OTSU};
	void getImage(cv::Mat ** imageMatrix, ImageType type);
	ImageCollector(int width, int height);
	void newFrame();
	void teardown();


private:
	Mat *mrgba, *mgray, *mbin;
	bool rgbaUpdated, grayUpdated, otsuUpdated, grayOptMode;
	int width, height;
	VideoCapture * myCapture;


	void generateImage(ImageType type);


};

#endif
