#include "j_classloader.hpp"
#include "injector.hpp"

j_classloader::j_classloader(jobject classloader)
{
	this->classloader = classloader;
	jclass c_classloader = client->env->FindClass("java/lang/ClassLoader");
	this->m_findClass = client->env->GetMethodID(c_classloader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

	client->env->DeleteLocalRef(c_classloader);
}

j_classloader::~j_classloader()
{
	client->env->DeleteLocalRef(this->classloader);
}

jclass j_classloader::find_class(const char* class_name)
{
	jclass clazz = nullptr;
	jstring j_class_name = client->env->NewStringUTF(class_name);

	jobject result = client->env->CallObjectMethod(this->classloader, m_findClass, j_class_name);

	if (result)
	{
		clazz = reinterpret_cast<jclass>(result);
	}
	
	client->env->DeleteLocalRef(j_class_name);
	return clazz;
}