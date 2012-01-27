LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

OPENCV_CAMERA_MODULES:=off

#LIB_STL_PATH = C:\Android\android-ndk-r7\sources\cxx-stl\gnu-libstdc++\Android.mk
#include $(LIB_STL_PATH)

OPENCV_MK_PATH= ${OPENCV_HOME}/share/OpenCV/OpenCV.mk
include $(OPENCV_MK_PATH)




LOCAL_MODULE    := amplify_reality
LOCAL_SRC_FILES :=	\
Main.cpp\
AmplifyRunner.cpp \
datastructures/CircularList.cpp\
positioning/QRFinder.cpp\
positioning/FindPattern.cpp \
positioning/PositionSelector.cpp\
positioning/ARConfigurator.cpp\
datacollection/ImageCollector.cpp \
datacollection/SensorCollector.cpp\
display/opengl/OpenGLRenderer.cpp \
display/opengl/OpenGLHelper.cpp \
display/opengl/QuadBackground.cpp\
display/model/AugmentedView.cpp\
display/model/ARObject.cpp\
controllers/CalibrationController.cpp \
controllers/ARController.cpp \
controllers/Controller.cpp \
controllers/StartupController.cpp\
positioning/QRLocator.cpp \
model/FrameItem.cpp \
datacollection/ImageProcessor.cpp \
userinterface/AndroidInputHandler.cpp  \
userinterface/events/EventArgs.cpp \
userinterface/uimodel/Button.cpp \
userinterface/uimodel/Label.cpp \
userinterface/uimodel/GridLayout.cpp \
userinterface/uimodel/CertaintyIndicator.cpp\
userinterface/uimodel/NumberSpinner.cpp\
VirtualDeclarations.cpp


LOCAL_LDLIBS +=  -llog -ldl -landroid -lGLESv1_CM -lEGL
LOCAL_STATIC_LIBRARIES += android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)