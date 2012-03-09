#include <android/log.h>
#include <android_native_app_glue.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "AmplifyRealityGlobals.hpp"

#ifndef LOGDEF_HPP_
#define LOGDEF_HPP_




#define  LOG_TAG_BASE    "AmplifyR-"
#define  LOG_TAG_TIME    "AmplifyR-Time"
#define  LOG_TAG_TIME_BASE    "AmplifyR-Time-"

#define LOGTAG_QR "QR"
#define LOGTAG_QRFAST "QRFast"
#define LOGTAG_ARCONTROLLER "ARController"
#define LOGTAG_MAIN "Main"
#define LOGTAG_OPENGL "OpenGL"
#define LOGTAG_IMAGECAPTURE "ImageCapture"
#define LOGTAG_IMAGEPROCESSING "ImageProcessing"
#define LOGTAG_INPUT "Input"
#define LOGTAG_CALIBRATION "Calibration"
#define LOGTAG_SENSOR "Sensor"
#define LOGTAG_POSITION "Positioning"
#define LOGTAG_IO "IO"
#define LOGTAG_NETWORKING "Networking"
#define LOGTAG_JNI "JNI"
#define LOGTAG_WORLD "WorldLoader"
#define LOGTAG_ARINPUT "ARInput"


#define STR(tok) #tok

#define DETAILED_LOGGING_ENABLED 


#define calc_time_double(start,end) (((end.tv_sec*1000000000.0 + end.tv_nsec) - (start.tv_sec*1000000000.0 + start.tv_nsec))/1000.0)
#define LOG_TIME_PRECISE(message,start,end) __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG_TIME,"%s took %10.3lf us",message,calc_time_double(start,end))


#define calc_time(start,end) (((end.tv_sec*1000000000LL + end.tv_nsec) - (start.tv_sec*1000000000LL + start.tv_nsec))/1000000LL)
#define LOGV(TAG,...)  __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG_BASE TAG,__VA_ARGS__) 
#define LOG_TIME(message,start,end) __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG_TIME,"%s took %ld ms",message,calc_time(start,end))
#define LOG_TIME_DEBUG(message,start,end) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG_TIME,"%s took %ld ms",message,calc_time(start,end))
#define SET_TIME(timeObject) clock_gettime(CLOCK_REALTIME_HR, timeObject);

//Disabling verbose and time logging *MAY* improve performance (these events fire really often)
#ifndef DETAILED_LOGGING_ENABLED
	#define LOGV(TAG,...)  
	#define LOG_TIME(message,start,end)
	#define SET_TIME(timeObject)
#endif

#define  LOGD(TAG,...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG_BASE TAG,__VA_ARGS__)
#define  LOGI(TAG,...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG_BASE TAG,__VA_ARGS__)
#define  LOGW(TAG,...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG_BASE TAG,__VA_ARGS__)

//Error is always global
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,"AmplifyR",__VA_ARGS__)



/*
Suggested logcat filters:
#1
AmplifyR:D AmplifyR-Main:D AmplifyR-QR:D AmplifyR-ImageCapture:D AmplifyR-ImageProcessing:D *:S
#2
AmplifyR:D AmplifyR-ARController:D AmplifyR-Main:V AmplifyR-QR:D AmplifyR-ImageCapture:I AmplifyR-ImageProcessing:D  AmplifyR-Calibration:I AmplifyR-Positioning:I *:S

AmplifyR:D AmplifyR-ARController:D AmplifyR-Main:V AmplifyR-QR:D AmplifyR-ImageCapture:I AmplifyR-ImageProcessing:I AmplifyR-Calibration:I AmplifyR-Positioning:V AmplifyR-Sensor:I AmplifyR-OpenGL:D *:S

AmplifyR:D AmplifyR-ARController:D AmplifyR-Main:D AmplifyR-QR:D AmplifyR-ImageCapture:I AmplifyR-ImageProcessing:I AmplifyR-Calibration:V AmplifyR-Positioning:D AmplifyR-Sensor:I AmplifyR-OpenGL:D AmplifyR-Input:D *:S
*/





static void LOG_INTRO()
{
	LOGI(LOGTAG_MAIN,"******************************");
	LOGI(LOGTAG_MAIN,"     --Amplified Reality--    ");
	LOGI(LOGTAG_MAIN,"       Version %s             ","0.02");
	LOGI(LOGTAG_MAIN,"         Adam Skubel          ");
	LOGI(LOGTAG_MAIN,"******************************");
}


//static std::string GetFormatFromDepth(int depth)
//{
//	switch(depth)
//	{
//	case CV_8U:
//		return "%u";
//	case CV_8S:
//		return "%d";
//	case CV_16S:
//		return "%d";
//	case CV_32S:
//		return "%d";
//	case CV_32F:
//		return "%f";
//	case CV_64F:
//		return "%lf";
//	}
//	return "UNKNOWNTYPE: %d";
//	
//
//}
//
//static std::string WriteMatrixCell(cv::Mat * matrix,std::string & matString)
//{
//	switch(matrix->depth())
//	{
//	case CV_8U:
//		sprintf(matString,"%s,%u",matString,matrix->at<
//	case CV_8S:
//		return "%d";
//	case CV_16S:
//		return "%d";
//	case CV_32S:
//		return "%d";
//	case CV_32F:
//		return "%f";
//	case CV_64F:
//		return "%lf";
//	}
//	return "UNKNOWNTYPE: %d";
//	
//}

static void LOG_Mat(android_LogPriority logPriority, std::string matrixTag, const char * matDescription,cv::Mat * matrix)
{
	if (matrix == NULL)
	{
		std::string prefix = std::string("AmplifyR-");
		__android_log_print(logPriority,(prefix.append(matrixTag)).c_str(),"%s is NULL",matDescription);
	}
	else if (matrix->size().area() == 0)
	{
		std::string prefix = std::string("AmplifyR-");
		__android_log_print(logPriority,(prefix.append(matrixTag)).c_str(),"%s is empty",matDescription);
	}
	else
	{		
		int charCount = 0;
		std::stringstream textStream;		
		textStream.precision(2);
		textStream.setf(std::ios_base::fixed);
		int maxCol = matrix->cols*matrix->channels();

		for (int i = 0; i < matrix->rows; i++)
		{			
			textStream << "[";
			for (int j = 0; j < maxCol; j++)
			{		
				switch(matrix->depth())
				{
				case CV_8U:
					textStream << matrix->at<unsigned char>(i,j);
					break;
				case CV_8S:
					textStream << matrix->at<signed char>(i,j);
					break;
				case CV_16S:
					textStream << matrix->at<signed short>(i,j);
					break;
				case CV_32S:
					textStream << matrix->at<int>(i,j);
					break;
				case CV_32F:
					textStream << matrix->at<float>(i,j);
					break;
				case CV_64F:
					textStream << matrix->at<double>(i,j);
					break;
				default:
					textStream << "?";
				}
				
				if (j != (maxCol-1))
					textStream << ",";
			}
			textStream << "]\n";			
		}
		std::string prefix = std::string("AmplifyR-");
		__android_log_print(logPriority,(prefix.append(matrixTag)).c_str(),"%s=\n%s",matDescription,textStream.str().c_str());
	}
}

static void LOGD_Mat(std::string matrixTag, const char * matDescription,cv::Mat * matrix)
{
	LOG_Mat(ANDROID_LOG_DEBUG,matrixTag,matDescription,matrix);
}


static void LOG_Vector(android_LogPriority logPriority, std::string vectorTag, const char * vectorDescription,std::vector<cv::Point3f> * pointVector)
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
		sprintf(string,"%s,(%f,%f,%f)",string,pointVector->at(i).x,pointVector->at(i).y, pointVector->at(i).z);
	}
	sprintf(string,"%s]",string);
	std::string prefix = std::string("AmplifyR-");
	__android_log_print(logPriority,(prefix.append(vectorTag)).c_str(),"%s=%s",vectorDescription,string);
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

static void LOG_Vector(android_LogPriority logPriority, std::string vectorTag, const char * vectorDescription,std::vector<cv::Point2i> * pointVector)
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
		sprintf(string,"%s,(%d,%d)",string,pointVector->at(i).x,pointVector->at(i).y);
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

