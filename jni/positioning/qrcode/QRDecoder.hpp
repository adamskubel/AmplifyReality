#ifndef QR_DECODER_HPP_
#define QR_DECODER_HPP_

#include "QRCode.hpp"
#include <opencv2/core/core.hpp>
#include "zxing/common/PerspectiveTransform.h"
#include "zxing/common/BitMatrix.h"
#include "zxing/qrcode/decoder/Decoder.h"
#include "zxing/common/Counted.h"
#include "model/DebugShape.hpp"
#include "controllers/ARControllerDebugUI.hpp"
#include "datacollection/ImageProcessor.hpp"

using namespace cv;
class QRDecoder
{
public:
	QRDecoder(ARControllerDebugUI * config);
	~QRDecoder();
	bool DecodeQRCode(Mat * grayImage, Mat * binaryImage, QRCode * qrCode, vector<Drawable*> & debugVector);

private:
	zxing::qrcode::Decoder * decoder;
	ARControllerDebugUI * config;
};

#endif