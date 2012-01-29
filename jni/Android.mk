LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

OPENCV_CAMERA_MODULES:=off

#LIB_STL_PATH = C:\Android\android-ndk-r7\sources\cxx-stl\gnu-libstdc++\Android.mk
#include $(LIB_STL_PATH)

OPENCV_MK_PATH= ${OPENCV_HOME}/share/OpenCV/OpenCV.mk
include $(OPENCV_MK_PATH)


LOCAL_MODULE    := amplify_reality

LOCAL_SRC_FILES +=  $(wildcard *.cpp) $(wildcard */*.cpp)  $(wildcard */*/*.cpp)  $(wildcard */*/*/*.cpp)


LOCAL_LDLIBS +=  -llog -ldl -landroid -lGLESv1_CM -lEGL
LOCAL_STATIC_LIBRARIES += android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)