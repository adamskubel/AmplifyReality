#include "RealmDefinition.hpp"

RealmDefinition * RealmDefinition::FromJNIEnv(JNIEnv * env, jobject realmObject)
{
	jclass realmClass = env->GetObjectClass(realmObject);
	JNIUtils::LogJNIError(env);
	try
	{
		
		/*jfieldID nameField = env->GetFieldID(realmClass,"Name","Ljava/lang/String;");
		jstring nameObject = (jstring)env->GetObjectField(realmObject,nameField);	*/

		jmethodID nameMethod = env->GetMethodID(realmClass,"getCodeName","()Ljava/lang/String;");
		if (JNIUtils::LogJNIError(env)) 
			return new RealmDefinition("fail");
		jstring nameObject = (jstring)env->CallObjectMethod(realmObject,nameMethod);

		jfieldID qrUnitField = env->GetFieldID(realmClass,"QRUnitSize","F");
		jfloat qrSize = env->GetFloatField(realmObject,qrUnitField);		

		RealmDefinition * newRealm = new RealmDefinition( std::string(env->GetStringUTFChars(nameObject,JNI_FALSE)));

		jmethodID listMethod = env->GetMethodID(realmClass,"getObjectList","()Ljava/util/ArrayList;");
		jobject arrayList = env->CallObjectMethod(realmObject,listMethod);
		JNIUtils::LogJNIError(env);
		if (arrayList != NULL)
		{
			jclass arrayListClass = env->GetObjectClass(arrayList);
			jmethodID getMethod = env->GetMethodID(arrayListClass,"get","(I)Ljava/lang/Object;");

			jmethodID sizeMethod = env->GetMethodID(arrayListClass,"size","()I");
			jint listSize = env->CallIntMethod(arrayList,sizeMethod);

			for (int i=0;i<listSize;i++)
			{
				jobject arObject = env->CallObjectMethod(arrayList,getMethod,i);
				if (arObject == NULL)
				{
					LOGI(LOGTAG_JNI,"Object %d was NULL!",i);
				}
				LOGI(LOGTAG_JNI,"Converting Java ARObject %d, val =%d",i,arObject);
				newRealm->Children.push_back(ARObjectDefinition::FromJNIObject(env,arObject));
			}
		}

		LOGI(LOGTAG_JNI,"Created realm with name = %s, qrSize=%f",newRealm->Name.c_str(),qrSize);
		newRealm->qrSize = qrSize;

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