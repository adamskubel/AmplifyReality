LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

OPENCV_CAMERA_MODULES:=off

OPENCV_MK_PATH= ${OPENCV_HOME}/share/OpenCV/OpenCV.mk
include $(OPENCV_MK_PATH)


LOCAL_MODULE    := amplify_reality
LOCAL_SRC_FILES :=	\
Main.cpp\
AmplifyRunner.cpp \
positioning/QRFinder.cpp\
positioning/FindPattern.cpp \
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
positioning/QRLocator.cpp \
model/FrameItem.cpp \
datacollection/ImageProcessor.cpp \
userinterface/AndroidInputHandler.cpp  \
userinterface/events/EventArgs.cpp \
userinterface/uimodel/Button.cpp \
userinterface/uimodel/Label.cpp \
userinterface/uimodel/GridLayout.cpp \
VirtualDeclarations.cpp


LOCAL_LDLIBS +=  -llog -ldl -landroid -lGLESv1_CM -lEGL
LOCAL_STATIC_LIBRARIES += android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)