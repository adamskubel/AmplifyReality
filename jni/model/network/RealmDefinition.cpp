#include "RealmDefinition.hpp"

RealmDefinition * RealmDefinition::FromJNIEnv(JNIEnv * env, jobject realmObject)
{
	jclass realmClass = env->GetObjectClass(realmObject);


	/*jfieldID mapField = env->GetFieldID(realmClass,"objectMap","Ljava/util/Map");
	jobject map = env->GetObjectField(realmObject,mapField);

	map*/

	try
	{
		jmethodID listMethod = env->GetMethodID(realmClass,"getObjectList","()Ljava/util/ArrayList;");
		jobject arrayList = env->CallObjectMethod(realmObject,listMethod);

		jclass arrayListClass = env->GetObjectClass(arrayList);
		jmethodID getMethod = env->GetMethodID(arrayListClass,"get","(I)Ljava/lang/Object;");

		jmethodID sizeMethod = env->GetMethodID(arrayListClass,"size","()I");
		jint listSize = env->CallIntMethod(arrayList,sizeMethod);

		jfieldID nameField = env->GetFieldID(realmClass,"Name","Ljava/lang/String;");
		jstring nameObject = (jstring)env->GetObjectField(realmObject,nameField);		

		RealmDefinition * newRealm = new RealmDefinition( std::string(env->GetStringUTFChars(nameObject,JNI_FALSE)));

		LOGI(LOGTAG_JNI,"Created realm with name = %s",newRealm->Name.c_str());


		for (int i=0;i<listSize;i++)
		{
			LOGI(LOGTAG_JNI,"Converting Java ARObject %d",i);
			jobject arObject = env->CallObjectMethod(arrayList,getMethod,i);
			newRealm->Children.push_back(ARObjectDefinition::FromJNIObject(env,arObject));
		}
		return newRealm;
	}
	catch (std::exception &e)
	{
		LOGE("Error in RealmDefinition.FromJNIEnv = %s",e.what());
	}
	return new RealmDefinition("fail");


	//jmethodID listMethod = env->GetMethodID(realmClass,"getObjectArray","()[Lcom/amplifyreality/networking/model/ARObject;");
	//env->
	//jobject objectArray = env->CallObjectMethod(realmObject,listMethod);
	//env->NewObjectArray(
	//jobject object = env->GetObjectArrayElement(objectArray,0);
	//env->GetArrayLength(
	//jclass arObjectClass = env->CallObjectMethod(arrayList,getMethod,0);

}