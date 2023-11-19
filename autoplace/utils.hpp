#include <jni.h>
#include "j_classloader.hpp"

namespace utils {
	JavaVM* get_jvm_instance();
	j_classloader* get_classloader_by_thread_name(const char* thread_name);
}