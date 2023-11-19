#include "injector.hpp"
#include "gui.hpp"

std::unique_ptr<c_client> client;

DWORD injector::main_thread(HMODULE hModule)
{
	JavaVM* jvm = utils::get_jvm_instance();

	if (!jvm)
	{
		ERROR_LOG("Couldn't find JVM instance");
		FreeLibraryAndExitThread(hModule, 1);
	}

	client = std::make_unique<c_client>(jvm);
	client->initialize();

	if (!client->env)
	{
		ERROR_LOG("Couldn't find JNIEnv");
		FreeLibraryAndExitThread(hModule, 2);
	}

	if (!client->jvmti)
	{
		ERROR_LOG("Couldn't find jvmtiEnv");
		FreeLibraryAndExitThread(hModule, 3);
	}

	gui::start_gui();
	FreeLibraryAndExitThread(hModule, 0);
}