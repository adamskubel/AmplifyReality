/* File: FindPattern.h; Mode: C++; Tab-width: 3; Author: Simon Flannery;      */


#ifndef FinderPattern_H
#define FinderPattern_H


#include <opencv2/core/core.hpp>
#include <vector>
#include "LogDefinitions.h"

using std::vector;

struct FinderPattern
{
   cv::Point2i pt;
   long size;
   static int instanceCount;
  
   static long Distance(FinderPattern * a, FinderPattern * b);
   static long AcuteAngleGeometry(FinderPattern* a, FinderPattern* b, FinderPattern* c);

   FinderPattern()
   {
	   FinderPattern::instanceCount++;
	   pt = cv::Point2i(0,0);
	   size = 1;	  
   }

   FinderPattern(cv::Point2i _position, int _size)
   {
	   FinderPattern::instanceCount++;
	   pt = _position;
	   size = _size;
   }


   ~FinderPattern()
   {	   
	   FinderPattern::instanceCount--;
   }

   FinderPattern(FinderPattern & copy)
   {
	   FinderPattern::instanceCount++;
	   pt = copy.pt;
	   size = copy.size;
   }

};

#endif
