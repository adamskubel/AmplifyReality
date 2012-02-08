/* File: FindPattern.h; Mode: C++; Tab-width: 3; Author: Simon Flannery;      */


#ifndef FinderPattern_H
#define FinderPattern_H


#include <opencv2/core/core.hpp>
#include <vector>
#include "LogDefinitions.h"

using std::vector;

#define DEFAULT_HIT_CONFIDENCE 4



struct FinderPattern
{
   cv::Point2i pt;
   long size;
   long hitCount;
   int * patternWidths;
   static int instanceCount;
  
   static long Distance(FinderPattern * a, FinderPattern * b);
   static long AcuteAngleGeometry(FinderPattern* a, FinderPattern* b, FinderPattern* c);

   FinderPattern()
   {
	   FinderPattern::instanceCount++;
	   pt = cv::Point2i(0,0);
	   size = 1;
	   hitCount = 0;
	   patternWidths = new int[5];
	   LOGD(LOGTAG_QR,"Init FP");
   }

   ~FinderPattern()
   {
	   if (patternWidths != NULL)
	   {
		   LOGD(LOGTAG_QR,"Deleting int[5]");
		   delete[] patternWidths;
	   }
	   FinderPattern::instanceCount--;
   }

   FinderPattern(FinderPattern & copy)
   {
	   FinderPattern::instanceCount++;
	   pt = copy.pt;
	   size = copy.size;
	   hitCount = copy.hitCount;

	   patternWidths = new int[5];
	   for (int i=0;i<5;i++)
	   {
		   patternWidths[i] = copy.patternWidths[i];
	   }
	   LOGD(LOGTAG_QR,"Copy FP");
   }

};

bool operator<(const FinderPattern& fp1, const FinderPattern& fp2);

class FinderPattern_vector : public vector<FinderPattern*>
{
public:
/* Use this push back member function instead of the push back function. */
   bool push_back_pattern(FinderPattern * fp);

/* Adding sorting capabilites to the standard vector. */
   FinderPattern_vector& Sort();

   long HitConfidence(long c = DEFAULT_HIT_CONFIDENCE) const;
};

#endif
