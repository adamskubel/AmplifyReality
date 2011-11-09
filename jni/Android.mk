LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

OPENCV_CAMERA_MODULES:=off

OPENCV_MK_PATH= ../OpenCV/OpenCV-2.3.1/share/OpenCV/OpenCV.mk
include $(OPENCV_MK_PATH)


#include ../OpenCV/includeOpenCV.mk
#OPENCV_MK_PATH = 
#ifeq ("$(wildcard $(OPENCV_MK_PATH))","")
#	#try to load OpenCV.mk from default install location
#	include $(TOOLCHAIN_PREBUILT_ROOT)/user/share/OpenCV/OpenCV.mk
#else
#	include $(OPENCV_MK_PATH)
#endif



LOCAL_MODULE    := amplify_reality
LOCAL_SRC_FILES := Main.cpp  \ display/NativeWindowRenderer.cpp  \ positioning/QRFinder.cpp \ positioning/FindPattern.cpp  \ datacollection/ImageCollector.cpp \ display/opengl/OpenGLRenderer.cpp \ controllers/CalibrationController.cpp
LOCAL_LDLIBS +=  -llog -ldl -landroid -lGLESv1_CM -lEGL
LOCAL_STATIC_LIBRARIES += android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)