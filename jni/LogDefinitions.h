#include <android/log.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "DebugSettings.hpp"

#ifndef LOG_TAG
#define  LOG_TAG    ""
#endif

#define  LOG_TAG_BASE    "AmplifyR-"
#define  LOG_TAG_TIME    "AmplifyR-Time"
#define  LOG_TAG_TIME_BASE    "AmplifyR-Time-"

#define STR(tok) #tok


#define  LOGV(TAG,...)  __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG_BASE TAG,__VA_ARGS__)
#define  LOGD(TAG,...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG_BASE TAG,__VA_ARGS__)
#define  LOGI(TAG,...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG_BASE TAG,__VA_ARGS__)
#define  LOGW(TAG,...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG_BASE TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,"AmplifyR",__VA_ARGS__)

#define LOGTAG_QRCONTROLLER "QR"
#define LOGTAG_MAIN "Main"
#define QRFINDER_LOGTAG "QR"
#define LOGTAG_QR "QR"
#define LOGTAG_OPENGL "OpenGL"
#define LOGTAG_IMAGECAPTURE "ImageCapture"
#define LOGTAG_IMAGEPROCESSING "ImageProcessing"
#define LOGTAG_INPUT "Input"
#define LOGTAG_CALIBRATION "Calibration"
#define LOGTAG_BUTTON "Input"


/*
Suggested logcat filters:
AmplifyR-All:D AmplifyR-QRController:D AmplifyR-Main:D AmplifyR-QRFinder:D AmplifyR-ImageCapture:D AmplifyR-ImageProcessing:D *:S
*/


#define calc_time(start,end) (((end.tv_sec*1000000000LL + end.tv_nsec) - (start.tv_sec*1000000000LL + start.tv_nsec))/1000000LL)

#define SET_TIME(timeObject) clock_gettime(CLOCK_PROCESS_CPUTIME_ID, timeObject);

#define LOG_TIME(message,start,end) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG_TIME,"%s took %ld ms",message,calc_time(start,end))


#define START_TIMER struct timespec start,end; clock_gettime(CLOCK_PROCESS_CPUTIME_ID, start);
#define STOP_TIMER(TAG,MSG)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG_TIME_BASE TAG,"%s took %ld ms",message,calc_time(start,end))

#ifndef LOGDEF_HPP_
#define LOGDEF_HPP_


static void LOG_INTRO()
{
	LOGI(LOGTAG_MAIN,"******************************");
	LOGI(LOGTAG_MAIN,"     --Amplified Reality--    ");
	LOGI(LOGTAG_MAIN,"       Version %s             ","0.01");
	LOGI(LOGTAG_MAIN,"         Adam Skubel          ");
	LOGI(LOGTAG_MAIN,"******************************");
}


static void LOGD_Mat(std::string matrixTag, const char * matDescription,cv::Mat * matrix)
{
	if (matrix == NULL)
	{
		std::string prefix = std::string("AmplifyR-");
		__android_log_print(ANDROID_LOG_DEBUG,(prefix.append(matrixTag)).c_str(),"%s is NULL",matDescription);
	}


	for (int i = 0; i < matrix->rows; i++)
	{
		char string[300];
		sprintf(string,"[");
		for (int j = 0; j < matrix->cols*matrix->channels(); j++)
		{			
			sprintf(string,"%s,%lf",string,matrix->at<double>(i,j));
		}
		sprintf(string,"%s]",string);
		std::string prefix = std::string("AmplifyR-");
		__android_log_print(ANDROID_LOG_DEBUG,(prefix.append(matrixTag)).c_str(),"%s[:,%d]=%s",matDescription,i,string);
	}
}


static void LOG_Vector(android_LogPriority logPriority, std::string vectorTag, const char * vectorDescription,std::vector<cv::Point2f> * pointVector)
{
	if (pointVector == NULL)
	{
		std::string prefix = std::string("AmplifyR-");
		__android_log_print(logPriority,(prefix.append(vectorTag)).c_str(),"%s is NULL",vectorDescription);
	}

	char string[300];
	sprintf(string,"[");
	for (int i = 0; i < pointVector->size();i++)
	{		
		sprintf(string,"%s,(%f,%f)",string,pointVector->at(i).x,pointVector->at(i).y);
	}
	sprintf(string,"%s]",string);
	std::string prefix = std::string("AmplifyR-");
	__android_log_print(logPriority,(prefix.append(vectorTag)).c_str(),"%s=%s",vectorDescription,string);
}


//static void LOGI_Mat(cv::Mat& matrix)
//{
////	if (matrix == NULL)
////	{
////		LOGI("Matrix is null");
////		return;
////	}
//
//	std::string s = std::string();
//	s.reserve(1024);
//	s += "{";
//	for (int i = 0; i < matrix.rows; i++)
//	{
//		s += "[";
//		for (int j = 0; j < matrix.cols; j++)
//		{
//			//char * strPnt = s.begin();
//			char * strPnt = const_cast<std::string::pointer>(s.data());
//			sprintf(strPnt,"[%lf",matrix.at<double>(i,j));
//			//sprintf(strPnt,"%lf",matrix.at<double>(i,j));
//			if (j != matrix.cols - 1)
//				s+= ",";
//		}
//		s += "]";
////		if (i != matrix.rows - 1)
////			s += "\n";
//	}
//	s += "}";
//	LOGI("Matrix: %s",s.c_str());
//}

//static void LOGI_Mat(cv::Mat *A)
//{
//	int i, j;
//	LOGI("Matrix:");
//	for (i = 0; i < A->rows; i++)
//	{
//		LOGI("Row %d",i);
//		switch(CV_MAT_DEPTH(A->type()))
//		{
//			case CV_32F:
//			case CV_64F:
//			for (j = 0; j < A->cols; j++)
//			LOGI ("%8.3f ", (float)cvGetReal2D(A, i, j));
//			break;
//			case CV_8U:
//			case CV_16U:
//			for(j = 0; j < A->cols; j++)
//			LOGI ("%6d",(int)cvGetReal2D(A, i, j));
//			break;
//			default:
//			break;
//		}
//	}
//	LOGI("endMatrix");
//}

#endif

//struct timespec startTime;

//void LogTime_StartEnd(struct timespec start,struct timespec end, char * message)
//{
//	char myString[100];
//	sprintf(myString,"%s took %ld ms",message,calc_time(start,end));
//}
//
//void LogTime_Start(struct timespec start,char * message)
//{
//	struct timespec end;
//	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
//	char myString[100];
//	sprintf(myString,"%s took %ld ms",message,calc_time(start,end));
//}
//
//void LogTime(char * message)
//{
//	LogTime_Start(startTime,message);
//}
