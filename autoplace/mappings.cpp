#include "mappings.hpp"

using namespace mappings;

std::vector<c_classmapping*> mappings::class_mappings;
std::vector<c_methodmapping*> mappings::method_mappings;
std::vector<c_fieldmapping*> mappings::field_mappings;
bool mappings::mcp_mappings = false;

c_classmapping::c_classmapping(const char* name, const char* mapped_name)
{
	this->name = name;
	this->mapped_name = mapped_name;
}

c_methodmapping::c_methodmapping(const char* name, const char* desc, const char* mapped_name, const char* mapped_desc)
{
	this->name = name;
	this->mapped_name = mapped_name;
	this->desc = desc;
	this->mapped_desc = mapped_desc;
}

c_fieldmapping::c_fieldmapping(const char* name, const char* desc, const char* mapped_name, const char* mapped_desc)
{
	this->name = name;
	this->mapped_name = mapped_name;
	this->desc = desc;
	this->mapped_desc = mapped_desc;
}

const char* c_classmapping::get_class_name()
{
	return mcp_mappings ? this->name : this->mapped_name;
}

const char* c_methodmapping::get_method_name()
{
	return mcp_mappings ? this->name : this->mapped_name;
}

const char* c_methodmapping::get_method_desc()
{
	return mcp_mappings ? this->desc : this->mapped_desc;
}

jmethodID c_methodmapping::get_static_method_id(JNIEnv* env, jclass cls)
{
	return env->GetStaticMethodID(cls, this->get_method_name(), this->get_method_desc());
}

jmethodID c_methodmapping::get_method_id(JNIEnv* env, jclass cls)
{
	return env->GetMethodID(cls, this->get_method_name(), this->get_method_desc());
}

const char* c_fieldmapping::get_field_name()
{
	return mcp_mappings ? this->name : this->mapped_name;
}

const char* c_fieldmapping::get_field_desc()
{

	return mcp_mappings ? this->desc : this->mapped_desc;
}

jfieldID c_fieldmapping::get_static_field_id(JNIEnv* env, jclass cls)
{
	return env->GetStaticFieldID(cls, this->get_field_name(), this->get_field_desc());
}

jfieldID c_fieldmapping::get_field_id(JNIEnv* env, jclass cls)
{
	return env->GetFieldID(cls, this->get_field_name(), this->get_field_desc());
}

c_classmapping* mappings::get_classmapping_by_name(const char* name)
{
	for (c_classmapping* mapping : class_mappings)
	{
		if (strcmp(name, mapping->name) != 0)
		{
			continue;
		}

		return mapping;
	}

	return NULL;
}

c_methodmapping* mappings::get_methodmapping_by_name(const char* name, const char* desc)
{
	for (c_methodmapping* mapping : method_mappings)
	{
		if (strcmp(name, mapping->name) != 0 || strcmp(desc, mapping->desc) != 0)
		{
			continue;
		}

		return mapping;
	}

	return NULL;
}

c_fieldmapping* mappings::get_fieldmapping_by_name(const char* name, const char* desc)
{
	for (c_fieldmapping* mapping : field_mappings)
	{
		if (strcmp(name, mapping->name) != 0 || strcmp(desc, mapping->desc) != 0)
		{
			continue;
		}

		return mapping;
	}

	return NULL;
}

// that's so gay
void mappings::initialize_mappings()
{
	class_mappings.push_back(new c_classmapping("net/minecraft/client/Minecraft", "ave"));
	class_mappings.push_back(new c_classmapping("net/minecraft/util/MovingObjectPosition", "auh"));
	class_mappings.push_back(new c_classmapping("net/minecraft/util/MovingObjectPosition$MovingObjectType", "auh$a"));
	class_mappings.push_back(new c_classmapping("net/minecraft/util/EnumFacing", "cq"));

	field_mappings.push_back(new c_fieldmapping("rightClickDelayTimer", "I", "ap", "I"));
	field_mappings.push_back(new c_fieldmapping("thePlayer", "Lnet/minecraft/client/entity/EntityPlayerSP;", "h", "Lbew;"));
	field_mappings.push_back(new c_fieldmapping("UP", "Lnet/minecraft/util/EnumFacing;", "b", "Lcq;"));
	field_mappings.push_back(new c_fieldmapping("DOWN", "Lnet/minecraft/util/EnumFacing;", "a", "Lcq;"));
	field_mappings.push_back(new c_fieldmapping("objectMouseOver", "Lnet/minecraft/util/MovingObjectPosition;", "s", "Lauh;"));
	field_mappings.push_back(new c_fieldmapping("BLOCK", "Lnet/minecraft/util/MovingObjectPosition$MovingObjectType;", "b", "Lauh$a;"));
	field_mappings.push_back(new c_fieldmapping("sideHit", "Lnet/minecraft/util/EnumFacing;", "b", "Lcq;"));
	field_mappings.push_back(new c_fieldmapping("typeOfHit", "Lnet/minecraft/util/MovingObjectPosition$MovingObjectType;", "a", "Lauh$a;"));

	method_mappings.push_back(new c_methodmapping("getMinecraft", "()Lnet/minecraft/client/Minecraft;", "A", "()Lave;"));
	method_mappings.push_back(new c_methodmapping("rightClickMouse", "()V", "ax", "()V"));
}