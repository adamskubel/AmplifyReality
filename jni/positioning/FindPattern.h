/* File: FindPattern.h; Mode: C++; Tab-width: 3; Author: Simon Flannery;      */

#include <opencv2/core/core.hpp>

#ifndef FinderPattern_H
#define FinderPattern_H

#include <vector>
using std::vector;

#define DEFAULT_HIT_CONFIDENCE 4


struct FinderPattern
{
   cv::Point2i pt;
   long size;
   long hitCount;

   static long Distance(const FinderPattern& a, const FinderPattern& b);
   static long AcuteAngleGeometry(const FinderPattern& a, const FinderPattern& b, const FinderPattern& c);
};

bool operator<(const FinderPattern& fp1, const FinderPattern& fp2);

class FinderPattern_vector : public vector<FinderPattern>
{
public:
/* Use this push back member function instead of the push back function. */
   bool push_back_pattern(const FinderPattern& fp);

/* Adding sorting capabilites to the standard vector. */
   FinderPattern_vector& Sort();

   long HitConfidence(long c = DEFAULT_HIT_CONFIDENCE) const;
};

#endif
