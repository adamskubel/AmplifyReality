/* File: PerspectiveTransfrom.cpp; Mode: C++; Tab-width: 3; Author: Simon Flannery; */

#include "PerspectiveTransform.h"

PerspectiveTransform::PerspectiveTransform(long a11, long a21, long a31,
                                           long a12, long a22, long a32,
                                           long a13, long a23, long a33) : 
m11(a11), m21(a21), m31(a31), m12(a12), m22(a22), m32(a32), m13(a13), m23(a23), m33(a33)
{
}

bool PerspectiveTransform::TransformPoint(Point2i& pt) const
{
   long x = pt.x;
   long y = pt.y;
   long denominator = m13 * x + m23 * y + m33;

   if (denominator != 0)
   {
      pt.x = (m11 * x + m21 * y + m31) / denominator;
      pt.y = (m12 * x + m22 * y + m32) / denominator;
   }

   return denominator != 0;
}

PerspectiveTransform PerspectiveTransform::QuadrilateralToQuadrilateral(Point2i pt0, Point2i pt1,
                                                                        Point2i pt2, Point2i pt3,
                                                                        Point2i pt0p, Point2i pt1p,
                                                                        Point2i pt2p, Point2i pt3p)
{
   PerspectiveTransform quadToSqr = PerspectiveTransform::QuadrilateralToSquare(pt0, pt1, pt2, pt3);
   PerspectiveTransform sqrToQuad = PerspectiveTransform::SquareToQuadrilateral(pt0p, pt1p, pt2p, pt3p);

   PerspectiveTransform p = sqrToQuad * quadToSqr;

   return p;
}

PerspectiveTransform PerspectiveTransform::SquareToQuadrilateral(Point2i a, Point2i b,
                                                                 Point2i c, Point2i d)
{
   PerspectiveTransform pt;

   long dy2 = d.y - c.y;
   long dy3 = a.y - b.y + c.y - d.y;

   if (dy2 == 0 && dy3 == 0)
   {
      pt = PerspectiveTransform(b.x - a.x, c.x - b.x, a.x,
                                b.y - a.y, c.y - b.y, a.y,
                                0, 0, 1);
   }
   else
   {
      long dx1 = b.x - c.x;
      long dx2 = d.x - c.x;
      long dx3 = a.x - b.x + c.x - d.x;
      long dy1 = b.y - c.y;
      long denominator = dx1 * dy2 - dx2 * dy1;
      long a13 = (dx3 * dy2 - dx2 * dy3) / denominator;
      long a23 = (dx1 * dy3 - dx3 * dy1) / denominator;

      pt = PerspectiveTransform(b.x - a.x + a13 * b.x, d.x - a.x + a23 * d.x, a.x,
                                b.y - a.y + a13 * b.y, d.y - a.y + a23 * d.y, a.y,
                                a13, a23, 1);
   }

   return pt;
}

PerspectiveTransform PerspectiveTransform::QuadrilateralToSquare(Point2i a, Point2i b,
                                                                 Point2i c, Point2i d)
{
   PerspectiveTransform p = SquareToQuadrilateral(a, b, c, d);

   return p.BuildAdjoint();
}

PerspectiveTransform PerspectiveTransform::BuildAdjoint()
{
   return PerspectiveTransform(m22 * m33 - m23 * m32,
                               m23 * m31 - m21 * m33,
                               m21 * m32 - m22 * m31,
                               m13 * m32 - m12 * m33,
                               m11 * m33 - m13 * m31,
                               m12 * m31 - m11 * m32,
                               m12 * m23 - m13 * m22,
                               m13 * m21 - m11 * m23,
                               m11 * m22 - m12 * m21);
}

PerspectiveTransform operator*(const PerspectiveTransform& a, const PerspectiveTransform& b)
{
   return PerspectiveTransform(a.m11 * b.m11 + a.m21 * b.m12 + a.m31 * b.m13,
                               a.m11 * b.m21 + a.m21 * b.m22 + a.m31 * b.m23,
                               a.m11 * b.m31 + a.m21 * b.m32 + a.m31 * b.m33,
                               a.m12 * b.m11 + a.m22 * b.m12 + a.m32 * b.m13,
                               a.m12 * b.m21 + a.m22 * b.m22 + a.m32 * b.m23,
                               a.m12 * b.m31 + a.m22 * b.m32 + a.m32 * b.m33,
                               a.m13 * b.m11 + a.m23 * b.m12 + a.m33 * b.m13,
                               a.m13 * b.m21 + a.m23 * b.m22 + a.m33 * b.m23,
                               a.m13 * b.m31 + a.m23 * b.m32 + a.m33 * b.m33);
}
