#include "ResourceManager.hpp"


ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
	map<std::string,IncomingMessage*>::iterator it;

	for (it = resourceMap.begin();it != resourceMap.end();it++)
	{
		delete (*it).second;
	}
	resourceMap.clear();

	LOGD(LOGTAG_NETWORKING,"Deleted resource manager.");
}



void ResourceManager::AddResourceCallback(ResourcesLoadedDelegate _delegate)
{
	callbackVector.push_back(_delegate);
}


bool ResourceManager::AlreadyRequested(string resourceName)
{	
	if (resourcesWaiting.find(resourceName) == resourcesWaiting.end() &&
		resourcesRequired.find(resourceName) == resourcesRequired.end())
		return false;

	return true;
}

bool ResourceManager::FindResource(string resourceName)
{
	//Predefined resources
	if (resourceName.compare("Cube") == 0)
	{
		LOGD(LOGTAG_IO,"Using predefined resource '%s'", resourceName.c_str());
		return true;
	}

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
		return true;
}


void ResourceManager::RequestResource(string resourceName)
{
	//check if already requested
	if (!AlreadyRequested(resourceName))
		resourcesRequired.insert(resourceName);
}

void ResourceManager::Update(Engine * engine)
{
	//Check if any of the required resources arrived this update
	if (!resourcesWaiting.empty())
	{
		vector<IncomingMessage*> newMsgs;
		if (engine->communicator->FilterMessages("Wavefront",newMsgs))
		{
			for (int i=0;i<newMsgs.size();i++)
			{
				WavefrontModel * wavefrontMsg = (WavefrontModel*)newMsgs.at(i);
				//Check if the expected resource is one we're waiting for
				if (resourcesWaiting.find(wavefrontMsg->ModelName) != resourcesWaiting.end())
				{
					LOGI(LOGTAG_NETWORKING,"Resource received! Name=%s",wavefrontMsg->ModelName.c_str());
					resourcesWaiting.erase(resourcesWaiting.find(wavefrontMsg->ModelName));	
					resourceMap[wavefrontMsg->ModelName] = newMsgs.at(i);
				}
				else
				{
					LOGW(LOGTAG_NETWORKING,"Unexpected resource, name=%s", wavefrontMsg->ModelName.c_str());
				}
			}
		}

		if (resourcesWaiting.empty())
		{
			LOGD(LOGTAG_NETWORKING,"All resources found, calling listeners.");
			for (int i=0;i<callbackVector.size();i++)
			{
				callbackVector.at(i)(this);
			}
		}
	}


	//Send requests for needed resources, and add them to waiting list
	if (!resourcesRequired.empty())
	{
		set<std::string>::iterator it;
		for (it = resourcesRequired.begin();it != resourcesRequired.end();it++)
		{
			engine->communicator->SendMessage(createResourceRequest(*it));
			resourcesWaiting.insert(*it);
		}
		resourcesRequired.clear();
	}
}

OutgoingMessage * ResourceManager::createResourceRequest(std::string resourceName)
{
	std::string msgText = "ResourceRequest:";
	msgText.append(resourceName);
	return new StringMessage(msgText);
}