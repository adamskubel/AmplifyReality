#include <android/log.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"

#define  LOG_TAG    "AmplifyR"
#define  LOG_TAG_BASE    "AmplifyR"
#define  LOG_TAG_TIME    "AmplifyR-Time"

#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGT(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG_TIME,__VA_ARGS__)

#define calc_time(start,end) (((end.tv_sec*1000000000LL + end.tv_nsec) - (start.tv_sec*1000000000LL + start.tv_nsec))/1000000LL)

#define SET_TIME(timeObject) clock_gettime(CLOCK_PROCESS_CPUTIME_ID, timeObject);

#define LOG_TIME(message,start,end) LOGT("%s took %ld ms",message,calc_time(start,end))

#ifndef LOGDEF_HPP_
#define LOGDEF_HPP_

static void LOGI_Mat(const char * matDescription,cv::Mat * matrix)
{
	if (matrix == NULL)
		LOGI("%s is null");

	for (int i = 0; i < matrix->rows; i++)
	{
		for (int j = 0; j < matrix->cols*matrix->channels(); j++)
		{
			LOGI("%s[%d,%d]=%lf",matDescription,i,j,matrix->at<double>(i,j));
		}
	}
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
