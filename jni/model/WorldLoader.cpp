#include "WorldLoader.hpp"


WorldLoader::WorldLoader()
{
	resourceManager = new ResourceManager();
	state = WorldStates::LookingForCode;
}

WorldLoader::~WorldLoader()
{
	LOGD(LOGTAG_WORLD,"Deleting WorldLoader");
	delete resourceManager;
}

void WorldLoader::SetState(WorldStates::WorldState newState)
{
	LOGD(LOGTAG_WORLD,"Changing state from %d to %d",state,newState);
	state = newState;
}


WorldStates::WorldState WorldLoader::GetState()
{
	return state;
}

void WorldLoader::LoadRealm(std::string code)
{
	realmCode = code;
	SetState(WorldStates::CodeProvided);
}

void WorldLoader::Update(Engine * engine)
{	
	vector<IncomingMessage*> msgs;

	resourceManager->Update(engine);

	switch(state)
	{
	case(WorldStates::CodeProvided):
		engine->communicator->SendMessage(createRealmRequest(realmCode));
		SetState(WorldStates::WaitingForRealm);
		break;
	case(WorldStates::WaitingForRealm):
		if (engine->communicator->FilterMessages("RealmDefinition",msgs))
		{
			LOGI(LOGTAG_WORLD,"Realm received from server. Count=%d",msgs.size());
			RealmDefinition * realm = (RealmDefinition*) msgs.at(0);	
			SetCurrentRealm(realm);
		}
		break;
	case(WorldStates::WaitingForResources):
		//All we can do is wait
		break;
	case(WorldStates::ResourcesReady):
		BuildRealm();
		break;
	}
}

void WorldLoader::ResourcesReady(void * sender)
{
	SetState(WorldStates::ResourcesReady);
}

void WorldLoader::SetCurrentRealm(RealmDefinition * realm)
{
	currentRealm = realm;

	bool resourcesFound = true;
	for (int i=0;i<currentRealm->Children.size();i++)
	{
		ARObjectDefinition * arObjectDef = currentRealm->Children.at(i);

		string modelName = arObjectDef->ModelName;
		IncomingMessage * model = NULL;

		//bool found = resourceManager->GetResource<WavefrontModel>(modelName,model);
		bool found = resourceManager->FindResource(modelName);


		//Resource is not available in cache, so we need to wait for the resource manager to find it. 		
		//Set waiting state. Ensure it is only set once.
		if (!found && state != WorldStates::WaitingForResources)
		{
			LOGI(LOGTAG_WORLD,"A resource was not found. Changing state to waiting.");						
			SetState(WorldStates::WaitingForResources);
			resourceManager->AddResourceCallback(ResourcesLoadedDelegate::from_method<WorldLoader,&WorldLoader::ResourcesReady>(this));
		}
	}

	//If all resources were available, construct the objects
	if (state == WorldStates::WaitingForRealm)
		BuildRealm();

}

void WorldLoader::BuildRealm()
{
	LOGD(LOGTAG_WORLD,"Building realm. %d objects to construct.",currentRealm->Children.size());
	
	for (int i=0;i<currentRealm->Children.size();i++)
	{
		ARObjectDefinition * arObjectDef = currentRealm->Children.at(i);
		if (arObjectDef == NULL)
		{
			LOGW(LOGTAG_WORLD,"Error! Object %d is null!",i);
		}
		LOGD(LOGTAG_WORLD,"Creating object with name=%s",arObjectDef->Name.c_str());
		string modelName = arObjectDef->ModelName;

		WavefrontModel * model = NULL;
		bool found = resourceManager->GetResource<WavefrontModel>(modelName,model);
	
		if (found) 
		{
			objLoader loader;
			loader.loadFromString(model->ModelData);
			WavefrontGLObject * glObject = WavefrontGLObject::FromObjFile(loader);
			LOGD(LOGTAG_WORLD, "Created GLObject with %d vertices",glObject->numVertices);

			ARObject * newARObject = new ARObject(glObject,arObjectDef->Position,arObjectDef->Rotation,arObjectDef->Scale);
			arObjects.push_back(newARObject);
		}
		else
		{
			LOGW(LOGTAG_WORLD, "Resource with name %s should be present, but isn't!",modelName.c_str());
		}
	}

	SetState(WorldStates::WorldReady);
}

void WorldLoader::PopulateARView(AugmentedView * arView)
{
	LOGD(LOGTAG_WORLD,"Populating ARView(%d) with %d objects...",(int)arView,arObjects.size());
	while (!arObjects.empty())
	{
		arView->AddObject(arObjects.back());
		arObjects.pop_back();
	}
	LOGD(LOGTAG_WORLD,"Complete");
}

OutgoingMessage * WorldLoader::createRealmRequest(std::string code)
{
	std::string request("RealmRequest:");
	request.append(code);
	return new StringMessage(request);

}