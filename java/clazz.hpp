#include "java\clazz.h"

#include <vector>

using namespace java;

std::string clazz::name()
{
    auto cls_cls = jvm::get_object_class(_class);
    auto getName = jvm::get_method_id(cls_cls, "getName", "()Ljava/lang/String;");
    return jstring_str((jstring)jvm::call_method<jobject>(_class, getName));
}

object clazz::static_field(const char* name)
{
    return object(_class).call("getField", name).call("get", object::null());
}

method clazz::lookup_method(const char* name, const std::vector<clazz>& classes)
{
    auto methods = get_methods();
    auto match = std::find_if(methods.begin(), methods.end(), [&](const method& m)
    {
        return m.name() == name && m.is_args_assignable(classes);
    });
    
    if (match == methods.end()) throw std::exception("Method lookup failed");
    return *match;
}

method clazz::lookup_constructor(const std::vector<clazz>& classes)
{
    auto ctors = get_constructors();
    auto match = std::find_if(ctors.begin(), ctors.end(), [&](const method& ctor)
    {
        return ctor.is_args_assignable(classes);
    });
    
    if (match == ctors.end()) throw std::exception("Method lookup failed");
    return *match;
}

method_list clazz::get_methods()
{
    // Get the java.lang.Class object associated with this class.
    jclass cls = jvm::get_object_class(_class);

    // Get an array of methods
    auto getMethods = jvm::get_method_id(cls, "getMethods", "()[Ljava/lang/reflect/Method;");
    auto methods = (jobjectArray)jvm::call_method<jobject>(_class, getMethods);

    return method_list(_class, methods);
}

method_list clazz::get_constructors()
{
    // Get the java.lang.Class object associated with this class.
    jclass cls = jvm::get_object_class(_class);

    // Get an array of methods
    auto getMethods = jvm::get_method_id(cls, "getConstructors", "()[Ljava/lang/reflect/Constructor;");
    auto methods = (jobjectArray)jvm::call_method<jobject>(_class, getMethods);

    return method_list(_class, methods);
}

#define do_call_static_method(cls, id, return_type, ...) \
    if (return_type == "void") \
    { \
        jvm::call_static_method<void>(_class, id, __VA_ARGS__); \
        return object(); \
    } \
    else if (return_type == "boolean") \
        return object(jvm::call_static_method<jboolean>(_class, id, __VA_ARGS__)); \
    else if (return_type == "byte") \
        return object(jvm::call_static_method<jbyte>(_class, id, __VA_ARGS__)); \
    else if (return_type == "char") \
        return object(jvm::call_static_method<jchar>(_class, id, __VA_ARGS__)); \
    else if (return_type == "double") \
        return object(jvm::call_static_method<jdouble>(_class, id, __VA_ARGS__)); \
    else if (return_type == "float") \
        return object(jvm::call_static_method<jfloat>(_class, id, __VA_ARGS__)); \
    else if (return_type == "int") \
        return object(jvm::call_static_method<jint>(_class, id, __VA_ARGS__)); \
    else if (return_type == "long") \
        return object(jvm::call_static_method<jlong>(_class, id, __VA_ARGS__)); \
    else if (return_type == "short") \
        return object(jvm::call_static_method<jshort>(_class, id, __VA_ARGS__)); \
    else \
        return object(jvm::call_static_method<jobject>(_class, id, __VA_ARGS__));

// These methods call static Java methods on the class given the 
// method name and a number of arguments.  An exception is thrown
// if an appropriate method is not found.  Also, the first method 
// which accepts the given arguments is called, even if there is a 
// "better" match.  For now, if it is necessary to disambiguate, the
// low-level jvm::call_xxx_method() functions must be used.
object clazz::call_static(const char* method_name)
{
    std::vector<clazz> classes;
    auto m = lookup_method(method_name, classes);
    do_call_static_method(_class, m.id(), m.return_type());
}
object clazz::call_static(const char* method_name, object a1)
{
    std::vector<clazz> classes;
    classes.push_back(a1.get_clazz());
    auto m = lookup_method(method_name, classes);
    do_call_static_method(_class, m.id(), m.return_type(), a1.native_object());
}
object clazz::call_static(const char* method_name, object a1, object a2)
{
    std::vector<clazz> classes;
    classes.push_back(a1.get_clazz());
    classes.push_back(a2.get_clazz());
    auto m = lookup_method(method_name, classes);
    do_call_static_method(_class, m.id(), m.return_type(), a1.native_object(), a2.native_object());
}
object clazz::call_static(const char* method_name, object a1, object a2, object a3)
{
    std::vector<clazz> classes;
    classes.push_back(a1.get_clazz());
    classes.push_back(a2.get_clazz());
    classes.push_back(a3.get_clazz());
    auto m = lookup_method(method_name, classes);
    do_call_static_method(_class, m.id(), m.return_type(), a1.native_object(), a2.native_object(), a3.native_object());
}

clazz java::load_class(const char* class_name, jbyte* class_data, jsize size)
{
    auto loader = java::clazz("java/lang/ClassLoader").call_static("getSystemClassLoader");
    jclass cls = java::internal::get_thread_local_vm()->env->DefineClass(class_name, loader.native_object(), class_data, size);
    return clazz(cls);
}
