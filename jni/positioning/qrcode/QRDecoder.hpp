#ifndef QR_DECODER_HPP_
#define QR_DECODER_HPP_

#include "QRCode.hpp"
#include <opencv2/core/core.hpp>
#include "PerspectiveTransform.h"
#include "zxing/common/BitMatrix.h"
#include "zxing/qrcode/decoder/Decoder.h"
#include "zxing/common/Counted.h"
#include "model/DebugShape.hpp"
#include "controllers/ARControllerDebugUI.hpp"

using namespace cv;
class QRDecoder
{
public:
	QRDecoder(ARControllerDebugUI * config);
	~QRDecoder();
	bool DecodeQRCode(Mat & image, QRCode * qrCode, vector<Drawable*> & debugVector);

private:
	zxing::qrcode::Decoder * decoder;
	ARControllerDebugUI * config;
};

#endif