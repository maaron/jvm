
#include "java\method.h"
#include "java\clazz.h"
#include "java\jvm.h"

namespace java
{
	method::method()
		: _class(), _id(nullptr), _methodObj()
	{
	}

	method::method(local_ref<jclass> cls, local_ref<jobject> methodObj)
		: _class(cls), _id(jni::from_reflected_method(methodObj.get())), _methodObj(methodObj)
	{
	}

	std::string method::name() const
	{
		local_ref<jclass> method_class = jni::get_object_class(_methodObj.get());
		auto getName = jni::get_method_id(method_class.get(), "getName", "()Ljava/lang/String;");
		local_ref<jstring> name = jni::call_method<jobject>(_methodObj.get(), getName);
		return jstring_str(name.get());
	}

	jsize method::num_args()
	{
		local_ref<jclass> method_class = jni::get_object_class(_methodObj.get());
		auto getParameterTypes = jni::get_method_id(method_class.get(), "getParameterTypes", "()[Ljava/lang/Class;");
		local_ref<jobjectArray> parameter_types = jni::call_method<jobject>(_methodObj.get(), getParameterTypes);
		return jni::get_array_length((jobjectArray)parameter_types.get());
	}

	bool method::is_args_assignable(const std::vector<clazz>& classes) const
	{
		local_ref<jclass> method_class = jni::get_object_class(_methodObj.get());
		auto getParameterTypes = jni::get_method_id(method_class.get(), "getParameterTypes", "()[Ljava/lang/Class;");
		local_ref<jobjectArray> parameter_types = (jobjectArray)jni::call_method<jobject>(_methodObj.get(), getParameterTypes);
		auto num_args = jni::get_array_length(parameter_types.get());
		if (num_args != classes.size()) return false;

		for (long i = 0; i < num_args; i++)
		{
			local_ref<jclass> target = (jclass)jni::get_object_array_element(parameter_types.get(), i);
			jclass src = classes[i].native();
			if (src != nullptr && !jni::is_assignable_from(src, target.get()))
				return false;
		}

		return true;
	}

	std::string method::return_type()
	{
		// Get the return type in order to know which JNI function to call
		local_ref<jclass> method_class = jni::get_object_class(_methodObj.get());
		auto getReturnType = jni::get_method_id(method_class.get(), "getReturnType", "()Ljava/lang/Class;");
		local_ref<jobject> return_type = jni::call_method<jobject>(_methodObj.get(), getReturnType);

		local_ref<jclass> class_class = jni::get_object_class(return_type.get());
		auto getName = jni::get_method_id(class_class.get(), "getName", "()Ljava/lang/String;");
		local_ref<jstring> jstr = jni::call_method<jobject>(return_type.get(), getName);
		return jstring_str(jstr.get());
	}

	void method_iterator::get()
	{
		_current = method(_class, jni::get_object_array_element((jobjectArray)_methods.get(), _index));
	}

	method_list::method_list(local_ref<jclass> cls, local_ref<jobjectArray> methods) : _class(cls), _methods(methods)
	{
		_size = jni::get_array_length(_methods.get());
	}

	method_iterator method_list::begin() { return method_iterator(_class, _methods, 0); }

	method_iterator method_list::end() { return method_iterator(_class, _methods, _size); }
}