#ifndef REALM_DEFINITION_HPP_
#define REALM_DEFINITION_HPP_

#include "LogDefinitions.h"
#include "ARObjectDefinition.hpp"
#include "NetworkMessages.hpp"
#include <vector>
#include <jni.h>

class RealmDefinition : public IncomingMessage
{
public:
	RealmDefinition(std::string _Name)
	{
		Name = _Name;
	}

	std::string GetAction()
	{
		return "RealmDefinition";
	};

	//Fields
	std::vector<ARObjectDefinition*> Children;
	std::string Name;

	float qrSize;

	static RealmDefinition * FromJNIEnv(JNIEnv * env, jobject realmObject);

		
};



#endif
