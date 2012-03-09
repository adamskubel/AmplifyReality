#include "PreferenceManager.hpp"

PreferenceManager::PreferenceManager(jobject _activityObject)
{
	activityObject = _activityObject;
	preferenceMap.insert(pair<string,string>("connect.hostname","192.168.1.9:12312"));
	preferenceMapUpdated = true;
}

string PreferenceManager::GetPreference(string key, string defaultValue)
{
	if (preferenceMap.count(key) == 0)
		return defaultValue;
	else
		return preferenceMap[key];
}

void PreferenceManager::SetPreference(string key, string value)
{	
	if (preferenceMap.count(key) == 0 || preferenceMap[key].compare(value) == 0)
	{
		preferenceMap[key] = value;
		preferenceMapUpdated = true;
		LOGI(LOGTAG_MAIN,"Added/updated preference. %s=%s",key.c_str(),value.c_str());
	}
}

void PreferenceManager::Update(JavaVM * javaVM)
{
	if (preferenceMapUpdated)
	{
		JNIEnv * env = JNIUtils::GetJNIEnv(javaVM);
		SavePreferences(env);
		LOGD(LOGTAG_JNI,"Detaching thread");
		javaVM->DetachCurrentThread();
	}
}

void PreferenceManager::LoadAllPreferences(JNIEnv * env)
{
	vector<string> prefVector;
	prefVector.push_back("connect.hostname");
	prefVector.push_back("connect.username");
	prefVector.push_back("connect.password");

	for (int i=0;i<prefVector.size();i++)
	{
		string key = prefVector[i];
		try
		{
			string value = GetPreferenceFromJava(env,key);
			preferenceMap.insert(pair<string,string>(key,value));
		}
		catch (exception & e)
		{
			LOGW(LOGTAG_JNI,"Preference '%s' not found! Exception = %s",key.c_str(),e.what());
		}
	}
}

string PreferenceManager::GetPreferenceFromJava(JNIEnv * env, string key)
{	
	LOGD(LOGTAG_JNI,"Getting preference from Java: %s",key.c_str());
	jclass activityClass = env->GetObjectClass(activityObject);
	jmethodID getMethod = env->GetMethodID(activityClass,"getPreference","(Ljava/lang/String;)Ljava/lang/String;");


	jstring prefKey = env->NewStringUTF(key.c_str());
	jstring result = (jstring)env->CallObjectMethod(activityObject,getMethod,prefKey);
	if (result == NULL)
		throw exception();

	return string(env->GetStringUTFChars(result,NULL));		
}

void PreferenceManager::SavePreferences(JNIEnv * env)
{
	for (map<string,string>::iterator it = preferenceMap.begin();it != preferenceMap.end();it++)
	{
		SetPreferenceInJava(env,(*it).first,(*it).second);
	}
	preferenceMapUpdated = false;
}

void PreferenceManager::SetPreferenceInJava(JNIEnv * env, string key, string value)
{	
	LOGD(LOGTAG_JNI,"Setting preference: %s=%s",key.c_str(),value.c_str());

	jclass activityClass = env->GetObjectClass(activityObject);
	jmethodID setMethod = env->GetMethodID(activityClass,"setPreference","(Ljava/lang/String;Ljava/lang/String;)V");

	jstring prefKey = env->NewStringUTF(key.c_str());
	jstring prefValue = env->NewStringUTF(value.c_str());

	env->CallVoidMethod(activityObject,setMethod,prefKey,prefValue);
}