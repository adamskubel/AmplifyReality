/* File: PerspectiveTransfrom.h; Mode: C++; Tab-width: 3; Author: Simon Flannery; */

#ifndef PT_H
#define PT_H

#include "FindPattern.h"
#include <opencv2/core/core.hpp>

using namespace cv;

class PerspectiveTransform
{
public:
   PerspectiveTransform(long a11 = 0, long a21 = 0, long a31 = 0,
                        long a12 = 0, long a22 = 0, long a32 = 0,
                        long a13 = 0, long a23 = 0, long a33 = 0);

   bool TransformPoint(Point2f& pt) const;

   static PerspectiveTransform QuadrilateralToQuadrilateral(Point2i x0, Point2i x1,
                                                            Point2i x2, Point2i x3,
                                                            Point2i x0p, Point2i x1p,
                                                            Point2i x2p, Point2i x3p);

   static PerspectiveTransform SquareToQuadrilateral(Point2i a, Point2i b,
                                                     Point2i c, Point2i d);

   static PerspectiveTransform QuadrilateralToSquare(Point2i a, Point2i b,
                                                     Point2i c, Point2i d);

private:
   PerspectiveTransform BuildAdjoint();

   friend PerspectiveTransform operator*(const PerspectiveTransform& a, const PerspectiveTransform& b);

   long m11, m21, m31, m12, m22, m32, m13, m23, m33;
};

#endif
