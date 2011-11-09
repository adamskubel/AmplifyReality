#include "LogDefinitions.h"
#include "android_native_app_glue.h"
#include <opencv2/core/core.hpp>

#ifndef NATIVE_WIN_RENDER
#define NATIVE_WIN_RENDER

using namespace cv;
using namespace std;

struct NativeWindowRenderer
{
public:
	static void drawToBuffer(ANativeWindow_Buffer* buffer, Mat * drawMatrix);
};
#endif
