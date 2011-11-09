/* File: PerspectiveTransfrom.h; Mode: C++; Tab-width: 3; Author: Simon Flannery; */

#ifndef PT_H
#define PT_H

#include "FindPattern.h"

class PerspectiveTransform
{
public:
   PerspectiveTransform(long a11 = 0, long a21 = 0, long a31 = 0,
                        long a12 = 0, long a22 = 0, long a32 = 0,
                        long a13 = 0, long a23 = 0, long a33 = 0);

   bool TransformPoint(POINT& pt) const;

   static PerspectiveTransform QuadrilateralToQuadrilateral(POINT x0, POINT x1,
                                                            POINT x2, POINT x3,
                                                            POINT x0p, POINT x1p,
                                                            POINT x2p, POINT x3p);

   static PerspectiveTransform SquareToQuadrilateral(POINT a, POINT b,
                                                     POINT c, POINT d);

   static PerspectiveTransform QuadrilateralToSquare(POINT a, POINT b,
                                                     POINT c, POINT d);

private:
   PerspectiveTransform BuildAdjoint();

   friend PerspectiveTransform operator*(const PerspectiveTransform& a, const PerspectiveTransform& b);

   long m11, m21, m31, m12, m22, m32, m13, m23, m33;
};

#endif
