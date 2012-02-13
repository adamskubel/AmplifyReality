LOCAL_PATH := $(call my-dir)

#--Build QCAR---#
#include $(CLEAR_VARS)
#QCAR_PATH = C:/Android/qcar-android-1-0-6/
#TARGET_ARCH_ABI = armeabi-v7a
#LOCAL_MODULE := QCAR-prebuilt
#LOCAL_SRC_FILES = qcar-android-1-0-6\build\lib\$(TARGET_ARCH_ABI)\libQCAR.so
#LOCAL_EXPORT_C_INCLUDES := $(QCAR_PATH)/build/include
#include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)

OPENCV_CAMERA_MODULES:=off
OPENCV_MK_PATH= ${OPENCV_HOME}/share/OpenCV/OpenCV.mk
include $(OPENCV_MK_PATH)



LOCAL_MODULE    := amplify_reality
LOCAL_SRC_FILES +=  $(wildcard *.cpp) $(wildcard */*.cpp)  $(wildcard */*/*.cpp)  $(wildcard */*/*/*.cpp)

LOCAL_LDLIBS +=  -llog -ldl -landroid -lEGL -lGLESv2 
LOCAL_STATIC_LIBRARIES += android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)