#ifndef RESOURCE_MANAGER_HPP_
#define RESOURCE_MANAGER_HPP_

#include "model/Engine.hpp"
#include <set>

using namespace std;



typedef srutil::delegate<void (void*)> ResourcesLoadedDelegate;


class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();
	void Update(Engine * engine);
	
	template<typename T>
	bool GetResource(std::string resourceName,T *& resource);

	bool FindResource(std::string resourceName);
	void AddResourceCallback(ResourcesLoadedDelegate  resourceLoadedDelegate); 


private:
	map<string, IncomingMessage*> resourceMap;
	
	set<string> resourcesRequired;
	set<string> resourcesWaiting;

	void RequestResource(string resourceName);

	bool AlreadyRequested(string resourceName);

	OutgoingMessage * createResourceRequest(string resourceName);
	vector<ResourcesLoadedDelegate> callbackVector;

};



template<typename T>
bool ResourceManager::GetResource(std::string resourceName, T *& resource)
{
	if (resourceMap.find(resourceName) == resourceMap.end())
	{
		if (!AlreadyRequested(resourceName))
		{
			LOGD(LOGTAG_IO,"Resource not found, requesting. Name=%s",resourceName.c_str());
			RequestResource(resourceName);
		}
		else
		{
			LOGV(LOGTAG_IO,"Already requested resource with name=%s",resourceName.c_str());			
		}
		return false;
	}
	else
	{
		LOGD(LOGTAG_IO,"Resource found, returning. Name=%s",resourceName.c_str());
		resource = (T*)resourceMap.at(resourceName);
		return true;
	}
}


#endif



