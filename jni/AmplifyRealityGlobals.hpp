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

	#define UI_BUTTON_COLOR Colors::LightSteelBlue
	#define UI_SPECIAL_BUTTON_COLOR Colors::DarkGray
	#define UI_BUTTON_SIZE	Size2i(140,70)
	#define UI_BUTTON_ALT_STATE_COLOR Colors::LightSeaGreen
	#define UI_BUTTON_PRESSED_COLOR Colors::DeepSkyBlue
	#define UI_BORDER_THICKNESS 2
	#define UI_BORDER_COLOR Colors::Transparent
	
	#define UI_ALPHA_ENABLED false

	#define VERSION_STRING "0.47";
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



#endif