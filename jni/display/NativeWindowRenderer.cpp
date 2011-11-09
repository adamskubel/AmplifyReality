#include "NativeWindowRenderer.hpp"


struct timespec start, end;
char myTimeString[100];

void NativeWindowRenderer::drawToBuffer(ANativeWindow_Buffer* buffer, Mat * drawMatrix)
{
	void* pixels = buffer->bits;
	int imWidth = (drawMatrix->cols) * 4;
	for (int y = 0; y < drawMatrix->rows; y++)
	{
		uint32_t* line = (uint32_t*) pixels;
		const uint32_t* Mi = drawMatrix->ptr<uint32_t>(y);
		memcpy(line,Mi,imWidth);
		pixels = (uint32_t*) pixels + buffer->stride;
	}

}
