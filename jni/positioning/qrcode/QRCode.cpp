#include "QRCode.hpp"




QRCode::QRCode(vector<FinderPattern*> _finderPatterns) 
{ 
	QRCode::instanceCount++;
	finderPatterns = _finderPatterns;
	trackingCorners.clear();		

	QRCodeDimension = 10.0f;
}

QRCode::~QRCode() 
{		
	QRCode::instanceCount--;
	while (!finderPatterns.empty())
	{
		delete finderPatterns.back();
		finderPatterns.pop_back();
	}	
}

static Point2i ExtendLine(Point2i pt0, Point2i pt1, Point2i & diff, float scale)
{
	/*diff = pt1 - pt0;
	Point2f floatEnd = Point2f(pt0.x,pt0.y) + (Point2f(diff.x,diff.y) * scale);
	return Point2i((int)round(floatEnd.x),(int)round(floatEnd.y));*/
	int dx = pt1.x - pt0.x;
	int dy = pt1.y - pt0.y;
	diff = Point2i(dx,dy);

	float xDelta = (float)dx * scale;
	float yDelta = (float)dy * scale;

	xDelta += (float)pt0.x;
	yDelta += (float)pt0.y;

	return Point2i((int)round(xDelta),(int)round(yDelta));
}

void QRCode::SetAlignmentCorners(vector<Point2i> & alignmentCorners)
{
	std::sort(alignmentCorners.begin(),alignmentCorners.end(),PointCompare(codeCenter));
	alignmentPattern = alignmentCorners.back();
	LOGD(LOGTAG_QR,"Selected alignment pattern (%d,%d)",alignmentPattern.x,alignmentPattern.y);
}

float QRCode::getAvgPatternSize()
{
	float avgPatternSize = 0;
	for (int i=0;i<finderPatterns.size();i++)
	{
		avgPatternSize += finderPatterns[i]->size;
	}
	avgPatternSize = idiv(avgPatternSize,finderPatterns.size());
	return avgPatternSize;
}


int QRCode::getCodeDimension()
{
	return 29;
}

float QRCode::getModuleSize()
{
	return QRCodeDimension;
}

void QRCode::setModuleSize(float moduleSize)
{
	QRCodeDimension = moduleSize;
}


static Point2f getIntersectionPoint(Point2f p1, Point2f p2, Point2f p3, Point2f p4)
{
	float data[] = { p2.x - p1.x, -(p4.x-p3.x), p2.y - p1.y, -(p4.y-p3.y)};
	Mat m = Mat(2,2,CV_32F,data);
	Mat p = Mat(2,1,CV_32F);
	p.at<float>(0,0) = p3.x - p1.x;
	p.at<float>(1,0) = p3.y - p1.y;

	Mat mInv = Mat(m.inv());

	Mat answer = Mat(2,1,CV_32F);
	answer = mInv * p;

	float s = answer.at<float>(0,0);
	//float t = answer.at<float>(1,0); 

	return Point2f(p1.x + (p2.x-p1.x)*s, p1.y + (p2.y-p1.y)*s);
}


bool QRCode::GuessAlignmentPosition(Point2i & result, Rect & searchArea)
{
	const float APError = 0.1f;
	const float FinderPatternAlignmentRatioLow = (21.0f/7.0f) * (1.0f-APError); 
	const float FinderPatternAlignmentRatioCenter = (22.5f/7.0f); 
	const float FinderPatternAlignmentRatioHigh = (24.0f/7.0f) * (1.0f+APError);

	if (finderPatterns.size() == 3)
	{			

		if (finderPatterns[1]->patternCorners.size() != 4 || finderPatterns[2]->patternCorners.size() != 4)
		{			
			int fpSize = (int)round(finderPatterns[0]->size/2.0f);
			result.x = (finderPatterns[1]->pt.x - finderPatterns[0]->pt.x) + finderPatterns[2]->pt.x - fpSize;
			result.y = (finderPatterns[2]->pt.y - finderPatterns[0]->pt.y) + finderPatterns[1]->pt.y - fpSize;		
			alignmentPattern = result;

			fpSize = (int)(round(fpSize * 1.5f));

			int startX = result.x - fpSize, endX = result.x + fpSize;
			int startY = result.y- fpSize, endY = result.y + fpSize;

			searchArea = Rect(Point2i(startX,startY),Point2i(endX,endY));

			return false;
		}
		else
		{
			sortCorners();
			for (int i=0;i<finderPatterns.size();i++)
			{
				finderPatterns[i]->SortCorners();
			}

			//Get AP-aligning corners
			if (finderPatterns[1]->patternCorners.size()  < 4 || finderPatterns[2]->patternCorners.size() < 4)
			{
				int fpSize = (int)round(finderPatterns[0]->size/2.0f);
				result.x = (finderPatterns[1]->pt.x - finderPatterns[0]->pt.x) + finderPatterns[2]->pt.x - fpSize;
				result.y = (finderPatterns[1]->pt.y - finderPatterns[0]->pt.y) + finderPatterns[2]->pt.y - fpSize;		
				alignmentPattern = result;
				
				fpSize = (int)(round(fpSize * 1.5f));

				int startX = result.x - fpSize, endX = result.x + fpSize;
				int startY = result.y- fpSize, endY = result.y + fpSize;

				searchArea = Rect(Point2i(startX,startY),Point2i(endX,endY));

				return false;
			}

			Point2i upperRight0 = finderPatterns[1]->patternCorners[3];
			Point2i upperRight1 = finderPatterns[1]->patternCorners[2];

			Point2i lowerLeft0 = finderPatterns[2]->patternCorners[1];
			Point2i lowerLeft1 = finderPatterns[2]->patternCorners[2];

			LOGV(LOGTAG_QR,"Calculating AP position. Corner1=(%d,%d), Corner2=(%d,%d)",upperRight0.x,upperRight0.y,upperRight1.x,upperRight1.y);
			LOGV(LOGTAG_QR,"Calculating AP position. Corner3=(%d,%d), Corner4=(%d,%d)",lowerLeft0.x,lowerLeft0.y,lowerLeft1.x,lowerLeft1.y);

			Point2i diff1;
			Point2i ap0 = ExtendLine(upperRight0,upperRight1,diff1,FinderPatternAlignmentRatioLow);
			Point2i ap2 = ExtendLine(upperRight0,upperRight1,diff1,FinderPatternAlignmentRatioHigh);

			Point2i diff2;
			Point2i ap3 = ExtendLine(lowerLeft0,lowerLeft1,diff2,FinderPatternAlignmentRatioLow);
			Point2i ap5 = ExtendLine(lowerLeft0,lowerLeft1,diff2,FinderPatternAlignmentRatioHigh);

			int dx1 = abs(ap0.x - ap2.x);
			int dx2 = abs(ap3.x - ap5.x);

			int dy1 = abs(ap0.y - ap2.y);
			int dy2 = abs(ap3.y - ap5.y);

			int dx = ((float) MAX(dx1,dx2)) * 1.2f;
			int dy = ((float)MAX(dy1,dy2)) * 1.2f;
			
			dx = idiv(dx,2);
			dy = idiv(dy,2);

			LOGV(LOGTAG_QR,"Finding intersection point");
			Point2f center = getIntersectionPoint(upperRight0,upperRight1,lowerLeft0,lowerLeft1);
			LOGV(LOGTAG_QR,"Intersection point is (%f,%f)",center.x,center.y);
			result = Point2i((int)round(center.x),(int)round(center.y));
			searchArea = Rect(Point2i(center.x - dx, center.y - dy), Point2i(center.x + dx, center.y + dy));
			return true;
		}
	}
	else
	{
		LOGW(LOGTAG_QR,"Not enough finder patterns to guess alignment pattern. Size=%d",finderPatterns.size());
		return false;
	}
}


void QRCode::SetDrawingLevel(int level)
{
	debugDrawingLevel = level;
}

void QRCode::sortCorners()
{
	if (trackingCorners.size() < 3)
	{
		LOGD(LOGTAG_QR,"Sorting corners for %d patterns",finderPatterns.size());

		//Top left, top right, bottom left
		codeCenter = Point2i(0,0);
		for (int i=0;i<finderPatterns.size();i++)
		{
			codeCenter += finderPatterns[i]->pt;
		}
		codeCenter = Point2i(codeCenter.x/(float)finderPatterns.size(),codeCenter.y/(float)finderPatterns.size());

		for (int i=0;i<finderPatterns.size();i++)
		{
			trackingCorners.push_back(finderPatterns[i]->getFarthestCorner(codeCenter));
		}
	}
}

bool QRCode::isValidCode()
{
	return finderPatterns.size() == 3 && alignmentPattern.x != 0 && alignmentPattern.y != 0;
}

bool QRCode::isDecoded()
{
	return isValidCode() && TextValue.length() > 0;
}

zxing::Ref<zxing::PerspectiveTransform> QRCode::getPerspectiveTransform()
{
	vector<Point3f> codePoints;
	vector<Point2f> imagePoints;
	
	getTrackingPoints(imagePoints,codePoints);

	if (codePoints.size() < 4 || imagePoints.size() < 4)
	{
		LOGE("Insufficient points for transform");
		throw exception();
	}
	
	return zxing::PerspectiveTransform::quadrilateralToQuadrilateral
		(codePoints[0].x,codePoints[0].y,codePoints[1].x,codePoints[1].y,codePoints[2].x,codePoints[2].y,codePoints[3].x,codePoints[3].y,
		imagePoints[0].x,imagePoints[0].y,imagePoints[1].x,imagePoints[1].y,imagePoints[2].x,imagePoints[2].y,imagePoints[3].x,imagePoints[3].y);
}

Rect QRCode::getBoundaryRectangle()
{
	float error = 0.1f;
	zxing::Ref<zxing::PerspectiveTransform> pt = getPerspectiveTransform();

	int codeSize = getCodeDimension() * getModuleSize();
	vector<Point2f> corners;
	corners.push_back(Point2f(0,0));
	corners.push_back(Point2f(codeSize,0));
	corners.push_back(Point2f(codeSize,codeSize));
	corners.push_back(Point2f(0,codeSize));

	Point2f minPoint(INT_MAX,INT_MAX);
	Point2f maxPoint(0,0);
	
	for (int i=0;i<corners.size();i++)
	{
		vector<float> ptVector;
		ptVector.push_back(corners[i].x);
		ptVector.push_back(corners[i].y);
		pt->transformPoints(ptVector);
		minPoint.x = MIN(minPoint.x,ptVector[0]);
		maxPoint.x = MAX(maxPoint.x,ptVector[0]);
		minPoint.y = MIN(minPoint.y,ptVector[1]);
		maxPoint.y = MAX(maxPoint.y,ptVector[1]);
	}
	Point2f sizePoint = (maxPoint-minPoint);
	minPoint -= sizePoint * error;
	maxPoint += sizePoint *error;
	Rect rectangle = Rect(minPoint,maxPoint);
	return rectangle;
}

void QRCode::getTrackingPoints(vector<cv::Point2f> & points, vector<Point3f> & qrVector, bool extraPoints)
{
	float finderPatternSpacing = 29.0f * QRCodeDimension;
	float alignmentPatternSpacing = 21.0f * QRCodeDimension;
	float finderPatternSize = 7.0f * QRCodeDimension;

	sortCorners();

	points.push_back(Point2f(trackingCorners.at(0).x,trackingCorners.at(0).y));
	points.push_back(Point2f(trackingCorners.at(1).x,trackingCorners.at(1).y));
	points.push_back(Point2f(alignmentPattern.x,alignmentPattern.y));
	points.push_back(Point2f(trackingCorners.at(2).x,trackingCorners.at(2).y));	

	qrVector.push_back(Point3f(0,0,0));
	qrVector.push_back(Point3f(finderPatternSpacing,0,0));
	qrVector.push_back(Point3f(alignmentPatternSpacing,alignmentPatternSpacing,0)); //alignment pattern
	qrVector.push_back(Point3f(0,finderPatternSpacing,0));
	
	if (extraPoints && finderPatterns.size() == 3)
	{		
		if (finderPatterns[0]->patternCorners.size() == 4)
		{
			points.push_back(finderPatterns[0]->patternCorners[1]);
			qrVector.push_back(Point3f(finderPatternSize,0,0));
						
			points.push_back(finderPatterns[0]->patternCorners[2]);
			qrVector.push_back(Point3f(finderPatternSize,finderPatternSize,0));

			points.push_back(finderPatterns[0]->patternCorners[3]);
			qrVector.push_back(Point3f(0,finderPatternSize,0));
		}

		if (finderPatterns[1]->patternCorners.size() == 4)
		{
			points.push_back(finderPatterns[1]->patternCorners[1]);
			qrVector.push_back(Point3f(finderPatternSpacing,finderPatternSize,0));
						
			points.push_back(finderPatterns[1]->patternCorners[2]);
			qrVector.push_back(Point3f(finderPatternSpacing-finderPatternSize,finderPatternSize,0));

			points.push_back(finderPatterns[1]->patternCorners[3]);
			qrVector.push_back(Point3f(finderPatternSpacing-finderPatternSize,0,0));
		}

		if (finderPatterns[2]->patternCorners.size() == 4)
		{
			points.push_back(finderPatterns[2]->patternCorners[1]);
			qrVector.push_back(Point3f(0,finderPatternSpacing-finderPatternSize,0));
						
			points.push_back(finderPatterns[2]->patternCorners[2]);
			qrVector.push_back(Point3f(finderPatternSize,finderPatternSpacing-finderPatternSize,0));

			points.push_back(finderPatterns[2]->patternCorners[3]);
			qrVector.push_back(Point3f(finderPatternSize,finderPatternSpacing,0));
		}
	}
}

void QRCode::Draw(Mat * rgbaImage)
{
	if ((debugDrawingLevel == -3 || debugDrawingLevel == 2) && isDecoded())
	{
		DebugLabel(finderPatterns[0]->pt,TextValue,Colors::Aqua,1.0f,Colors::Blue).Draw(rgbaImage);
	}

	if (debugDrawingLevel > 0)
	{
		if (isValidCode())
		{
			
			if (trackingCorners.size() != 3) //Use centers
			{
				Point2i points[4];

				points[0] = (finderPatterns.at(0))->pt;
				points[1] = (finderPatterns.at(1))->pt;
				//Alignment Pattern is bottom right point
				points[2] = alignmentPattern;
				points[3] = (finderPatterns.at(2))->pt;

				int npts = 4;
				const Point2i * pArray[] = {points};
				polylines(*rgbaImage,pArray,&npts,1,true,Colors::Red,2);
			}
			else
			{
				vector<Point2i> drawPoints;
				drawPoints.push_back(trackingCorners[0]);
				drawPoints.push_back(trackingCorners[1]);
				drawPoints.push_back(alignmentPattern);
				drawPoints.push_back(trackingCorners[2]);
				DebugPoly(drawPoints,Colors::Lime,2).Draw(rgbaImage);
			}
		}
		else 
		{
			LOGD(LOGTAG_QR,"Code is invalid! FPSize=%d,Apx=%d,Apy=%d",finderPatterns.size(),alignmentPattern.x,alignmentPattern.y);
		}
	}

	if (debugDrawingLevel > 2)
	{

		for (size_t i = 0; i < finderPatterns.size(); i++)
		{
			FinderPattern *  pattern = finderPatterns.at(i);

			if (pattern->patternCorners.size() == 4)
			{
				if (debugDrawingLevel > 3)
				{
					DebugCircle(pattern->patternCorners[0],10,Colors::Red,2).Draw(rgbaImage);
					DebugCircle(pattern->patternCorners[1],10,Colors::Orange,2).Draw(rgbaImage);
					DebugCircle(pattern->patternCorners[2],10,Colors::Yellow,2).Draw(rgbaImage);
					DebugCircle(pattern->patternCorners[3],10,Colors::LemonChiffon,2).Draw(rgbaImage);
				}
				else
				{
					DebugPoly(pattern->patternCorners,Colors::Orchid,2).Draw(rgbaImage);
				}
			}
			else
			{
				Point2i halfSize(idiv(pattern->patternSize.width,2), idiv(pattern->patternSize.height,2));
				DebugRectangle(Rect(pattern->pt - halfSize,pattern->pt + halfSize),Colors::Purple,2).Draw(rgbaImage);
			}
		}
	}

}

int QRCode::FPDistance(FinderPattern * a,  FinderPattern * b)
{
	long xd = a->pt.x - b->pt.x;
	long yd = a->pt.y - b->pt.y;

	return (long) sqrt((double) (xd * xd + yd * yd));
}

int QRCode::FPAcuteAngleGeometry(FinderPattern* a, FinderPattern* b, FinderPattern* c)
{
	return (b->pt.x - a->pt.x) * (c->pt.y - a->pt.y) - (b->pt.y - a->pt.y) * (c->pt.x - a->pt.x);
}

//Use cross product to arrange finder patterns into correct order
QRCode * QRCode::CreateFromFinderPatterns(vector<FinderPattern*> & finderPatterns)
{
	FinderPattern * bottom_left, * top_left, * top_right;

	long d0to1 = FPDistance(finderPatterns.at(0), finderPatterns.at(1));
	long d1to2 = FPDistance(finderPatterns.at(1), finderPatterns.at(2));
	long d0to2 = FPDistance(finderPatterns.at(0), finderPatterns.at(2));

	if (d1to2 > d0to1 && d1to2 > d0to2)
	{
		top_left = finderPatterns.at(0);
		bottom_left = finderPatterns.at(1);
		top_right = finderPatterns.at(2);
	} else if (d0to1 > d1to2 && d0to1 > d0to2)
	{
		top_left = finderPatterns.at(2);
		bottom_left = finderPatterns.at(1);
		top_right = finderPatterns.at(0);
	}  else if (d0to2 > d1to2 && d0to2 > d0to1)
	{
		top_left = finderPatterns.at(1);
		bottom_left = finderPatterns.at(0);
		top_right = finderPatterns.at(2);
	}
	else
	{
		LOGW("QRFinder","Unable to resolve finder pattern order by distance");
		//no clear winner, probably some significant perspective. 
		if (FPAcuteAngleGeometry(finderPatterns.at(0), finderPatterns.at(1), finderPatterns.at(2)) >= 0)
		{
			top_left = finderPatterns.at(0);
			top_right = finderPatterns.at(1);
			bottom_left = finderPatterns.at(2);
		}
		else if (FPAcuteAngleGeometry(finderPatterns.at(1), finderPatterns.at(0), finderPatterns.at(2)) >= 0)
		{
			top_left = finderPatterns.at(1);
			top_right = finderPatterns.at(0);
			bottom_left = finderPatterns.at(2);
		}
		else if (FPAcuteAngleGeometry(finderPatterns.at(2), finderPatterns.at(0), finderPatterns.at(1)) >= 0)
		{
			top_left = finderPatterns.at(2);
			top_right = finderPatterns.at(0);
			bottom_left = finderPatterns.at(1);
		}
		else
		{
			LOGE("Error determining finder pattern order. This shouldn't happen.");
			top_left = finderPatterns.at(0);
			bottom_left = finderPatterns.at(1);
			top_right = finderPatterns.at(2);
		}
	}

	if (FPAcuteAngleGeometry(bottom_left, top_left, top_right) < 0)
	{
		FinderPattern * t = bottom_left;
		bottom_left = top_right;
		top_right = t;
	}

	vector<FinderPattern*> patternVector;
	patternVector.push_back(top_left);
	patternVector.push_back(top_right);
	patternVector.push_back(bottom_left);




	return new QRCode(patternVector);
}

