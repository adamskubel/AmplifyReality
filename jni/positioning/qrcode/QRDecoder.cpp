#include "QRDecoder.hpp"



QRDecoder::QRDecoder()
{
	decoder = new zxing::qrcode::Decoder();
}

QRDecoder::~QRDecoder()
{	
	delete decoder;
	LOGI(LOGTAG_QR,"Finished deleting decoder");
}

void QRDecoder::DecodeQRCode(Mat & binaryImage, QRCode * qrCode, vector<Drawable*> & debugVector)
{
	float moduleSize = 0;
	for (int i=0;i<qrCode->finderPatterns.size();i++)
	{
		moduleSize += qrCode->finderPatterns.at(i)->size;
	}
	moduleSize /= 21.0f;

#ifdef QR_DECODE_DEBUGGING
	LOGV(LOGTAG_QR,"Decoding QR Code, modulesize = %f", moduleSize);
#endif

	float numModulesPerSide = 29;
	Point2i topLeft = Point2i((int)round((moduleSize*7)/2.0f),(int)round((moduleSize*7)/2.0f));
	Point2i topRight = Point2i(topLeft.x + (numModulesPerSide-7)*moduleSize, topLeft.y);
	Point2i bottomRight_Alignment = Point2i((int)round((numModulesPerSide-7.5f)*moduleSize), (int)round((numModulesPerSide-7.5f)*moduleSize));
	Point2i bottomLeft = Point2i(topLeft.x, (int)round(topLeft.y + (numModulesPerSide-7.0f)*moduleSize));

	Point2i imageTopLeft = qrCode->finderPatterns.at(0)->pt;
	Point2i imageTopRight = qrCode->finderPatterns.at(1)->pt;
	Point2i imageBottomRight_Alignment = qrCode->alignmentPattern;
	Point2i imageBottomLeft = qrCode->finderPatterns.at(2)->pt;

#ifdef QR_DECODE_DEBUGGING
	LOGV(LOGTAG_QR,"Creating perspective transform");
#endif
	
	PerspectiveTransform pt = PerspectiveTransform::QuadrilateralToQuadrilateral
		(topLeft,topRight,bottomRight_Alignment,bottomLeft,
		imageTopLeft,imageTopRight,imageBottomRight_Alignment,imageBottomLeft);

	Point2f startPoint = Point2f(moduleSize/2.0f,moduleSize/2.0f);
	Point2f endPoint = Point2f((numModulesPerSide)*moduleSize, (numModulesPerSide)*moduleSize);

#ifdef QR_DECODE_DEBUGGING

	LOGV(LOGTAG_QR,"CodeStart = (%f,%f), CodeEnd = (%f,%f)",startPoint.x,startPoint.y,endPoint.x,endPoint.y);

	Point2f tmp = startPoint, tmp2 = endPoint;

	pt.TransformPoint(tmp);
	pt.TransformPoint(tmp2);
	LOGV(LOGTAG_QR,"Transformed: CodeStart = (%f,%f), CodeEnd=(%f,%f)",tmp.x,tmp.y,tmp2.x,tmp2.y);

	debugVector.push_back(new DebugCircle(tmp,10,Colors::Green,true));
	debugVector.push_back(new DebugCircle(tmp2,10,Colors::Gold,true));

	LOGD(LOGTAG_QR,"Num modules per side: %f",numModulesPerSide);

#endif


	zxing::Ref<zxing::BitMatrix> matrix = zxing::Ref<zxing::BitMatrix>(new zxing::BitMatrix(numModulesPerSide));
	//matrix->clear();

	

	//Get binary values from image
	int yCount = 0;
	for (float y=startPoint.y; y <= endPoint.y; y += moduleSize, yCount++)
	{
		int xCount = 0;
		for (float x=startPoint.x; x <= endPoint.x; x += moduleSize, xCount++)
		{
			Point2f samplePoint(x,y);
			pt.TransformPoint(samplePoint);
			if (binaryImage.at<unsigned char>((int)round(samplePoint.y),(int)round(samplePoint.x)) == 0)
				matrix->set(xCount,yCount);
		}
		//LOGD(LOGTAG_QR,"For y = %d, xCount=%d",yCount,xCount);
	}
	
#ifdef QR_CODE_DEBUGGING
	//Log each row to make sure it's set right
	for (int y=0;y<matrix->getHeight(); y++)
	{		
		std::string qrString;// = new char[matrix->getWidth()];
		for (int x=0;x<matrix->getWidth();x++)
		{
			qrString += matrix->get(x,y) ? '1' : '0';
		}
		LOGV(LOGTAG_QR,"QRDATA:%s",qrString.c_str());
	}
	
	LOGD(LOGTAG_QR,"Calling decoder");
#endif
	
	try
	{
		zxing::Ref<zxing::DecoderResult> result(decoder->decode(matrix));
		LOGV(LOGTAG_QR,"Text = %s",result->getText()->getText().c_str());
		qrCode->TextValue = std::string(result->getText()->getText().c_str());
	}
	catch (exception & exp)
	{
		LOGD("Error decoding QRCode: %s", exp.what());
	}

//	delete matrix;
	
//	LOGV(LOGTAG_QR,"Exit decode");
}




