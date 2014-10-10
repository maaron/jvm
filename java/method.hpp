
#include "java\method.h"
#include "java\clazz.h"
#include "java\jvm.h"

using namespace java;

method::method()
    : _class(nullptr), _id(nullptr), _methodObj(nullptr)
{
}

method::method(jclass cls, jobject methodObj) 
    : _class(cls), _id(jvm::from_reflected_method(methodObj)), _methodObj(methodObj)
{
}

std::string method::name() const
{
    auto method_class = jvm::get_object_class(_methodObj);
    auto getName = jvm::get_method_id(method_class, "getName", "()Ljava/lang/String;");
    return jstring_str((jstring)jvm::call_method<jobject>(_methodObj, getName));
}

jsize method::num_args()
{
    auto method_class = jvm::get_object_class(_methodObj);
    auto getParameterTypes = jvm::get_method_id(method_class, "getParameterTypes", "()[Ljava/lang/Class;");
    auto parameter_types = (jobjectArray)jvm::call_method<jobject>(_methodObj, getParameterTypes);
    return jvm::get_array_length(parameter_types);
}

bool method::is_args_assignable(const std::vector<clazz>& classes) const
{
    auto method_class = jvm::get_object_class(_methodObj);
    auto getParameterTypes = jvm::get_method_id(method_class, "getParameterTypes", "()[Ljava/lang/Class;");
    auto parameter_types = (jobjectArray)jvm::call_method<jobject>(_methodObj, getParameterTypes);
    auto num_args = jvm::get_array_length(parameter_types);
    if (num_args != classes.size()) return false;

    for (long i = 0; i < num_args; i++)
    {
        jclass target = (jclass)jvm::get_object_array_element(parameter_types, i);
        jclass src = classes[i].native();
        if (src != nullptr && !jvm::is_assignable_from(src, target)) 
            return false;
    }

    return true;
}

std::string method::return_type()
{
    // Get the return type in order to know which JNI function to call
    auto method_class = jvm::get_object_class(_methodObj);
    auto getReturnType = jvm::get_method_id(method_class, "getReturnType", "()Ljava/lang/Class;");
    auto return_type = jvm::call_method<jobject>(_methodObj, getReturnType);

    auto class_class = jvm::get_object_class(return_type);
    auto getName = jvm::get_method_id(class_class, "getName", "()Ljava/lang/String;");
    return jstring_str((jstring)jvm::call_method<jobject>(return_type, getName));
}

void method_iterator::get()
{
    _current = method(_class, jvm::get_object_array_element(_array, _index));
}

method_list::method_list(jclass cls, jobjectArray a) : _class(cls), _array(a)
{
    _size = jvm::get_array_length(_array);
}

method_iterator method_list::begin() { return method_iterator(_class, _array, 0); }

method_iterator method_list::end() { return method_iterator(_class, _array, _size); }
