
#include "object.h"
#include "jvm.h"
#include "clazz.h"

namespace java
{
    object::object(const char* str)
        : _type(jobject_value)
    {
        _ref = jni::new_string_utf(str);
        _value.l = _ref.get();
    }

    object::object(const clazz& cls)
        : _type(jobject_value)
    {
        _ref = cls.ref();
        _value.l = _ref.get();
    }

    void object::make_global()
    {
        if (_type == jobject_value)
        {
            _ref.make_global();
            _value.l = _ref.get();
        }
    }

	clazz object::get_clazz() const
    {
        switch (_type)
        {
        case jboolean_value: return java::clazz("java/lang/Boolean").static_field("TYPE");
        case jbyte_value: return java::clazz("java/lang/Byte").static_field("TYPE");
        case jchar_value: return java::clazz("java/lang/Character").static_field("TYPE");
        case jshort_value: return java::clazz("java/lang/Short").static_field("TYPE");
        case jint_value: return java::clazz("java/lang/Integer").static_field("TYPE");
        case jlong_value: return java::clazz("java/lang/Long").static_field("TYPE");
        case jfloat_value: return java::clazz("java/lang/Float").static_field("TYPE");
        case jdouble_value: return java::clazz("java/lang/Double").static_field("TYPE");
        case jobject_value: 
            if (_value.l == nullptr) throw std::exception("Null object reference");
            return clazz(jni::get_object_class(native()));
		    case void_value: throw std::exception("value is void");

        default:
            throw std::exception("Unsupported Java type");
        }
    }

    bool object::is_void() const
    {
        return (_type == void_value);
    }

    bool object::is_null() const 
    {
        return (_type == jobject_value && _value.l == nullptr);
    }

    bool object::is_ref() const
    {
        return _type == jobject_value;
    }

    bool object::is_bool() const { return _type == jboolean_value; }
    bool object::as_bool() const
	{
		if (_type != jboolean_value) throw std::exception("Java object is not a boolean");
		return _value.z == JNI_TRUE;
	}

    bool object::is_byte() const { return _type == jbyte_value; }
	jbyte object::as_byte() const
	{
		if (_type != jbyte_value) throw std::exception("Java object is not a byte");
		return _value.b;
	}

    bool object::is_char() const { return _type == jchar_value; }
	jchar object::as_char() const
	{
		if (_type != jchar_value) throw std::exception("Java object is not a char");
		return _value.c;
	}

    bool object::is_short() const { return _type == jshort_value; }
	jshort object::as_short() const
	{
		if (_type != jshort_value) throw std::exception("Java object is not a short");
		return _value.s;
	}

    bool object::is_int() const { return _type == jint_value; }
	jint object::as_int() const
	{
		if (_type != jint_value) throw std::exception("Java object is not a int");
		return _value.i;
	}

    bool object::is_long() const { return _type == jlong_value; }
	jlong object::as_long() const
	{
		if (_type != jlong_value) throw std::exception("Java object is not a long");
		return _value.j;
	}

    bool object::is_float() const { return _type == jfloat_value; }
	jfloat object::as_float() const
	{
		if (_type != jfloat_value) throw std::exception("Java object is not a float");
		return _value.f;
	}

    bool object::is_double() const { return _type == jdouble_value; }
	jdouble object::as_double() const
	{
		if (_type != jdouble_value) throw std::exception("Java object is not a double");
		return _value.d;
	}

    bool object::is_string() const
    {
        auto cls = get_clazz();
        clazz stringCls("java/lang/String");
        return cls.call("equals", stringCls).as_bool();
    }
	std::string object::as_string() const
	{
		if (!is_string()) throw std::exception("Java object is not a String");
		return jstring_str(reinterpret_cast<jstring>(native()));
	}

    bool object::is_array() const
    {
        if (is_null()) return false;

        auto name = get_clazz().name();
        return name.size() > 1 && name[0] == '[';
    }

    jsize object::array_size()
    {
        if (_type != jobject_value) throw std::exception("Not an object type");
        return jni::get_array_length((jarray)native());
    }

    array_element object::operator[](size_t index)
    {
        auto class_name = get_clazz().name();
        if (class_name.size() < 2 || class_name[0] != '[') throw std::exception("Not an array type");

        switch (class_name[1])
        {
        case 'Z': return array_element(*this, get_element<jboolean>(index), index); break;
        case 'B': return array_element(*this, get_element<jbyte>(index), index); break;
        case 'C': return array_element(*this, get_element<jchar>(index), index); break;
        case 'L': return array_element(*this, jni::get_object_array_element((jobjectArray)_ref.get(), index), index); break;
        case 'D': return array_element(*this, get_element<jdouble>(index), index); break;
        case 'F': return array_element(*this, get_element<jfloat>(index), index); break;
        case 'I': return array_element(*this, get_element<jint>(index), index); break;
        case 'J': return array_element(*this, get_element<jlong>(index), index); break;
        case 'S': return array_element(*this, get_element<jshort>(index), index); break;
        default:
            throw std::exception("Unsupported Java type");
        }
    }

    std::string object::to_string() const
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
    object object::call(const char* method_name, object a1, object a2, object a3, object a4)
    {
        std::vector<clazz> classes;
        classes.push_back(a1.get_clazz());
        classes.push_back(a2.get_clazz());
        classes.push_back(a3.get_clazz());
        classes.push_back(a4.get_clazz());
        auto m = get_clazz().lookup_method(method_name, classes);
        return call_method(_value.l, m.return_type(), m.id(), a1.native(), a2.native(), a3.native(), a4.native());
    }
    object object::call(const char* method_name, object a1, object a2, object a3, object a4, object a5)
    {
        std::vector<clazz> classes;
        classes.push_back(a1.get_clazz());
        classes.push_back(a2.get_clazz());
        classes.push_back(a3.get_clazz());
        classes.push_back(a4.get_clazz());
        classes.push_back(a5.get_clazz());
        auto m = get_clazz().lookup_method(method_name, classes);
        return call_method(_value.l, m.return_type(), m.id(), a1.native(), a2.native(), a3.native(), a4.native(), a5.native());
    }
    object object::call(const char* method_name, object a1, object a2, object a3, object a4, object a5, object a6)
    {
        std::vector<clazz> classes;
        classes.push_back(a1.get_clazz());
        classes.push_back(a2.get_clazz());
        classes.push_back(a3.get_clazz());
        classes.push_back(a4.get_clazz());
        classes.push_back(a5.get_clazz());
        classes.push_back(a6.get_clazz());
        auto m = get_clazz().lookup_method(method_name, classes);
        return call_method(_value.l, m.return_type(), m.id(), a1.native(), a2.native(), a3.native(), a4.native(), a5.native(), a6.native());
    }
    object object::call(const char* method_name, object a1, object a2, object a3, object a4, object a5, object a6, object a7)
    {
        std::vector<clazz> classes;
        classes.push_back(a1.get_clazz());
        classes.push_back(a2.get_clazz());
        classes.push_back(a3.get_clazz());
        classes.push_back(a4.get_clazz());
        classes.push_back(a5.get_clazz());
        classes.push_back(a6.get_clazz());
        classes.push_back(a7.get_clazz());
        auto m = get_clazz().lookup_method(method_name, classes);
        return call_method(_value.l, m.return_type(), m.id(), a1.native(), a2.native(), a3.native(), a4.native(), a5.native(), a6.native(), a7.native());
    }
    object object::call(const char* method_name, object a1, object a2, object a3, object a4, object a5, object a6, object a7, object a8)
    {
        std::vector<clazz> classes;
        classes.push_back(a1.get_clazz());
        classes.push_back(a2.get_clazz());
        classes.push_back(a3.get_clazz());
        classes.push_back(a4.get_clazz());
        classes.push_back(a5.get_clazz());
        classes.push_back(a6.get_clazz());
        classes.push_back(a7.get_clazz());
        classes.push_back(a8.get_clazz());
        auto m = get_clazz().lookup_method(method_name, classes);
        return call_method(_value.l, m.return_type(), m.id(), a1.native(), a2.native(), a3.native(), a4.native(), a5.native(), a6.native(), a7.native(), a8.native());
    }

    array_element& array_element::operator= (const object& rhs)
    {
        switch (_type)
        {
        case jboolean_value: set<jboolean>(rhs.as_bool()); break;
        case jbyte_value: set(rhs.as_byte()); break;
        case jchar_value: set(rhs.as_char()); break;
        case jobject_value: set(rhs.native()); break;
        case jfloat_value: set(rhs.as_float()); break;
        case jdouble_value: set(rhs.as_double()); break;
        case jint_value: set(rhs.as_int()); break;
        case jlong_value: set(rhs.as_long()); break;
        case jshort_value: set(rhs.as_short()); break;
        default:
            throw std::exception("Unsupported Java type");
        }

        return *this;
    }

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
