#ifndef PREF_MANAGER_HPP_
#define PREF_MANAGER_HPP_

#include <map>
#include <jni.h>
#include "util/JNIUtils.hpp"
#include "LogDefinitions.h"

using namespace std;

class PreferenceManager
{
public:
	PreferenceManager(jobject activityObject);
	string GetPreference(string key, string defaultValue);
	void SetPreference(string key, string value);

	void Update(JavaVM * javaVM);
	void LoadAllPreferences(JNIEnv * env);
	void SavePreferences(JNIEnv * env);

private:
	string GetPreferenceFromJava(JNIEnv * env, string key);
	void SetPreferenceInJava(JNIEnv * env, string key, string value);
	JavaVM * javaVM;
	jobject activityObject;
	map<string,string> preferenceMap;
	bool preferenceMapUpdated;
};
#endif