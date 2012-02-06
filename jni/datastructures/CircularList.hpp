//Custom data structure for circular linked list


#ifndef CIRCULAR_LIST_HPP_
#define CIRCULAR_LIST_HPP_


//struct CircularListNode
//{
//public:
//	T data;
//	CircularListNode<T> next;
//	CircularListNode<T> previous;
//
//
//};

template <typename T>
class CircularList
{
public:
	CircularList(int maxSize);
	~CircularList();
	void add(T data);
	T front();
	T next();
	T getRelative(int offset);

	void pop_front();

	void clear();
	bool empty();
	int size();
	int getMaxSize();




private:
	T * dataArray;
	int lastPosition, numDefined, maxSize;
	int getPosition(int num);


};

template <typename T>
CircularList<T>::CircularList(int _maxSize)
{
	maxSize = _maxSize;
	dataArray = new T[maxSize];
	LOGV("CLIST","Creating array of size %d",maxSize);
	numDefined = 0;
	lastPosition = maxSize -1;
}

template <typename T>
CircularList<T>::~CircularList()
{
	delete[] dataArray;
}

template <typename T>
void CircularList<T>::add(T data)
{
	lastPosition = (lastPosition + 1) % maxSize;
	//LOGV("CLIST","Adding data in position %d",lastPosition);
	dataArray[lastPosition] = data;
	numDefined = (numDefined > maxSize) ? maxSize : numDefined+1;
	//LOGV("CLIST","Complete");
}

template <typename T>
T CircularList<T>::next()
{
	lastPosition = (lastPosition + 1) % maxSize;
	//LOGV("CLIST","Getting data in position=%d",lastPosition);
	return dataArray[lastPosition];
}

template <typename T>
T CircularList<T>::front()
{
	//LOGV("CLIST","Getting data in position=%d",lastPosition);
	return dataArray[lastPosition];
}


//Remove the last item added
template <typename T>
void CircularList<T>::pop_front()
{
	delete dataArray[lastPosition];
	lastPosition = (lastPosition - 1) % maxSize;
	if (lastPosition < 0) lastPosition += maxSize;
	if (numDefined > 0) 
		numDefined--;
}

template <typename T>
T CircularList<T>::getRelative(int offset)
{
	int index = (lastPosition+offset)%maxSize;
	if (index < 0) index += maxSize;
	return dataArray[index];
}

template <typename T>
int CircularList<T>::size()
{
	return numDefined;
}

template <typename T>
int CircularList<T>::getMaxSize()
{
	return maxSize;
}

template <typename T>
bool CircularList<T>::empty()
{
	return numDefined == 0;
}

template <typename T>
void CircularList<T>::clear()
{	
	delete[] dataArray;
	dataArray = new T[maxSize];
}



#endif