
#include "java\object.h"
#include "java\jvm.h"
#include "java\clazz.h"

using namespace java;

object::object(const char* str)
    : _type(java_object)
{
    _value.l = jvm::new_string_utf(str);
}

object::object(const clazz& cls)
    : _type(java_object)
{
    _value.l = cls.native();
}

clazz object::get_clazz()
{
    return clazz(native_class());
}

jclass object::native_class()
{
    switch (_type)
    {
    case java_boolean: return jvm::find_class("java/lang/Boolean");
    case java_byte: return jvm::find_class("java/lang/Byte");
    case java_char: return jvm::find_class("java/lang/Character");
    case java_short: return jvm::find_class("java/lang/Short");
    case java_int: return jvm::find_class("java/lang/Integer");
    case java_long: return jvm::find_class("java/lang/Long");
    case java_float: return jvm::find_class("java/lang/Float");
    case java_double: return jvm::find_class("java/lang/Double");
    case java_object: return jvm::get_object_class(_value.l);
    default:
        throw std::exception("Unsupported Java type");
    }
}

jsize object::array_size()
{
    if (_type != java_object) throw std::exception("Not an object type");
    return jvm::get_array_length((jarray)_value.l);
}

object::array_element object::operator[](size_t index)
{
    if (_type != java_object) throw std::exception("Not an object type");
    return array_element(_value.l, index);
}

std::string object::to_string()
{
    auto toString = jvm::get_method_id(jvm::find_class("java/lang/Object"), "toString", "()Ljava/lang/String;");
    return jstring_str((jstring)jvm::call_method<jobject>(_value.l, toString));
}
/*
method_list object::get_methods()
{
    // Get the java.lang.Class object associated with this class.
    jclass cls = jvm::get_object_class(native_object());

    // Get an array of methods
    auto getMethods = jvm::get_method_id(cls, "getMethods", "()[Ljava/lang/reflect/Method;");
    auto methods = (jobjectArray)jvm::call_method<jobject>(native_class(), getMethods);

    return method_list(native_class(), methods);
}
*/
method_list get_methods();

#define call_method(obj, id, return_type, ...) \
{ \
    if (return_type == "void") \
    { \
        jvm::call_method<void>(obj, id, __VA_ARGS__); \
        return object(); \
    } \
    else if (return_type == "boolean") \
        return object(jvm::call_method<jboolean>(obj, id, __VA_ARGS__)); \
    else if (return_type == "byte") \
        return object(jvm::call_method<jbyte>(obj, id, __VA_ARGS__)); \
    else if (return_type == "char") \
        return object(jvm::call_method<jchar>(obj, id, __VA_ARGS__)); \
    else if (return_type == "double") \
        return object(jvm::call_method<jdouble>(obj, id, __VA_ARGS__)); \
    else if (return_type == "float") \
        return object(jvm::call_method<jfloat>(obj, id, __VA_ARGS__)); \
    else if (return_type == "int") \
        return object(jvm::call_method<jint>(obj, id, __VA_ARGS__)); \
    else if (return_type == "long") \
        return object(jvm::call_method<jlong>(obj, id, __VA_ARGS__)); \
    else if (return_type == "short") \
        return object(jvm::call_method<jshort>(obj, id, __VA_ARGS__)); \
    else \
        return object(jvm::call_method<jobject>(obj, id, __VA_ARGS__)); \
}

// These functions call Java methods on the object given the method 
// name and a number of arguments.  An exception is thrown if an 
// appropriate method is not found.
object object::call(const char* method_name)
{
    std::vector<clazz> classes;
    auto m = get_clazz().lookup_method(method_name, classes);
    call_method(_value.l, m.id(), m.return_type());
}
object object::call(const char* method_name, object a1)
{
    std::vector<clazz> classes;
    classes.push_back(a1.get_clazz());
    auto m = get_clazz().lookup_method(method_name, classes);
    call_method(_value.l, m.id(), m.return_type(), a1.native_object());
}
object object::call(const char* method_name, object a1, object a2)
{
    std::vector<clazz> classes;
    classes.push_back(a1.get_clazz());
    classes.push_back(a2.get_clazz());
    auto m = get_clazz().lookup_method(method_name, classes);
    call_method(_value.l, m.id(), m.return_type(), a1.native_object(), a2.native_object());
}
object object::call(const char* method_name, object a1, object a2, object a3)
{
    std::vector<clazz> classes;
    classes.push_back(a1.get_clazz());
    classes.push_back(a2.get_clazz());
    classes.push_back(a3.get_clazz());
    auto m = get_clazz().lookup_method(method_name, classes);
    call_method(_value.l, m.id(), m.return_type(), a1.native_object(), a2.native_object(), a3.native_object());
}

#undef call_method

object::array_element::array_element(jobject arr, size_t index)
    : _array((jarray)arr), _modified(false), _index(index)
{
    auto class_name = clazz(jvm::get_object_class(arr)).name();
    if (class_name.size() < 2 || class_name[0] != '[') throw std::exception("Not an array type");

    switch (class_name[1])
    {
    case 'Z': _type = java_boolean; _ptr = jvm::get_array_elements<jboolean>((jbooleanArray)_array, nullptr); break;
    case 'B': _type = java_byte; _ptr = jvm::get_array_elements<jbyte>((jbyteArray)_array, nullptr); break;
    case 'C': _type = java_char; _ptr = jvm::get_array_elements<jchar>((jcharArray)_array, nullptr); break;
    case 'L': _type = java_object; _ptr = nullptr; break;
    case 'D': _type = java_double; _ptr = jvm::get_array_elements<jdouble>((jdoubleArray)_array, nullptr); break;
    case 'F': _type = java_float; _ptr = jvm::get_array_elements<jfloat>((jfloatArray)_array, nullptr); break;
    case 'I': _type = java_int; _ptr = jvm::get_array_elements<jint>((jintArray)_array, nullptr); break;
    case 'J': _type = java_long; _ptr = jvm::get_array_elements<jlong>((jlongArray)_array, nullptr); break;
    case 'S': _type = java_short; _ptr = jvm::get_array_elements<jshort>((jshortArray)_array, nullptr); break;
    default:
        throw std::exception("Unsupported Java type");
    }
}

object::array_element::~array_element()
{
    int mode = _modified ? JNI_COMMIT : JNI_ABORT;

    switch (_type)
    {
    case java_boolean: jvm::release_array_elements<jboolean>((jbooleanArray)_array, (jboolean*)_ptr, mode); break;
    case java_byte: jvm::release_array_elements<jbyte>((jbyteArray)_array, (jbyte*)_ptr, mode); break;
    case java_char: jvm::release_array_elements<jchar>((jcharArray)_array, (jchar*)_ptr, mode); break;
    case java_object: break;
    case java_float: jvm::release_array_elements<jfloat>((jfloatArray)_array, (jfloat*)_ptr, mode); break;
    case java_double: jvm::release_array_elements<jdouble>((jdoubleArray)_array, (jdouble*)_ptr, mode); break;
    case java_int: jvm::release_array_elements<jint>((jintArray)_array, (jint*)_ptr, mode); break;
    case java_long: jvm::release_array_elements<jlong>((jlongArray)_array, (jlong*)_ptr, mode); break;
    case java_short: jvm::release_array_elements<jshort>((jshortArray)_array, (jshort*)_ptr, mode); break;
    default:
        throw std::exception("Unsupported Java type");
    }
}

object::array_element& object::array_element::operator= (const object& rhs)
{
    _modified = true;
    switch (_type)
    {
    case java_boolean: ((jboolean*)_ptr)[_index] = rhs.as_bool(); break;
    case java_byte: ((jbyte*)_ptr)[_index] = rhs.as_byte(); break;
    case java_char: ((jchar*)_ptr)[_index] = rhs.as_char(); break;
    case java_object: jvm::set_object_array_element((jobjectArray)_array, _index, rhs.native_object()); break;
    case java_float: ((jfloat*)_ptr)[_index] = rhs.as_float(); break;
    case java_double: ((jdouble*)_ptr)[_index] = rhs.as_double(); break;
    case java_int: ((jint*)_ptr)[_index] = rhs.as_int(); break;
    case java_long: ((jlong*)_ptr)[_index] = rhs.as_long(); break;
    case java_short: ((jshort*)_ptr)[_index] = rhs.as_short(); break;
    default:
        throw std::exception("Unsupported Java type");
    }

    return *this;
}

object::array_element::operator object()
{
    _modified = true;
    switch (_type)
    {
    case java_boolean: return object(((jboolean*)_ptr)[_index]); break;
    case java_byte: return object(((jbyte*)_ptr)[_index]); break;
    case java_char: return object(((jchar*)_ptr)[_index]); break;
    case java_object: return jvm::get_object_array_element((jobjectArray)_array, _index); break;
    case java_float: return object(((jfloat*)_ptr)[_index]); break;
    case java_double: return object(((jdouble*)_ptr)[_index]); break;
    case java_int: return object(((jint*)_ptr)[_index]); break;
    case java_long: return object(((jlong*)_ptr)[_index]); break;
    case java_short: return object(((jshort*)_ptr)[_index]); break;
    default:
        throw std::exception("Unsupported Java type");
    }
}

namespace java
{

    object create(const char* class_name)
    {
        clazz cls(class_name);
        std::vector<clazz> classes;
        auto ctor = cls.lookup_constructor(classes);
        return jvm::new_object(cls.native(), ctor.id());
    }

    object create(const char* class_name, object a1)
    {
        clazz cls(class_name);
        std::vector<clazz> classes;
        classes.push_back(a1.get_clazz());
        auto ctor = cls.lookup_constructor(classes);
        return jvm::new_object(cls.native(), ctor.id(), a1.native_object());
    }

    object create(const char* class_name, object a1, object a2)
    {
        clazz cls(class_name);
        std::vector<clazz> classes;
        classes.push_back(a1.get_clazz());
        classes.push_back(a2.get_clazz());
        auto ctor = cls.lookup_constructor(classes);
        return jvm::new_object(cls.native(), ctor.id(), a1.native_object(), a2.native_object());
    }

    object create(const char* class_name, object a1, object a2, object a3)
    {
        clazz cls(class_name);
        std::vector<clazz> classes;
        classes.push_back(a1.get_clazz());
        classes.push_back(a2.get_clazz());
        classes.push_back(a3.get_clazz());
        auto ctor = cls.lookup_constructor(classes);
        return jvm::new_object(cls.native(), ctor.id(), a1.native_object(), a2.native_object(), a3.native_object());
    }

    // Creates a new object (non-primitive) array
    object create_array(const char* class_name, size_t size, object initial)
    {
        auto cls = jvm::find_class(class_name);
        return object(jvm::new_object_array(cls, size, initial.native_object()));
    }

}
