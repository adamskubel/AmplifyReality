#include "ResourceManager.hpp"


ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
	callbackVector.clear();
}


bool ResourceManager::getFromFakeMap(std::string key, void *& output)
{
	for (int i=0;i<fakeMapKey.size();i++)
	{
		if (key.compare(fakeMapKey.at(i)) == 0)
		{
			output = fakeMapValue.at(i);
			return true;
		}
	}
	return false;
}

void ResourceManager::addToFakeMap(std::string key, void * value)
{
	for (int i=0;i<fakeMapKey.size();i++)
	{
		if (key.compare(fakeMapKey.at(i)) == 0)
		{
			fakeMapValue.at(i) = value;
			return;
		}
	}
	fakeMapValue.push_back(value);
	fakeMapKey.push_back(key);

}

void ResourceManager::AddResourceCallback(ResourcesLoadedDelegate _delegate)
{
	callbackVector.push_back(_delegate);
}


bool ResourceManager::AlreadyRequested(string resourceName)
{
	for (int i=0;i<resourcesWaiting.size();i++)
	{
		if (resourcesWaiting.at(i).compare(resourceName) == 0)
			return true;
	}
	for (int i=0;i<resourcesRequired.size();i++)
	{
		if (resourcesRequired.at(i).compare(resourceName) == 0)
			return true;
	}
	return false;
}



void ResourceManager::RequestResource(string resourceName)
{
	//check if already requested
	resourcesRequired.push_back(resourceName);
}

void ResourceManager::Update(Engine * engine)
{
	//Check if any of the required resources arrived this update
	if (!resourcesWaiting.empty())
	{
		vector<WavefrontModel*> newMsgs;
		if (engine->communicator->GetNewResourceMessages(newMsgs))
		{
			for (int i=0;i<newMsgs.size();i++)
			{
				std::string modelName = newMsgs.at(i)->ModelName;

				for (int j=0;j<resourcesWaiting.size();j++)
				{
					if (resourcesWaiting.at(j).compare(modelName) == 0)
					{
						LOGI(LOGTAG_NETWORKING,"Resource received! Name=%s",modelName.c_str());
						//addToFakeMap(modelName,newMsgs.at(i));
						resourceMap[modelName] = newMsgs.at(i);
					}
				}
			}
		}		
	}

	//Send requests for needed resources, and add them to waiting list
	if (!resourcesRequired.empty())
	{
		for (int i=0;i<resourcesRequired.size();i++)
		{
			engine->communicator->SendMessage(new ResourceRequestMessage(resourcesRequired.at(i)));
			resourcesWaiting.push_back(resourcesRequired.at(i));
		}
		resourcesRequired.clear();
	}
}