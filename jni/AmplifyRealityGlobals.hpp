#ifndef AMPLIFY_REALITY_GLOBALS_HPP_
#define AMPLIFY_REALITY_GLOBALS_HPP_

	#define AMPLIFY_REALITY_DEBUG 

	#define ENABLE_TEXTURE_COLOR false
	#define DRAW_BACKGROUND_BORDER false
	#define ENABLE_UNDISTORT false
	#define USE_ADAPTIVE_THRESH false
	#define USE_LOCAL_THRESH false

	#define USE_FEEDBACK_THRESH false

	#define NUM_CALIBRATION_SAMPLES 4
	#define USE_CALCULATED_CAMERA_MATRIX false


	#define VERSION_STRING "0.01";

	#define NO_ICONV true //Required for compiling zxing..doesn't work here though

	//#define IS_NEXUS_S  //<--- UNCOMMENT THIS LINE FOR NEXUS S AND COMMENT FOR HTC SENSATION


	
	//HTC Sensation Camera Properties
	#define HTC_SENSATION_CAMERA_IMAGE_WIDTH 800
	#define HTC_SENSATION_CAMERA_IMAGE_HEIGHT 480
	//#define HTC_SENSATION_CAMERA_MATRIX {92.5, 0, 400, 0, 92.5, 240, 0, 0 ,1};
	#define HTC_SENSATION_CAMERA_MATRIX {934, 0, 400, 0, 934, 260, 0, 0 ,1}; //This one is from actual calibration
	#define HTC_SENSATION_DISTORTION_MATRIX {0, 0, 0, 0, 0};

	//NEXUS S Camera Properties
	#define NEXUS_S_CAMERA_IMAGE_WIDTH 720
	#define NEXUS_S_CAMERA_IMAGE_HEIGHT 480
	#define NEXUS_S_CAMERA_MATRIX {934, 0, 400, 0, 934, 260, 0, 0 ,1}; //This one is from actual calibration
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


	//OpenCV-compatible .NET Colors

	#define COLOR_TYPE static const cv::Scalar
	namespace Colors
	{	
		COLOR_TYPE Transparent(0xFF,0xFF,0xFF,0x00);

		COLOR_TYPE White(0xFF,0xFF,0xFF,0xFF);
		COLOR_TYPE Black(0x00,0x00,0x00,0xFF);
		COLOR_TYPE Blue(0x00,0x00,0xFF,0xFF);
		COLOR_TYPE Red(0xFF,0x00,0x00,0xFF);
		COLOR_TYPE Green(0x00,0x80,0x00,0xFF);
	
		COLOR_TYPE Aqua(0x00,0xFF,0xFF,0xFF);
		COLOR_TYPE Gold(0xFF,0xD7,0x00,0xFF);
		COLOR_TYPE CornflowerBlue(0x64,0x95,0xED,0xFF);
		COLOR_TYPE Crimson(0xDC,0x14,0x3C,0xFF);
		COLOR_TYPE MidnightBlue(0x19,0x19,0x70,0xFF);
		COLOR_TYPE OliveDrab(0x6B,0x8E,0x23,0xFF);
		COLOR_TYPE MediumSeaGreen(0x66,0xCD,0xAA,0xFF);
		COLOR_TYPE PeachPuff(0xFF,0xDA,0xB9,0xFF);
		COLOR_TYPE Lime(0x00,0xFF,0x00,0xFF);


		static cv::Scalar RandomColor()
		{
			return cv::Scalar(std::rand() % 255,std::rand() % 255,std::rand() % 255,255);
		}
	}
	#undef COLOR_TYPE



#endif