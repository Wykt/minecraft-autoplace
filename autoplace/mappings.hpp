#include <memory>
#include <vector>
#include <jni.h>

namespace mappings {
	class c_classmapping {
	public:
		const char* name;
		const char* mapped_name;

		c_classmapping(const char* name, const char* mapped_name);
		const char* get_class_name();
	};

	class c_methodmapping {
	public:
		const char* name;
		const char* mapped_name;
		const char* desc;
		const char* mapped_desc;

		c_methodmapping(const char* name, const char* desc, const char* mapped_name, const char* mapped_desc);

		const char* get_method_name();
		const char* get_method_desc();

		jmethodID get_static_method_id(JNIEnv* env, jclass cls);
		jmethodID get_method_id(JNIEnv* env, jclass cls);
	};

	class c_fieldmapping {
	public:
		const char* name;
		const char* mapped_name;
		const char* desc;
		const char* mapped_desc;

		c_fieldmapping(const char* name, const char* desc, const char* mapped_name, const char* mapped_desc);

		const char* get_field_name();
		const char* get_field_desc();

		jfieldID get_static_field_id(JNIEnv* env, jclass cls);
		jfieldID get_field_id(JNIEnv* env, jclass cls);
	};

	extern std::vector<c_classmapping*> class_mappings;
	extern std::vector<c_methodmapping*> method_mappings;
	extern std::vector<c_fieldmapping*> field_mappings;
	extern bool mcp_mappings;

	c_classmapping* get_classmapping_by_name(const char* name);
	c_methodmapping* get_methodmapping_by_name(const char* name, const char* desc);
	c_fieldmapping* get_fieldmapping_by_name(const char* name, const char* desc);

	void initialize_mappings();
};
