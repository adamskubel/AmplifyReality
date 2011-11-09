/* File: FindPattern.h; Mode: C++; Tab-width: 3; Author: Simon Flannery;      */

#ifndef FINDPATTERN_H
#define FINDPATTERN_H

#include <vector>
using std::vector;

#define DEFAULT_HIT_CONFIDENCE 4

struct POINT
{
   long x;
   long y;
};

struct FINDPATTERN
{
   POINT pt;
   long size;
   long hitCount;

   static long Distance(const FINDPATTERN& a, const FINDPATTERN& b);
   static long AcuteAngleGeometry(const FINDPATTERN& a, const FINDPATTERN& b, const FINDPATTERN& c);
};

bool operator<(const FINDPATTERN& fp1, const FINDPATTERN& fp2);

class FINDPATTERN_vector : public vector<FINDPATTERN>
{
public:
/* Use this push back member function instead of the push back function. */
   bool push_back_pattern(const FINDPATTERN& fp);

/* Adding sorting capabilites to the standard vector. */
   FINDPATTERN_vector& Sort();

   long HitConfidence(long c = DEFAULT_HIT_CONFIDENCE) const;
};

#endif
