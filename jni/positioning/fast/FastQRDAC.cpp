////#include "FastQRFinder.hpp"
////
////void FastQRFinder::CheckDistanceDACPane(vector<Point2i> & points, vector<Point2i> & closestPoints)
////{
////	int distance;
////
////	vector<Point2i> sortedPoints;
////
////	// Copy current collection
////	for (int i = 0; i < points.size(); i++)
////	{
////		sortedPoints.push_back(points[i]);
////	}
////
////	// Used for comparing X values of points
////	struct compareXStructPane
////	{
////		bool operator ()(Point2i firstDot, Point2i secondDot)
////		{
////			Point2i fXY = firstDot;
////			Point2i sXY = secondDot;
////
////			if (fXY.x < sXY.x)
////			{
////				return true;
////			} else
////			{
////				return false;
////			}
////		}
////	};
////
////	// Sort the points using the X values
////	sort(sortedPoints.begin(), sortedPoints.end(), compareXStructPane());
////
////	if (sortedPoints.size() >= 2)
////	{
////		vector <Point2i> closestPoints = findClosest(sortedPoints);
////	}
////}
////
////vector<Point2i> FastQRFinder::CheckValidDistancePane(vector<Point2i> points)
////{
////	int distance;
////
////	vector<Point2i> validPoints;
////	vector<Point2i> sortedPoints;
////
////	// Copy current collection
////	for (int i = 0; i < points.size(); i++)
////	{
////		sortedPoints.push_back(points[i]);
////	}
////
////	// Used for comparing X values of points
////	struct compareXStructPane
////	{
////		bool operator ()(Point2i firstDot, Point2i secondDot)
////		{
////			Point2i fXY = firstDot;
////			Point2i sXY = secondDot;
////
////			if (fXY.x < sXY.x)
////			{
////				return true;
////			} else
////			{
////				return false;
////			}
////		}
////	};
////
////	// Sort the points using the X values
////	sort(sortedPoints.begin(), sortedPoints.end(), compareXStructPane());
////
////	if (sortedPoints.size() >= 2)
////	{
////		vector<Point2i> closestPoints = findClosest(sortedPoints);
////		return closestPoints;
////	} else
////	{
////		return validPoints;
////	}
////}
////
////vector<Point2i> FastQRFinder::findClosest(vector<Point2i> & px)
////{
////	vector<Point2i> tempPoints;
////
////	int n = px.size();
////
////	if (n <= 3)
////	{
////		return shortest(px);
////	} else
////	{
////		int left = n / 2; // left side
////		int right = n / 2 + n % 2; // right side
////
////		vector<Point2i> Pleft;
////		vector<Point2i> Pright;
////		vector<Point2i> Pleftmin;
////		vector<Point2i> Prightmin;
////		vector<Point2i> Pclosest;
////
////		for (int i = 0; i < left; i++)
////		{
////			Pleft.push_back(px[i]);
////		}
////
////		for (int i = 0; i < right; i++)
////		{
////			Pright.push_back(px[i + left]);
////		}
////
////		Pleftmin = findClosest(Pleft); // closest points on the right
////		Prightmin = findClosest(Pright); // closest points on the left
////		Pclosest = mergePlanes(Pleftmin, Prightmin);
////
////		return Pclosest;
////	}
////
////}
////
////vector<Point2i> FastQRFinder::mergePlanes(vector<Point2i> p1, vector<Point2i> p2)
////{
////	vector<Point2i> pMin;
////
////	vector<Point2i> pAll;
////
////	for (int i = 0; i < p1.size(); i++)
////	{
////		pAll.push_back(p1[i]);
////	}
////
////	for (int i = 0; i < p2.size(); i++)
////	{
////		pAll.push_back(p2[i]);
////	}
////
////	vector<Point2i> closest;
////
////	closest = shortest(pAll);
////
////	int D = GetDistanceFast(closest[0], closest[1]);
////
////	for (int i = 0; i < p1.size(); i++)
////	{
////		for (int j = 0; j < p2.size(); j++)
////		{
////			Point2i pi = p1[i];
////			Point2i pj = p2[j];
////
////			if (pi == pj)
////				continue;
////
////			Point2i p1XY = p1[i];
////			Point2i p2XY = p2[j];
////
////			int xi = p1XY.x;
////			int xj = p2XY.x;
////			int yi = p1XY.y;
////			int yj = p2XY.x;
////
////			if (xi < xj + D && yi + D > yj && yj > yi - D)
////			{
////				if (GetDistanceFast(pi, pj) < D)
////				{
////					vector<Point2i> distVector;
////					distVector.push_back(pi);
////					distVector.push_back(pj);
////
////					return distVector;
////				}
////			}
////		}
////	}
////
////	pMin.push_back(closest[0]);
////	pMin.push_back(closest[1]);
////
////	return pMin;
////}
////
////vector<Point2i> FastQRFinder::shortest(vector<Point2i> ps)
////{
////	vector<Point2i> shortestVector;
////
////	Point2i p1;
////	Point2i p2;
////
////	int distance = 8192;
////
////	for (int i = 0; i < ps.size(); i++)
////	{
////		for (int j = 0; j < i; j++)
////		{
////			Point2i ptemp1;
////			Point2i ptemp2;
////
////			ptemp1 = ps[i];
////			ptemp2 = ps[j];
////
////			int newDistance = GetDistanceFast(ptemp1, ptemp2);
////
////			if (newDistance < distance)
////			{
////				distance = newDistance;
////				p1 = ptemp1;
////				p2 = ptemp2;
////			}
////		}
////	}
////
////	shortestVector.push_back(p1);
////	shortestVector.push_back(p2);
////
////	return shortestVector;
////}
