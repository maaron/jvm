
#include "java\object.h"
#include "java\jvm.h"
#include "java\clazz.h"

using namespace java;

object::object(const char* str)
    : _type(java_object)
{
    _ref = jni::new_string_utf(str);
    _value.l = _ref.get();
}

object::object(const clazz& cls)
    : _type(java_object)
{
    _ref = cls.ref();
    _value.l = _ref.get();
}

clazz object::get_clazz()
{
    switch (_type)
    {
    case java_boolean: return jni::find_class("java/lang/Boolean");
    case java_byte: return jni::find_class("java/lang/Byte");
    case java_char: return jni::find_class("java/lang/Character");
    case java_short: return jni::find_class("java/lang/Short");
    case java_int: return jni::find_class("java/lang/Integer");
    case java_long: return jni::find_class("java/lang/Long");
    case java_float: return jni::find_class("java/lang/Float");
    case java_double: return jni::find_class("java/lang/Double");
    case java_object: return _value.l == nullptr ? clazz() : clazz(jni::get_object_class(_value.l));
    default:
        throw std::exception("Unsupported Java type");
    }
}

jsize object::array_size()
{
    if (_type != java_object) throw std::exception("Not an object type");
    return jni::get_array_length((jarray)_value.l);
}

object::array_element object::operator[](size_t index)
{
    if (_type != java_object) throw std::exception("Not an object type");
    return array_element(*this, index);
}

std::string object::to_string()
{
    auto toString = jni::get_method_id(get_clazz().native(), "toString", "()Ljava/lang/String;");
    local_ref<jstring> jstr(jni::call_method<jobject>(_value.l, toString));
    return jstring_str(jstr.get());
}

object object::field(const char* name)
{
    return get_clazz().call_static("getField", name).call("get", *this);
}

object call_method(jobject obj, std::string& return_type, jmethodID id, ...)
{
    va_list args;
    va_start(args, id);

    object ret;

    try
    {
        auto rtype = return_type;
        if (return_type == "void")
        {
            jni::call_methodv<void>(obj, id, args);
            ret = object();
        }
        else if (rtype == "boolean")
            ret = object(jni::call_methodv<jboolean>(obj, id, args));
        else if (rtype == "byte")
            ret = object(jni::call_methodv<jbyte>(obj, id, args));
        else if (rtype == "char")
            ret = object(jni::call_methodv<jchar>(obj, id, args));
        else if (rtype == "double")
            ret = object(jni::call_methodv<jdouble>(obj, id, args));
        else if (rtype == "float")
            ret = object(jni::call_methodv<jfloat>(obj, id, args));
        else if (rtype == "int")
            ret = object(jni::call_methodv<jint>(obj, id, args));
        else if (rtype == "long")
            ret = object(jni::call_methodv<jlong>(obj, id, args));
        else if (rtype == "short")
            ret = object(jni::call_methodv<jshort>(obj, id, args));
        else
            ret = object(jni::call_methodv<jobject>(obj, id, args));
    }
    catch (...)
    {
        va_end(args);
        throw;
    }

    va_end(args);

    return ret;
}

// These functions call Java methods on the object given the method 
// name and a number of arguments.  An exception is thrown if an 
// appropriate method is not found.
object object::call(const char* method_name)
{
    std::vector<clazz> classes;
    auto m = get_clazz().lookup_method(method_name, classes);
    return call_method(_value.l, m.return_type(), m.id());
}
object object::call(const char* method_name, object a1)
{
    std::vector<clazz> classes;
    classes.push_back(a1.get_clazz());
    auto m = get_clazz().lookup_method(method_name, classes);
    return call_method(_value.l, m.return_type(), m.id(), a1.native());
}
object object::call(const char* method_name, object a1, object a2)
{
    std::vector<clazz> classes;
    classes.push_back(a1.get_clazz());
    classes.push_back(a2.get_clazz());
    auto m = get_clazz().lookup_method(method_name, classes);
    return call_method(_value.l, m.return_type(), m.id(), a1.native(), a2.native());
}
object object::call(const char* method_name, object a1, object a2, object a3)
{
    std::vector<clazz> classes;
    classes.push_back(a1.get_clazz());
    classes.push_back(a2.get_clazz());
    classes.push_back(a3.get_clazz());
    auto m = get_clazz().lookup_method(method_name, classes);
    return call_method(_value.l, m.return_type(), m.id(), a1.native(), a2.native(), a3.native());
}

#undef call_method

object::array_element::array_element(object arr, size_t index)
    : _ref(arr.ref()), _modified(false), _index(index)
{
    auto class_name = arr.get_clazz().name();
    if (class_name.size() < 2 || class_name[0] != '[') throw std::exception("Not an array type");

    switch (class_name[1])
    {
    case 'Z': _type = java_boolean; _ptr = jni::get_array_elements<jboolean>((jbooleanArray)_ref.get(), nullptr); break;
    case 'B': _type = java_byte; _ptr = jni::get_array_elements<jbyte>((jbyteArray)_ref.get(), nullptr); break;
    case 'C': _type = java_char; _ptr = jni::get_array_elements<jchar>((jcharArray)_ref.get(), nullptr); break;
    case 'L': _type = java_object; _ptr = nullptr; break;
    case 'D': _type = java_double; _ptr = jni::get_array_elements<jdouble>((jdoubleArray)_ref.get(), nullptr); break;
    case 'F': _type = java_float; _ptr = jni::get_array_elements<jfloat>((jfloatArray)_ref.get(), nullptr); break;
    case 'I': _type = java_int; _ptr = jni::get_array_elements<jint>((jintArray)_ref.get(), nullptr); break;
    case 'J': _type = java_long; _ptr = jni::get_array_elements<jlong>((jlongArray)_ref.get(), nullptr); break;
    case 'S': _type = java_short; _ptr = jni::get_array_elements<jshort>((jshortArray)_ref.get(), nullptr); break;
    default:
        throw std::exception("Unsupported Java type");
    }
}

object::array_element::~array_element()
{
    int mode = _modified ? JNI_COMMIT : JNI_ABORT;

    switch (_type)
    {
    case java_boolean: jni::release_array_elements<jboolean>((jbooleanArray)_ref.get(), (jboolean*)_ptr, mode); break;
    case java_byte: jni::release_array_elements<jbyte>((jbyteArray)_ref.get(), (jbyte*)_ptr, mode); break;
    case java_char: jni::release_array_elements<jchar>((jcharArray)_ref.get(), (jchar*)_ptr, mode); break;
    case java_object: break;
    case java_float: jni::release_array_elements<jfloat>((jfloatArray)_ref.get(), (jfloat*)_ptr, mode); break;
    case java_double: jni::release_array_elements<jdouble>((jdoubleArray)_ref.get(), (jdouble*)_ptr, mode); break;
    case java_int: jni::release_array_elements<jint>((jintArray)_ref.get(), (jint*)_ptr, mode); break;
    case java_long: jni::release_array_elements<jlong>((jlongArray)_ref.get(), (jlong*)_ptr, mode); break;
    case java_short: jni::release_array_elements<jshort>((jshortArray)_ref.get(), (jshort*)_ptr, mode); break;
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
    case java_object: jni::set_object_array_element((jobjectArray)_ref.get(), _index, rhs.native()); break;
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
    case java_object: return jni::get_object_array_element((jobjectArray)_ref.get(), _index); break;
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
        return jni::new_object(cls.native(), ctor.id());
    }

    object create(const char* class_name, object a1)
    {
        clazz cls(class_name);
        std::vector<clazz> classes;
        classes.push_back(a1.get_clazz());
        auto ctor = cls.lookup_constructor(classes);
        return jni::new_object(cls.native(), ctor.id(), a1.native());
    }

    object create(const char* class_name, object a1, object a2)
    {
        clazz cls(class_name);
        std::vector<clazz> classes;
        classes.push_back(a1.get_clazz());
        classes.push_back(a2.get_clazz());
        auto ctor = cls.lookup_constructor(classes);
        return jni::new_object(cls.native(), ctor.id(), a1.native(), a2.native());
    }

    object create(const char* class_name, object a1, object a2, object a3)
    {
        clazz cls(class_name);
        std::vector<clazz> classes;
        classes.push_back(a1.get_clazz());
        classes.push_back(a2.get_clazz());
        classes.push_back(a3.get_clazz());
        auto ctor = cls.lookup_constructor(classes);
        return jni::new_object(cls.native(), ctor.id(), a1.native(), a2.native(), a3.native());
    }

    // Creates a new object (non-primitive) array
    object create_array(const char* class_name, size_t size, object initial)
    {
        clazz cls(class_name);
        return object(jni::new_object_array(cls.native(), size, initial.native()));
    }

}
