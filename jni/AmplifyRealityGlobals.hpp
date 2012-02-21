#ifndef AMPLIFY_REALITY_GLOBALS_HPP_
#define AMPLIFY_REALITY_GLOBALS_HPP_

	#define AMPLIFY_REALITY_DEBUG 

	#define ENABLE_TEXTURE_COLOR false
	#define DRAW_BACKGROUND_BORDER false
	#define ENABLE_UNDISTORT false
	#define USE_ADAPTIVE_THRESH false
	#define USE_LOCAL_THRESH false

	#define USE_FEEDBACK_THRESH false

	#define MIN_CALIBRATION_SAMPLES 4
	#define MAX_CALIBRATION_SAMPLES 16
	#define USE_CALCULATED_CAMERA_MATRIX false


	#define VERSION_STRING "0.01";

	#define NO_ICONV true //Required for compiling zxing..doesn't work here though

	//#define IS_NEXUS_S  //<--- UNCOMMENT THIS LINE FOR NEXUS S AND COMMENT FOR HTC SENSATION


	
	//HTC Sensation Camera Properties
	#define HTC_SENSATION_CAMERA_IMAGE_WIDTH 800
	#define HTC_SENSATION_CAMERA_IMAGE_HEIGHT 480
	//#define HTC_SENSATION_CAMERA_MATRIX {92.5, 0, 400, 0, 92.5, 240, 0, 0 ,1};
	#define HTC_SENSATION_CAMERA_MATRIX {771.814475, 0.0, 379.685733, 0.0, 780.082272, 283.918826, 0.0, 0.0 ,1.0}; //This one is from actual calibration
	#define HTC_SENSATION_DISTORTION_MATRIX {0, 0, 0, 0, 0};

	//NEXUS S Camera Properties
	#define NEXUS_S_CAMERA_IMAGE_WIDTH 720
	#define NEXUS_S_CAMERA_IMAGE_HEIGHT 480
	#define NEXUS_S_CAMERA_MATRIX {934, 0, 400, 0, 934, 260, 0, 0 ,1}; //This one is from actual calibration

//Camera
//=[,771.814475,0.000000,379.685733]
//=[,0.000000,780.082272,283.918826]
//=[,0.000000,0.000000,1.000000]

//Distortion
//:,0]=[,0.200291,-1.526604,0.002497,-0.018703,3.546408]

	#define NEXUS_S_DISTORTION_MATRIX {0, 0, 0, 0, 0};

	//Default camera properties are for HTC sensation
	#define CAMERA_IMAGE_WIDTH HTC_SENSATION_CAMERA_IMAGE_WIDTH
	#define CAMERA_IMAGE_HEIGHT HTC_SENSATION_CAMERA_IMAGE_HEIGHT
	#define DEFAULT_CAMERA_MATRIX HTC_SENSATION_CAMERA_MATRIX
	#define DEFAULT_DISTORTION_MATRIX HTC_SENSATION_DISTORTION_MATRIX


	#ifdef IS_NEXUS_S
		#define CAMERA_IMAGE_WIDTH NEXUS_S_CAMERA_IMAGE_WIDTH
		#define CAMERA_IMAGE_HEIGHT NEXUS_S_CAMERA_IMAGE_HEIGHT
		#define DEFAULT_CAMERA_MATRIX NEXUS_S_CAMERA_MATRIX
		#define DEFAULT_DISTORTION_MATRIX NEXUS_S_DISTORTION_MATRIX
	#endif





	//Some common math functions, should go elsewhere 
	static bool IsClockWise(cv::Point2f p1, cv::Point2f p2, cv::Point2f p0)
	{
		return ((p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x)) > 0;
	}

	static int ipow(int base, int exp)
	{
		int result = 1;
		while (exp)
		{
			if (exp & 1)
				result *= base;
			exp >>= 1;
			base *= base;
		}

		return result;
	}
	
	static int GetSquaredDistance(cv::Point2i pt0, cv::Point2i pt1)
	{
		return ipow(pt0.x-pt1.x,2) + ipow(pt0.y-pt1.y,2);
	} 

	static float GetPointDistance(cv::Point2f p1, cv::Point2f p2)
	{
		return sqrt(pow(abs(p1.x-p2.x),2) + pow(abs(p1.y-p2.y),2));
	}

#endif