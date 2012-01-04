#include <android/log.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"

#ifndef LOG_TAG
#define  LOG_TAG    ""
#endif

#define  LOG_TAG_BASE    "AmplifyR-"
#define  LOG_TAG_TIME    "AmplifyR-Time"

#define STR(tok) #tok

#define  LOGD(TAG,...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG_BASE TAG,__VA_ARGS__)
#define  LOGI(TAG,...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG_BASE TAG,__VA_ARGS__)
#define  LOGW(TAG,...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG_BASE TAG,__VA_ARGS__)
#define  LOGE(TAG,...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG_BASE TAG,__VA_ARGS__)


#define calc_time(start,end) (((end.tv_sec*1000000000LL + end.tv_nsec) - (start.tv_sec*1000000000LL + start.tv_nsec))/1000000LL)

#define SET_TIME(timeObject) clock_gettime(CLOCK_PROCESS_CPUTIME_ID, timeObject);

#define LOG_TIME(message,start,end) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG_TIME,"%s took %ld ms",message,calc_time(start,end))

#ifndef LOGDEF_HPP_
#define LOGDEF_HPP_


static void LOGD_Mat(const char * matrixTag, const char * matDescription,cv::Mat * matrix)
{
	if (matrix == NULL)
		LOGD(STR(matrixTag),"%s is null",matDescription);

	for (int i = 0; i < matrix->rows; i++)
	{
		char string[300];
		for (int j = 0; j < matrix->cols*matrix->channels(); j++)
		{
			sprintf(string,"%lf",matrix->at<double>(i,j));
		}
		LOGD("%s[:,%d]=%s",matDescription,i,string);
	}
}

static void LOGI_Mat(const char * matrixTag, const char * matDescription,cv::Mat * matrix)
{
	if (matrix == NULL)
		LOGI(STR(matrixTag),"%s is null",matDescription);

	for (int i = 0; i < matrix->rows; i++)
	{
		char string[300];
		for (int j = 0; j < matrix->cols*matrix->channels(); j++)
		{
			sprintf(string,"%lf",matrix->at<double>(i,j));
		}
		LOGI("%s[:,%d]=%s",matDescription,i,string);
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
