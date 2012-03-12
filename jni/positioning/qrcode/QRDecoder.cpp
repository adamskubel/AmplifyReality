#include "QRDecoder.hpp"

#define QR_DECODE_DEBUGGING false

QRDecoder::QRDecoder(ARControllerDebugUI * _config)
{
	config = _config;
	decoder = new zxing::qrcode::Decoder();
}

QRDecoder::~QRDecoder()
{	
	delete decoder;
	LOGI(LOGTAG_QR,"Finished deleting decoder");
}

bool QRDecoder::DecodeQRCode(Mat * grayImage, Mat * binaryImage, QRCode * qrCode, vector<Drawable*> & debugVector)
{
	int debugLevel = config->GetIntegerParameter("ARControllerDebug");

	zxing::Ref<zxing::PerspectiveTransform> pt = qrCode->getPerspectiveTransform();

	float moduleSize = qrCode->getModuleSize();
	float codeDimension = qrCode->getCodeDimension();
	float codeSize = moduleSize * codeDimension;
	Rect window = qrCode->getBoundaryRectangle();
	
	Point2f startPoint = Point2f(0,0);
	Point2f endPoint = Point2f(codeSize,codeSize);

	Point2f tmp = startPoint, tmp2 = endPoint;
	vector<float> ptVector;
	ptVector.push_back(tmp.x);
	ptVector.push_back(tmp.y);
	ptVector.push_back(tmp2.x);
	ptVector.push_back(tmp2.y);
	pt->transformPoints(ptVector);

	tmp.x = ptVector[0];
	tmp.y = ptVector[1];
	tmp2.x = ptVector[2];
	tmp2.y = ptVector[3];

		
	ImageProcessor::WindowedThreshold(*grayImage,*binaryImage,window);
	
	if (debugLevel > 0) 
	{
		/*debugVector.push_back(new DebugCircle(Point2i(tmp.x,tmp.y),10,Colors::Green,2));
		debugVector.push_back(new DebugCircle(Point2i(tmp2.x,tmp2.y),10,Colors::Gold,2));*/
		debugVector.push_back(new DebugRectangle(window,Colors::Blue,1));
	}

	zxing::Ref<zxing::BitMatrix> matrix = zxing::Ref<zxing::BitMatrix>(new zxing::BitMatrix(codeDimension));

	struct timespec start,end;
	SET_TIME(&start);
	//Get binary values from image
	int yCount = 0;
	for (float y=startPoint.y; y <= endPoint.y  && yCount < codeDimension; y += moduleSize, yCount++)
	{
		int xCount = 0;
		for (float x=startPoint.x; x <= endPoint.x  && xCount < codeDimension; x += moduleSize, xCount++)
		{
			vector<float> transformVector;
			transformVector.push_back(x+moduleSize/2.0f);
			transformVector.push_back(y+moduleSize/2.0f);
			pt->transformPoints(transformVector);
			Point2i intPoint = Point2i((int)round(transformVector[0]),(int)round(transformVector[1]));

			if (intPoint.x < binaryImage->cols && intPoint.y < binaryImage->rows && intPoint.x >= 0 && intPoint.y >= 0)
			{
				if (binaryImage->at<unsigned char>(intPoint.y,intPoint.x) == 0)
				{	matrix->set(xCount,yCount);
					if (debugLevel == -1)
						debugVector.push_back(new DebugCircle(intPoint,2,Colors::Blue,-1));
				}
				else
				{					
					if (debugLevel == -1)
						debugVector.push_back(new DebugCircle(intPoint,2,Colors::Lime,-1));
				}
			}
		}
	}
	SET_TIME(&end);
	LOG_TIME("QRDecodeLoop",start,end);

#if QR_CODE_DEBUGGING 
	//for (int y=0;y<matrix->getHeight(); y++)
	//{		
	//	std::string qrString;
	//	for (int x=0;x<matrix->getWidth();x++)
	//	{
	//		qrString += matrix->get(x,y) ? '1' : '0';
	//	}
	//	LOGV(LOGTAG_QR,"QRDATA:%s",qrString.c_str());
	//}
	//
	//LOGD(LOGTAG_QR,"Calling decoder");
#endif
	
	bool success = false;
	try
	{
		zxing::Ref<zxing::DecoderResult> result(decoder->decode(matrix));
		LOGD(LOGTAG_QR,"Success! Text = %s",result->getText()->getText().c_str());
		qrCode->TextValue = std::string(result->getText()->getText().c_str());
		success = true;
	}
	catch (exception & exp)
	{
		LOGV(LOGTAG_QR,"Error decoding QRCode: %s", exp.what());
	} 

	return success;
}




