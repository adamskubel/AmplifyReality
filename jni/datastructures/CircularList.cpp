//#include "datastructures/CircularList.hpp"
//
//template <typename T>
//CircularList<T>::CircularList(int _maxSize)
//{
//	maxSize = _maxSize;
//	dataArray = new T[maxSize];
//	numDefined = 0;
//	lastPosition = maxSize -1;
//}
//
//template <typename T>
//CircularList<T>::~CircularList()
//{
//	delete[] dataArray;
//}
//
//template <typename T>
//void CircularList<T>::add(T data)
//{
//	lastPosition = (lastPosition + 1) % maxSize;
//	dataArray[lastPosition] = data;
//	numDefined = (numDefined > maxSize) ? maxSize : numDefined+1;
//}
//
//template <typename T>
//T CircularList<T>::front()
//{
//	return dataArray[lastPosition];
//}
//
//
////Remove the last item added
//template <typename T>
//void CircularList<T>::pop_front()
//{
//	delete dataArray[lastPosition];
//	lastPosition = (lastPosition - 1) % maxSize;
//	if (lastPosition < 0) lastPosition += maxSize;
//	if (numDefined > 0) 
//		numDefined--;
//}
//
//template <typename T>
//T CircularList<T>::getRelative(int offset)
//{
//	int index = (lastPosition+offset)%maxSize;
//	if (index < 0) index += maxSize;
//	return dataArray[index];
//}
//
//template <typename T>
//int CircularList<T>::size()
//{
//	return numDefined;
//}
//
//template <typename T>
//bool CircularList<T>::empty()
//{
//	return numDefined == 0;
//}
//
//template <typename T>
//void CircularList<T>::clear()
//{
//	while (!empty())
//	{
//		delete front();
//		pop_front();
//	}
//}
//
