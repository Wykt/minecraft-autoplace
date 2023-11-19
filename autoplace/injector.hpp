#pragma once

#include <Windows.h>
#include <jvmti.h>
#include <jni.h>
#include <memory>


#include "utils.hpp"

#define CLIENT_NAME "Client"

#define ERROR_LOG(log) MessageBoxA(NULL, log, CLIENT_NAME, MB_ICONERROR);
#define LOG(log) MessageBoxA(NULL, log, CLIENT_NAME, MB_OK);

namespace injector {
	DWORD main_thread(HMODULE hModule);
}

class c_client {
public:
	JavaVM* jvm;
	JNIEnv* env;
	jvmtiEnv* jvmti;

	j_classloader* classloader;

	c_client(JavaVM* jvm)
	{
		this->jvm = jvm;

		jvm->AttachCurrentThread((void**)&env, 0);
		jvm->GetEnv((void**)&env, JNI_VERSION_1_8);
		jvm->GetEnv((void**)&jvmti, JVMTI_VERSION_1_2);

	}

	~c_client()
	{
		jvm->DetachCurrentThread();
	}

	void initialize()
	{
		this->classloader = utils::get_classloader_by_thread_name("Client thread"); // not done in init method as client variable is still not initialized
	}
};

extern std::unique_ptr<c_client> client;