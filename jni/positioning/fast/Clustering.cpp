#include "FastQRFinder.hpp"



bool FastQRFinder::GetNodesInRadius(FastQR::Node* pt, double dblRadius, int nMinPts, int maxPts, vector<FastQR::Node*>& rgpNodesFound, flann::Index * kdIndex, vector<FastQR::Node*> & vecNodes) 
{ 

	int maxSize = MIN(maxPts,vecNodes.size());
	Mat indexMat = Mat(1,maxSize,CV_32S);
	Mat distMat = Mat(1,maxSize,CV_32F);
	int result = kdIndex->radiusSearch(pt->nodePoint,indexMat,distMat,dblRadius,maxSize,flann::SearchParams(32));

	//kdIndex->knnSearch(pt->nodePoint,indexMat,distMat,maxSize,flann::SearchParams(32));

	int * indexPtr = indexMat.ptr<int>(0);
	//float * distPtr = distMat.ptr<float>(0);
	for (int i=0;i < MIN(maxSize,result);i++)
	{		
		/*if (distPtr[i] < dblRadius)
		{
			result++;*/
			rgpNodesFound.push_back(vecNodes[indexPtr[i]]);
		//}
	}	
 	return true;//result > nMinPts;
} 


//void FastQRFinder::ExpandCluster(vector<FastQR::Node*>& rgp, int nCluster, double dblEpsilon, int nMinPts, int maxPts, flann::Index * kdIndex, vector<FastQR::Node*> & vecNodes) 
//{ 
//	vector<FastQR::Node*> rgpNeighbourhood; 
//	for (int i = 0; i < (int)rgp.size(); i++) 
//	{
//		if (!rgp[i]->visited) 
//		{ 
//			rgp[i]->clusterIndex = nCluster; 
//			rgp[i]->visited = true; 
//			rgpNeighbourhood.push_back(rgp[i]); 
//		} 
//
//		for (int i = 0; i < (int)rgpNeighbourhood.size(); i++) 
//		{ 
//			FastQR::Node* pNodeNear = rgpNeighbourhood[i]; 
//			vector<FastQR::Node*> rgpNeighbourhood2; 
//			if (GetNodesInRadius(pNodeNear, dblEpsilon, nMinPts, maxPts, rgpNeighbourhood2, kdIndex, vecNodes))
//			{ 
//				// append to rgpNeighbourhood items in rgpNeighbourhood2 that aren't already in rgpNeighbourhood 
//				for (int j = 0; j < (int)rgpNeighbourhood2.size(); j++) 
//				{ 
//					FastQR::Node* pNode = rgpNeighbourhood2[j]; 
//					if (!pNode->visited) 
//						pNode->visited = true; 
//					if (pNode->clusterIndex < 0) 
//					{ 
//						pNode->clusterIndex = nCluster; 
//						rgpNeighbourhood.push_back(pNode); 
//					} 
//				} 
//			} 
//		} 
//	}
//} 

void FastQRFinder::ExpandCluster_Recursive(FastQR::Node* node, int nCluster, double dblEpsilon, int nMinPts, int maxPts, flann::Index * kdIndex, vector<FastQR::Node*> & vecNodes) 
{ 
	if (!node->visited) 
	{ 
		node->clusterIndex = nCluster; 
		node->visited = true; 

		vector<FastQR::Node*> localNodes; 
		GetNodesInRadius(node, dblEpsilon, nMinPts, maxPts, localNodes, kdIndex, vecNodes);

		for (int j = 0; j < (int)localNodes.size(); j++) 
		{ 
			ExpandCluster_Recursive(localNodes[j],nCluster,dblEpsilon,nMinPts,maxPts,kdIndex,vecNodes);
			//if (!pNode->visited) 
			//{	
			//	pNode->visited = true; 
			//	if (pNode->clusterIndex < 0) 
			//		pNode->clusterIndex = nCluster;	
			//}
		} 
	}
} 

int FastQRFinder::RunDBScan(vector<FastQR::Node*> & vecNodes, flann::Index * kdIndex, double flannRadius, int nMinPts, int maxPts) 
{ 
	int nCluster = 0;

	for (vector<FastQR::Node*>::const_iterator it = vecNodes.begin(); it != vecNodes.end(); it++) 
	{ 
		FastQR::Node* pNode = *it; 
		if (!pNode->visited) 
		{ 
			pNode->visited = true; 

			vector<FastQR::Node*> rgpNeighbourhood; 
			if (GetNodesInRadius(pNode, flannRadius, nMinPts, maxPts, rgpNeighbourhood,kdIndex, vecNodes)) 
			{ 
				pNode->clusterIndex = nCluster; 
				pNode->visited = true; 
				for (int i=0;i<rgpNeighbourhood.size();i++)
				{
					ExpandCluster_Recursive(rgpNeighbourhood[i], nCluster, flannRadius, nMinPts, maxPts, kdIndex, vecNodes);
				}
			//	ExpandCluster(rgpNeighbourhood, nCluster, flannRadius, nMinPts, maxPts, kdIndex, vecNodes);
				nCluster++; 
			} 
		} 
	} 
	return nCluster; 
} 


/*
#include "FastQRFinder.hpp"



bool FastQRFinder::GetNodesInRadius(FastQR::Node* pt, double dblRadius, int nMinPts, vector<FastQR::Node*>& rgpNodesFound, flann::Index * kdIndex, vector<FastQR::Node*> & vecNodes) 
{ 
	int maxSize = nMinPts ;//vecNodes.size();
	Mat indexMat = Mat(1,maxSize,CV_32S);
	Mat distMat = Mat(1,maxSize,CV_32F);
	//int result = kdIndex->radiusSearch(pt->nodePoint,indexMat,distMat,dblRadius,maxSize,flann::SearchParams(32));

	kdIndex->knnSearch(pt->nodePoint,indexMat,distMat,nMinPts,flann::SearchParams(32));

	int result = 0;
	int * indexPtr = indexMat.ptr<int>(0);
	float * distPtr = distMat.ptr<float>(0);
	for (int i=0;i < nMinPts;i++)
	{		
		if (distPtr[i] < dblRadius)
		{
			result++;
			rgpNodesFound.push_back(vecNodes[indexPtr[i]]);
		}
	}	
 	return result == nMinPts;
} 


void FastQRFinder::ExpandCluster(vector<FastQR::Node*>& rgp, int nCluster, double dblEpsilon, int nMinPts, flann::Index * kdIndex, vector<FastQR::Node*> & vecNodes) 
{ 
	vector<FastQR::Node*> rgpNeighbourhood; 
	for (int i = 0; i < (int)rgp.size(); i++) 
	{
		if (!rgp[i]->visited) 
		{ 
			rgp[i]->clusterIndex = nCluster; 
			rgp[i]->visited = true; 
			rgpNeighbourhood.push_back(rgp[i]); 
		} 

		for (int i = 0; i < (int)rgpNeighbourhood.size(); i++) 
		{ 
			FastQR::Node* pNodeNear = rgpNeighbourhood[i]; 
			vector<FastQR::Node*> rgpNeighbourhood2; 
			GetNodesInRadius(pNodeNear, dblEpsilon, nMinPts, rgpNeighbourhood2, kdIndex, vecNodes);

			{ 
				// append to rgpNeighbourhood items in rgpNeighbourhood2 that aren't already in rgpNeighbourhood 
				for (int j = 0; j < (int)rgpNeighbourhood2.size(); j++) 
				{ 
					FastQR::Node* pNode = rgpNeighbourhood2[j]; 
					if (!pNode->visited) 
						pNode->visited = true; 
					if (pNode->clusterIndex < 0) 
					{ 
						pNode->clusterIndex = nCluster; 
						rgpNeighbourhood.push_back(pNode); 
					} 
				} 
			} 
		} 
	}
} 

int FastQRFinder::RunDBScan(vector<FastQR::Node*> & vecNodes, flann::Index * kdIndex, double flannRadius, int nMinPts) 
{ 
	int nCluster = 0;

	for (vector<FastQR::Node*>::const_iterator it = vecNodes.begin(); it != vecNodes.end(); it++) 
	{ 
		FastQR::Node* pNode = *it; 
		if (!pNode->visited) 
		{ 
			pNode->visited = true; 

			vector<FastQR::Node*> rgpNeighbourhood; 
			if (GetNodesInRadius(pNode, flannRadius, nMinPts, rgpNeighbourhood,kdIndex, vecNodes)) 
			{ 
				pNode->clusterIndex = nCluster; 
				pNode->visited = true; 
				ExpandCluster(rgpNeighbourhood, nCluster, flannRadius, nMinPts,kdIndex, vecNodes);
				nCluster++; 
			} 
		} 
	} 
	return nCluster; 
} 

*/