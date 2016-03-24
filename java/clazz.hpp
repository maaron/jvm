#include "clazz.h"

#include <vector>
#include <algorithm>

namespace java
{
    clazz::clazz()
    {
    }

    clazz::clazz(const char* name)
		    : object(jni::find_class(name))
    {
    }

	clazz::clazz(jclass cls) : object(cls) {}

    clazz::clazz(java::object obj) : object(obj) {}

    std::string clazz::name()
    {
        clazz cls_cls(jni::get_object_class(_ref.get()));
        auto getName = jni::get_method_id(cls_cls.native(), "getName", "()Ljava/lang/String;");
        local_ref<jstring> name = jni::call_method<jobject>(_ref.get(), getName);
        return jstring_str(name.get());
    }

    object clazz::static_field(const char* name)
    {
        return object(*this).call("getField", name).call("get", object::null());
    }

    method clazz::lookup_method(const char* name, const std::vector<clazz>& classes)
    {
        auto methods = get_methods();
        auto match = std::find_if(methods.begin(), methods.end(), [&](const method& m) -> bool
        {
            bool ret = m.name() == name && m.is_args_assignable(classes);
            return ret;
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
        clazz cls_cls(jni::get_object_class(_ref.get()));

        // Get an array of methods
        auto getMethods = jni::get_method_id(cls_cls.native(), "getMethods", "()[Ljava/lang/reflect/Method;");
        local_ref<jobjectArray> methods = jni::call_method<jobject>(_ref.get(), getMethods);

        return method_list(_ref, methods);
    }

    method_list clazz::get_constructors()
    {
        // Get the java.lang.Class object associated with this class.
        clazz cls_cls(jni::get_object_class(_ref.get()));

        // Get an array of methods
        auto getMethods = jni::get_method_id(cls_cls.native(), "getConstructors", "()[Ljava/lang/reflect/Constructor;");
        local_ref<jobjectArray> methods = jni::call_method<jobject>(_ref.get(), getMethods);

        return method_list(_ref, methods);
    }

    java::object clazz::call_static_method(java::method m, ...)
    {
        va_list args;
        va_start(args, m);

        auto cls = native();
        auto id = m.id();
        auto return_type = m.return_type();

        java::object ret;

        if (return_type == "void")
        {
            jni::call_static_methodv<void>(cls, id, args);
        }
        else if (return_type == "boolean")
            ret = object(jni::call_static_method<jboolean>(cls, id, args));
        else if (return_type == "byte")
            ret = object(jni::call_static_method<jbyte>(cls, id, args));
        else if (return_type == "char")
            ret = object(jni::call_static_method<jchar>(cls, id, args));
        else if (return_type == "double")
            ret = object(jni::call_static_method<jdouble>(cls, id, args));
        else if (return_type == "float")
            ret = object(jni::call_static_method<jfloat>(cls, id, args));
        else if (return_type == "int")
            ret = object(jni::call_static_method<jint>(cls, id, args));
        else if (return_type == "long")
            ret = object(jni::call_static_method<jlong>(cls, id, args));
        else if (return_type == "short")
            ret = object(jni::call_static_method<jshort>(cls, id, args));
        else
            ret = object(jni::call_static_method<jobject>(cls, id, args));

        va_end(args);

        return ret;
    }

#define do_call_static_method(cls, id, return_type, ...) \
    if (return_type == "void") \
    { \
        jni::call_static_method<void>(cls, id, __VA_ARGS__); \
        return object(); \
    } \
    else if (return_type == "boolean") \
        return object(jni::call_static_method<jboolean>(cls, id, __VA_ARGS__)); \
    else if (return_type == "byte") \
        return object(jni::call_static_method<jbyte>(cls, id, __VA_ARGS__)); \
    else if (return_type == "char") \
        return object(jni::call_static_method<jchar>(cls, id, __VA_ARGS__)); \
    else if (return_type == "double") \
        return object(jni::call_static_method<jdouble>(cls, id, __VA_ARGS__)); \
    else if (return_type == "float") \
        return object(jni::call_static_method<jfloat>(cls, id, __VA_ARGS__)); \
    else if (return_type == "int") \
        return object(jni::call_static_method<jint>(cls, id, __VA_ARGS__)); \
    else if (return_type == "long") \
        return object(jni::call_static_method<jlong>(cls, id, __VA_ARGS__)); \
    else if (return_type == "short") \
        return object(jni::call_static_method<jshort>(cls, id, __VA_ARGS__)); \
    else \
        return object(jni::call_static_method<jobject>(cls, id, __VA_ARGS__));

    // These methods call static Java methods on the class given the 
    // method name and a number of arguments.  An exception is thrown
    // if an appropriate method is not found.  Also, the first method 
    // which accepts the given arguments is called, even if there is a 
    // "better" match.  For now, if it is necessary to disambiguate, the
    // low-level jni::call_xxx_method() functions must be used.
    object clazz::call_static(const char* method_name)
    {
        std::vector<clazz> classes;
        auto m = lookup_method(method_name, classes);
        do_call_static_method((jclass)_ref.get(), m.id(), m.return_type());
    }
    object clazz::call_static(const char* method_name, object a1)
    {
        std::vector<clazz> classes;
        classes.push_back(a1.get_clazz());
        auto m = lookup_method(method_name, classes);
        do_call_static_method((jclass)_ref.get(), m.id(), m.return_type(), a1.native());
    }
    object clazz::call_static(const char* method_name, object a1, object a2)
    {
        std::vector<clazz> classes;
        classes.push_back(a1.get_clazz());
        classes.push_back(a2.get_clazz());
        auto m = lookup_method(method_name, classes);
        do_call_static_method((jclass)_ref.get(), m.id(), m.return_type(), a1.native(), a2.native());
    }
    object clazz::call_static(const char* method_name, object a1, object a2, object a3)
    {
        std::vector<clazz> classes;
        classes.push_back(a1.get_clazz());
        classes.push_back(a2.get_clazz());
        classes.push_back(a3.get_clazz());
        auto m = lookup_method(method_name, classes);
        do_call_static_method((jclass)_ref.get(), m.id(), m.return_type(), a1.native(), a2.native(), a3.native());
    }

    object clazz::call_static(const char* method_name, object a1, object a2, object a3, object a4)
    {
        std::vector<clazz> classes;
        classes.push_back(a1.get_clazz());
        classes.push_back(a2.get_clazz());
        classes.push_back(a3.get_clazz());
        classes.push_back(a4.get_clazz());
        auto m = lookup_method(method_name, classes);
        do_call_static_method((jclass)_ref.get(), m.id(), m.return_type(), a1.native(), a2.native(), a3.native(), a4.native());
    }

    java::clazz clazz::from_value(jint arg)
    {
        return java::clazz("java/lang/Integer").static_field("TYPE");
    }

    java::clazz clazz::from_value(java::object& arg)
    {
        return arg.get_clazz();
    }

    jobject clazz::get_native(java::object& value) { return value.native(); }
    jint clazz::get_native(jint value) { return value; }

    clazz java::load_class(const char* class_name, jbyte* class_data, jsize size)
    {
        auto loader = java::clazz("java/lang/ClassLoader").call_static("getSystemClassLoader");
        return clazz(jni::define_class(class_name, loader.native(), class_data, size));
    }
}