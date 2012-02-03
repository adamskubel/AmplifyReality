#ifndef RESOURCE_MANAGER_HPP_
#define RESOURCE_MANAGER_HPP_

#include "model/Engine.hpp"

using namespace std;



typedef srutil::delegate<void (void*)> ResourcesLoadedDelegate;


class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();
	void Update(Engine * engine);


	template< typename T>
	bool GetResource(string resourceName, T * resource);

	void AddResourceCallback(ResourcesLoadedDelegate  resourceLoadedDelegate); 


private:
//	map<string, void*> resourceMap;
	
	vector<string> resourcesRequired;
	vector<string> resourcesWaiting;

	vector<string> fakeMapKey;
	vector<void*> fakeMapValue;

	void RequestResource(string resourceName);

	bool AlreadyRequested(string resourceName);

	bool getFromFakeMap(std::string key, void *& output);
	void addToFakeMap(std::string key, void * value);

	vector<ResourcesLoadedDelegate> callbackVector;

};

#endif


template< typename T>
bool ResourceManager::GetResource(std::string resourceName, T * resource)
{
	/*map<std::string,void*>::iterator it;

	it = resourceMap.find(resourceName);
	if (it == map<std::string,void*>::end)*/
	void * obj;
	if (getFromFakeMap(resourceName,obj))
	{
		if (!AlreadyRequested(resourceName))
		{
			LOGD(LOGTAG_IO,"Resource not found, requesting. Name=%s",resourceName.c_str());
			RequestResource(resourceName);
		}
	}
	else
	{
		LOGD(LOGTAG_IO,"Resource found. Name=%s",resourceName.c_str());
		return (T*)obj;
		//return (T*)resourceMap.at(it);
	}
}