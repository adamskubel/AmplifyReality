#include "WorldLoader.hpp"


WorldLoader::WorldLoader()
{
	resourceManager = new ResourceManager();
	state = WorldStates::LookingForCode;
}


WorldStates::WorldState WorldLoader::GetState()
{
	return state;
}

void WorldLoader::LoadRealm(std::string code)
{
	realmCode = code;
	state = WorldStates::CodeProvided;
}

void WorldLoader::Update(Engine * engine)
{	
	vector<IncomingMessage*> msgs;

	resourceManager->Update(engine);

	switch(state)
	{
	case(WorldStates::CodeProvided):
		engine->communicator->SendMessage(createRealmRequest(realmCode));
		state = WorldStates::WaitingForRealm;
		break;
	case(WorldStates::WaitingForRealm):
		if (engine->communicator->FilterMessages("RealmDefinition",msgs))
		{
			LOGI(LOGTAG_WORLD,"Realm received from server. Count=%d",msgs.size());
			RealmDefinition * realm = (RealmDefinition*) msgs.at(0);			
		}
		break;
	case(WorldStates::WaitingForResources):

		break;
	}
}

void WorldLoader::ResourcesReady(void * sender)
{
	state = WorldStates::ResourcesReady;
}

void WorldLoader::SetCurrentRealm(RealmDefinition * realm)
{
	currentRealm = realm;

	bool resourcesFound = true;
	for (int i=0;i<currentRealm->Children.size();i++)
	{
		ARObjectDefinition * arObjectDef = currentRealm->Children.at(i);

		string modelName = arObjectDef->ModelName;
		WavefrontModel model;

		bool found = resourceManager->GetResource<WavefrontModel>(modelName,&model);

		//Resource is not available in cache, so we need to wait for the resource manager to find it. 		
		//Set waiting state. Ensure it is only set once.
		if (!found && state != WorldStates::WaitingForResources)
		{
			LOGI(LOGTAG_WORLD,"A resource was not found. Changing state to waiting.");

			//Set waiting state. Ensure it is only set once.
			state = WorldStates::WaitingForResources;
			resourceManager->AddResourceCallback(ResourcesLoadedDelegate::from_method<WorldLoader,&WorldLoader::ResourcesReady>(this));
		}
	}

	//If all resources were available, construct the objects
	if (state == WorldStates::WaitingForRealm)
		BuildRealm();

}

void WorldLoader::BuildRealm()
{
	for (int i=0;i<currentRealm->Children.size();i++)
	{
		ARObjectDefinition * arObjectDef = currentRealm->Children.at(i);

		string modelName = arObjectDef->ModelName;
		WavefrontModel model;

		bool found = resourceManager->GetResource<WavefrontModel>(modelName,&model);
	
		if (found) 
		{
			LOGI(LOGTAG_WORLD,"Resource found. Creating object.");
			objLoader loader;
			loader.loadFromString(model.ModelData);
			WavefrontGLObject * glObject = WavefrontGLObject::FromObjFile(loader);
			ARObject * newARObject = new ARObject(glObject);
			newARObject->scale = arObjectDef->Scale;
			newARObject->position = arObjectDef->Position;
			newARObject->rotation = arObjectDef->Rotation;
			//Name..?
			
			arObjects.push_back(newARObject);
		}
		else
		{
			LOGW(LOGTAG_WORLD, "Resource with name %s should be present, but isn't!",modelName.c_str());
		}
	}
	state = WorldStates::WorldReady;
}

void WorldLoader::PopulateARView(AugmentedView *& arView)
{
	LOGD(LOGTAG_WORLD,"Populating ARView with %d objects.",arObjects.size());
	while (!arObjects.empty())
	{
		arView->AddObject(arObjects.back());
		arObjects.pop_back();
	}
}

OutgoingMessage * createRealmRequest(std::string code)
{
	std::string request("RealmRequest:");
	request.append(code);
	return new StringMessage(request);

}