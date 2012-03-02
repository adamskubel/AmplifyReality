#ifndef WORLD_LOADER_HPP_
#define WORLD_LOADER_HPP_

#include "LogDefinitions.h"
#include "AmplifyRealityGlobals.hpp"
#include "model/Engine.hpp"
#include "model/ResourceManager.hpp"
#include "display/model/AugmentedView.hpp"

namespace WorldStates
{
	enum WorldState
	{
		LookingForCode,
		CodeProvided,
		WaitingForRealm,
		WaitingForResources,
		ResourcesReady,
		WorldReady
	};
}

class WorldLoader
{
public:
	WorldLoader();
	~WorldLoader();
	void LoadRealm(std::string code);
	void Update(Engine * engine);
	
	WorldStates::WorldState GetState();

	void ResourcesReady(void *);

	void PopulateARView(AugmentedView * arView);

private:
	WorldStates::WorldState state;
	std::string realmCode;
	OutgoingMessage * createRealmRequest(std::string code);
	//AugmentedView * myARView;
	RealmDefinition * currentRealm;
	vector<ARObject*> arObjects;
	
	void SetState(WorldStates::WorldState state);
	void SetCurrentRealm(RealmDefinition * realm);

	ResourceManager * resourceManager;
	void BuildRealm();


};


#endif