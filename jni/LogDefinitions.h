#include <android/log.h>
#include "opencv2/highgui/highgui.hpp"

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
