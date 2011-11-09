/* File: FindPattern.cpp; Mode: C++; Tab-width: 3; Author: Simon Flannery;    */

#include <cmath>
#include <algorithm>
#include "FindPattern.h"

long FINDPATTERN::Distance(const FINDPATTERN& a, const FINDPATTERN& b)
{
   long xd = a.pt.x - b.pt.x;
   long yd = a.pt.y - b.pt.y;

/* Currently, this is the only floating-point code in this project. */
   return (long) sqrt((double) (xd * xd + yd * yd));
}

/* http://en.wikipedia.org/wiki/Cross_product */
long FINDPATTERN::AcuteAngleGeometry(const FINDPATTERN& a, const FINDPATTERN& b, const FINDPATTERN& c)
{
   return (b.pt.x - a.pt.x) * (c.pt.y - a.pt.y) - (b.pt.y - a.pt.y) * (c.pt.x - a.pt.x);
}

bool operator<(const FINDPATTERN& fp1, const FINDPATTERN& fp2)
{
   return fp1.hitCount < fp2.hitCount;
}

bool FINDPATTERN_vector::push_back_pattern(const FINDPATTERN& fp)
{
   bool found = false;
   for (vector<FINDPATTERN>::iterator i = begin(); i != end(); ++i)
   {
      FINDPATTERN tMath = *i;
      tMath.size = (tMath.size << 8) / 7;
      tMath.pt.x = tMath.pt.x << 8;
      tMath.pt.y = tMath.pt.y << 8;

      FINDPATTERN fpMath = fp;
      fpMath.size = (fpMath.size << 8) / 7;
      fpMath.pt.x = fpMath.pt.x << 8;
      fpMath.pt.y = fpMath.pt.y << 8;

      if (abs(fpMath.pt.x - tMath.pt.x) < tMath.size &&
          abs(fpMath.pt.y - tMath.pt.y) < tMath.size &&
          abs(fpMath.size - tMath.size) < (tMath.size >> 2))
      {
         FINDPATTERN& t = *i;
         t.pt.x = (t.pt.x + fp.pt.x) / 2;
         t.pt.y = (t.pt.y + fp.pt.y) / 2;
         t.size = (t.size + fp.size) / 2;
         ++t.hitCount;

         found = true;
         break;
      }
   }

   if (found == false)
   {
      push_back(fp);
   }

   return found;
}

FINDPATTERN_vector& FINDPATTERN_vector::Sort()
{
   for (vector<FINDPATTERN>::iterator j = begin(); j != end(); ++j)
   {
      vector<FINDPATTERN>::iterator min_index = j;
      for (vector<FINDPATTERN>::iterator i = begin(); i != end(); ++i)
      {
         if (*i < *min_index) { min_index = i; }
      }

      if (j != min_index) { iter_swap(j, min_index); }
   }

   return *this;
}

long FINDPATTERN_vector::HitConfidence(long c) const
{
   long hits = 0;
   for (vector<FINDPATTERN>::const_iterator i = begin(); i != end(); ++i)
   {
      if (i->hitCount > c) { ++hits; }
   }

   return hits;
}
