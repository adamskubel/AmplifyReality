/* File: FindPattern.cpp; Mode: C++; Tab-width: 3; Author: Simon Flannery;    */

#include <cmath>
#include <algorithm>
#include "FindPattern.h"
#include "LogDefinitions.h"

long FinderPattern::Distance(FinderPattern * a,  FinderPattern * b)
{
   long xd = a->pt.x - b->pt.x;
   long yd = a->pt.y - b->pt.y;

/* Currently, this is the only floating-point code in this project. */
   return (long) sqrt((double) (xd * xd + yd * yd));
}

/* http://en.wikipedia.org/wiki/Cross_product */
long FinderPattern::AcuteAngleGeometry(FinderPattern* a, FinderPattern* b, FinderPattern* c)
{
   return (b->pt.x - a->pt.x) * (c->pt.y - a->pt.y) - (b->pt.y - a->pt.y) * (c->pt.x - a->pt.x);
}

bool operator<(const FinderPattern& fp1, const FinderPattern& fp2)
{
   return fp1.hitCount < fp2.hitCount;
}

bool FinderPattern_vector::push_back_pattern(FinderPattern * fp)
{
   bool found = false;
   for (vector<FinderPattern*>::iterator i = begin(); i != end(); ++i)
   {
      FinderPattern * tMath = new FinderPattern(**i);
      tMath->size = (tMath->size << 8) / 7;
      tMath->pt.x = tMath->pt.x << 8;
      tMath->pt.y = tMath->pt.y << 8;

      FinderPattern * fpMath = new FinderPattern(*fp);
      fpMath->size = (fpMath->size << 8) / 7;
      fpMath->pt.x = fpMath->pt.x << 8;
      fpMath->pt.y = fpMath->pt.y << 8;

      if (abs(fpMath->pt.x - tMath->pt.x) < tMath->size &&
          abs(fpMath->pt.y - tMath->pt.y) < tMath->size &&
          abs(fpMath->size - tMath->size) < (tMath->size >> 2))
      {
         FinderPattern * t = *i;
         t->pt.x = (t->pt.x + fp->pt.x) / 2;
         t->pt.y = (t->pt.y + fp->pt.y) / 2;
         t->size = (t->size + fp->size) / 2;
         ++(t->hitCount);

         found = true;
		 delete tMath;
		 delete fpMath;
         break;
      }

	  delete tMath;
	  delete fpMath;
   }

   if (found == false)
   {
      push_back(fp);
   }
   
   return found;
}

FinderPattern_vector& FinderPattern_vector::Sort()
{
   for (vector<FinderPattern*>::iterator j = begin(); j != end(); ++j)
   {
      vector<FinderPattern*>::iterator min_index = j;
      for (vector<FinderPattern*>::iterator i = begin(); i != end(); ++i)
      {
         if (*i < *min_index) { min_index = i; }
      }

      if (j != min_index) { iter_swap(j, min_index); }
   }

   return *this;
}

long FinderPattern_vector::HitConfidence(long c) const
{
   long hits = 0;
   for (vector<FinderPattern*>::const_iterator i = begin(); i != end(); ++i)
   {
      if ((*i)->hitCount > c) { ++hits; }
   }

   return hits;
}
