#include "QRDecoder.hpp"


//int QRDecoder::numRSBlocks[][] = {{ 7, 10, 13, 17 },{ 7, 10, 13, 17 }};

QRDecoder::QRDecoder()
{
	const int numRSBlock[][4] = 
	{
		{ 1, 1, 1, 1 },  { 1, 1, 1, 1 },  { 1, 1, 2, 2 }, 
		{ 1, 2, 2, 4 },  { 1, 2, 4, 4 },  { 2, 4, 4, 4 }, 
		{ 2, 4, 6, 5 },  { 2, 4, 6, 6 },  { 2, 5, 8, 8 }, 
		{ 4, 5, 8, 8 },  { 4, 5, 8, 11 },  { 4, 8, 10, 11 }, 
		{ 4, 9, 12, 16 },  { 4, 9, 16, 16 },  { 6, 10, 12, 18 },
		{ 6, 10, 17, 16 },  { 6, 11, 16, 19 },  { 6, 13, 18, 21 }, 
		{ 7, 14, 21, 25 },  { 8, 16, 20, 25 },  { 8, 17, 23, 25 }, 
		{ 9, 17, 23, 34 },  { 9, 18, 25, 30 },  { 10, 20, 27, 32 }, 
		{ 12, 21, 29, 35 },  { 12, 23, 34, 37 },  { 12, 25, 34, 40 }, 
		{ 13, 26, 35, 42 },  { 14, 28, 38, 45 },  { 15, 29, 40, 48 }, 
		{ 16, 31, 43, 51 },  { 17, 33, 45, 54 },  { 18, 35, 48, 57 }, 
		{ 19, 37, 51, 60 },  { 19, 38, 53, 63 },  { 20, 40, 56, 66 }, 
		{ 21, 43, 59, 70 },  { 22, 45, 62, 74 },  { 24, 47, 65, 77 }, 
		{ 25, 49, 68, 81 } 
	};

	const int numErrorCollectionCode[][4] = 
	{
		{ 7, 10, 13, 17 },  { 10, 16, 22, 28 },  { 15, 26, 36, 44 },  { 20, 36, 52, 64 }, 
		{ 26, 48, 72, 88 },  { 36, 64, 96, 112 },  { 40, 72, 108, 130 },  { 48, 88, 132, 156 }, 
		{ 60, 110, 160, 192 },  { 72, 130, 192, 224 },  { 80, 150, 224, 264 },  { 96, 176, 260, 308 }, 
		{ 104, 198, 288, 352 },  { 120, 216, 320, 384 },  { 132, 240, 360, 432 },  { 144, 280, 408, 480 }, 
		{ 168, 308, 448, 532 },  { 180, 338, 504, 588 },  { 196, 364, 546, 650 },  { 224, 416, 600, 700 }, 
		{ 224, 442, 644, 750 },  { 252, 476, 690, 816 },  { 270, 504, 750, 900 },  { 300, 560, 810, 960 }, 
		{ 312, 588, 870, 1050 },  { 336, 644, 952, 1110 },  { 360, 700, 1020, 1200 },  { 390, 728, 1050, 1260 }, 
		{ 420, 784, 1140, 1350 },  { 450, 812, 1200, 1440 },  { 480, 868, 1290, 1530 },  { 510, 924, 1350, 1620 }, 
		{ 540, 980, 1440, 1710 },  { 570, 1036, 1530, 1800 },  { 570, 1064, 1590, 1890 },  { 600, 1120, 1680, 1980 }, 
		{ 630, 1204, 1770, 2100 },  { 660, 1260, 1860, 2220 },  { 720, 1316, 1950, 2310 },  { 750, 1372, 2040, 2430 } 
	};
}

void QRDecoder::DecodeQRCode(Mat & binaryImage, QRCode * qrCode)
{
	float moduleSize = 0;
	for (int i=0;i<qrCode->finderPatterns->size();i++)
	{
		moduleSize += qrCode->finderPatterns->at(i)->size;
	}
	moduleSize /= 21.0f;

	float numModulesPerSide = 29;
	Point2i topLeft = Point2i((moduleSize*7)/2.0f,(moduleSize*7)/2.0f);
	Point2i topRight = Point2i(topLeft.x + (numModulesPerSide-7)*moduleSize, topLeft.y);
	Point2i bottomRight_Alignment = Point2i((int)round((numModulesPerSide-7.5f)*moduleSize), 
		(int)round((numModulesPerSide-7.5f)*moduleSize));

	Point2i bottomLeft = Point2i(topLeft.x, topLeft.y + (numModulesPerSide-7)*moduleSize);

	Point2i imageTopLeft = qrCode->finderPatterns->at(0)->pt;
	Point2i imageTopRight = qrCode->finderPatterns->at(1)->pt;
	Point2i imageBottomRight_Alignment = qrCode->alignmentPattern;
	Point2i imageBottomLeft = qrCode->finderPatterns->at(2)->pt;
	
	PerspectiveTransform pt = PerspectiveTransform::QuadrilateralToQuadrilateral
		(topLeft,topRight,bottomRight_Alignment,bottomLeft,
		imageTopLeft,imageTopRight,imageBottomRight_Alignment,imageBottomLeft);

	

}


bool QRDecoder::getElement(int x, int y)
{
	return (elements[y])[x];
}

void QRDecoder::GetBlocks(int *& words)
{
	int x = width - 1;
	int y = height - 1;
	vector<bool> codeBits;
	vector<int32_t> codeWords;
	int tempWord = 0;
	int figure = 7;
	int isNearFinish = 0;
	bool READ_UP = true;
	bool READ_DOWN = false;
	bool direction = READ_UP;
	do 
	{
		codeBits.push_back(getElement(x, y));
		if (getElement(x, y) == true)
		{
			tempWord += (1 << figure);
		}
		figure--;
		if (figure == - 1)
		{
			codeWords.push_back((int32_t)tempWord);
			figure = 7;
			tempWord = 0;
		}
		// determine module that read next
		do 
		{
			if (direction == READ_UP)
			{
				if ((x + isNearFinish) % 2 == 0)
					//if right side of two column
					x--;
				// to left
				else
				{
					if (y > 0)
					{
						//be able to move upper side
						x++;
						y--;
					}
					else
					{
						//can't move upper side
						x--; //change direction
						if (x == 6)
						{
							x--;
							isNearFinish = 1; // after through horizontal Timing Pattern, move pattern is changed
						}
						direction = READ_DOWN;
					}
				}
			}
			else
			{
				if ((x + isNearFinish) % 2 == 0)
					//if left side of two column
					x--;
				else
				{
					if (y < height - 1)
					{
						x++;
						y++;
					}
					else
					{
						x--;
						if (x == 6)
						{
							x--;
							isNearFinish = 1;
						}
						direction = READ_UP;
					}
				}
			}
		}
		while (isInFunctionPattern(x, y));
	}
	while (x != - 1);

	words = new int[codeWords.size()];
	for (int i = 0; i < codeWords.size(); i++)
	{
		int32_t temp = (int32_t) codeWords[i];
		words[i] = temp;
	}
	return;
}

bool  QRDecoder::isInFunctionPattern(int targetX, int targetY)
{
	if (targetX < 9 && targetY < 9)
		//in Left-Up Finder Pattern or function patterns around it
		return true;
	if (targetX > Width - 9 && targetY < 9)
		//in Right-up Finder Pattern or function patterns around it
		return true;
	if (targetX < 9 && targetY > Height - 9)
		//in Left-bottom Finder Pattern or function patterns around it
		return true;

	if (version >= 7)
	{
		if (targetX > Width - 12 && targetY < 6)
			return true;
		if (targetX < 6 && targetY > Height - 12)
			return true;
	}
	// in timing pattern
	if (targetX == 6 || targetY == 6)
		return true;

	// in alignment pattern. 		
	Point[][] alignmentPattern = AlignmentPattern;
	int sideLength = alignmentPattern.Length;

	for (int y = 0; y < sideLength; y++)
	{
		for (int x = 0; x < sideLength; x++)
		{
			if (!(x == 0 && y == 0) && !(x == sideLength - 1 && y == 0) && !(x == 0 && y == sideLength - 1))
				if (System.Math.Abs(alignmentPattern[x][y].X - targetX) < 3 && System.Math.Abs(alignmentPattern[x][y].Y - targetY) < 3)
					return true;
		}
	}			
	return false;
}

void  QRDecoder::decodeFormatInformation(bool * formatInformation)
{
	if (formatInformation[4] == false)
		if (formatInformation[3] == true)
			errorCollectionLevel = 0;
		else
			errorCollectionLevel = 1;
	else if (formatInformation[3] == true)
		errorCollectionLevel = 2;
	else
		errorCollectionLevel = 3;

	for (int i = 2; i >= 0; i--)
		if (formatInformation[i] == true)
			maskPattern += (1 << i);
}

void  QRDecoder::initialize()
{
	//calculate version by number of side modules
	version = (width - 17) / 4;
	//Point2i ** alignmentPattern = new Point2i;

	vector<vector<Point2i*>*> alignmentPattern;

	for (int i = 0; i < 1; i++)
	{
		alignmentPattern.at(i)->push_back(new Point2i());
	}

	int * logicalSeeds = new int[1];
	if (version >= 2 && version <= 40)
	{
		logicalSeeds = getSeed(version);
		Point[][] tmpArray = new Point[logicalSeeds.Length][];
		for (int i2 = 0; i2 < logicalSeeds.Length; i2++)
		{
			tmpArray[i2] = new Point[logicalSeeds.Length];
		}
		alignmentPattern = tmpArray;
	}

	//obtain alignment pattern's center coodintates by logical seeds
	for (int col = 0; col < logicalSeeds.Length; col++)
	{
		for (int row = 0; row < logicalSeeds.Length; row++)
		{
			alignmentPattern[row][col] = new Point(logicalSeeds[row], logicalSeeds[col]);
		}
	}
	this.alignmentPattern = alignmentPattern;			
	dataCapacity = calcDataCapacity();			
	bool[] formatInformation = readFormatInformation();
	decodeFormatInformation(formatInformation);			
	unmask();
}

int * QRDecoder::getSeed(int version)
{
	//int * seed[40] ;//= new int*[40];

				//int seed0[] = {6, 14};

				/*seed[1] = new int[]{6, 18};
				seed[2] = new int[]{6, 22};
				seed[3] = new int[]{6, 26};
				seed[4] = new int[]{6, 30};
				seed[5] = new int[]{6, 34};
				seed[6] = new int[]{6, 22, 38};
				seed[7] = new int[]{6, 24, 42};
				seed[8] = new int[]{6, 26, 46};
				seed[9] = new int[]{6, 28, 50};
				seed[10] = new int[]{6, 30, 54};
				seed[11] = new int[]{6, 32, 58};
				seed[12] = new int[]{6, 34, 62};
				seed[13] = new int[]{6, 26, 46, 66};
				seed[14] = new int[]{6, 26, 48, 70};
				seed[15] = new int[]{6, 26, 50, 74};
				seed[16] = new int[]{6, 30, 54, 78};
				seed[17] = new int[]{6, 30, 56, 82};
				seed[18] = new int[]{6, 30, 58, 86};
				seed[19] = new int[]{6, 34, 62, 90};
				seed[20] = new int[]{6, 28, 50, 72, 94};
				seed[21] = new int[]{6, 26, 50, 74, 98};
				seed[22] = new int[]{6, 30, 54, 78, 102};
				seed[23] = new int[]{6, 28, 54, 80, 106};
				seed[24] = new int[]{6, 32, 58, 84, 110};
				seed[25] = new int[]{6, 30, 58, 86, 114};
				seed[26] = new int[]{6, 34, 62, 90, 118};
				seed[27] = new int[]{6, 26, 50, 74, 98, 122};
				seed[28] = new int[]{6, 30, 54, 78, 102, 126};
				seed[29] = new int[]{6, 26, 52, 78, 104, 130};
				seed[30] = new int[]{6, 30, 56, 82, 108, 134};
				seed[31] = new int[]{6, 34, 60, 86, 112, 138};
				seed[32] = new int[]{6, 30, 58, 86, 114, 142};
				seed[33] = new int[]{6, 34, 62, 90, 118, 146};
				seed[34] = new int[]{6, 30, 54, 78, 102, 126, 150};
				seed[35] = new int[]{6, 24, 50, 76, 102, 128, 154};
				seed[36] = new int[]{6, 28, 54, 80, 106, 132, 158};
				seed[37] = new int[]{6, 32, 58, 84, 110, 136, 162};
				seed[38] = new int[]{6, 26, 54, 82, 110, 138, 166};
				seed[39] = new int[]{6, 30, 58, 86, 114, 142, 170};*/

	return 0;// (seed[version - 1]);
}

/// <summary> Returns a seed for a version and a pattern number</summary>
int QRDecoder::getSeed(int version, int patternNumber)
{
	return (getSeed(version)[patternNumber]);
}
		