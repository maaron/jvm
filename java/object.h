#pragma once

#include <memory>

namespace java
{
    using namespace jni;
    class method_list;
    class clazz;
    class array_element;

    // Generic container for any Java object.  This class is the fundamental 
    // type of the java::* namespace, and serves as a variant type that can 
    // contain any Java object- reference types, primitives and even a "void" 
    // type for Java methods that return void.
    class object
    {
    protected:
        value_type _type;
        jvalue _value;
        local_ref<jobject> _ref;

    private:
        template <typename jtype>
        jtype get_element(size_t index)
        {
            auto jobj = (typename jni::type_traits<jtype>::array_type)_ref.get();
            auto ptr = jni::get_array_elements<jtype>(jobj, nullptr);
            auto ret = ptr[index];
            jni::release_array_elements<jtype>(jobj, ptr, JNI_ABORT);
            return ret;
        }

    public:
        // Turns the local object reference into a global reference, 
        // allowing it to be used across different threads or native method 
        // contexts.  This does nothing if the java::object does not contain
        // a Java reference type (i.e., no-op for primitive types).
        void make_global();

        // This returns an object that serves as a "null reference" in Java
        static object null() { return object((jobject)nullptr); }

        // These constructors create new Java objects of the various types.
        object() : _type(void_value) { _value.l = nullptr; }
        object(jobject native) : _type(jobject_value), _ref(native) { _value.l = native; }
        object(jboolean native) : _type(jboolean_value) { _value.z = native; }
        object(jbyte native) : _type(jbyte_value) { _value.b = native; }
        object(jchar native) : _type(jchar_value) { _value.c = native; }
        object(jdouble native) : _type(jdouble_value) { _value.d = native; }
        object(jfloat native) : _type(jfloat_value) { _value.f = native; }
        object(jint native) : _type(jint_value) { _value.i = native; }
        object(jlong native) : _type(jlong_value) { _value.j = native; }
        object(jshort native) : _type(jshort_value) { _value.s = native; }

        // Returns the clazz object associated with the current object.
        clazz get_clazz();
        
        // This constructor is a convenience for passing strings to Java 
        // methods.  It is the same as the jobject constructor, but allocates 
        // a java.lang.String.
        object(const char* str);

        // This is a convenience for creating java.lang.Class Java objects 
        // from java::clazz C++ objects.
        object(const clazz&);

        // Looks up a non-static field given the name.  Throws an exception 
        // if a field with the given name is not found.
        object field(const char* name);

        // Wrapper around an object's java.lang.Object.toString() method.
        std::string to_string();

        // Convenience operator overload for the to_string() method.
        operator std::string()
        {
            return to_string();
        }

        bool as_bool() const
        {
            if (_type != jboolean_value) throw std::exception("Java object is not a boolean");
            return _value.z == JNI_TRUE;
        }
        jbyte as_byte() const
        {
            if (_type != jbyte_value) throw std::exception("Java object is not a byte");
            return _value.b;
        }
        jchar as_char() const
        {
            if (_type != jchar_value) throw std::exception("Java object is not a char");
            return _value.c;
        }
        jshort as_short() const
        {
            if (_type != jshort_value) throw std::exception("Java object is not a short");
            return _value.s;
        }
        jint as_int() const
        {
            if (_type != jint_value) throw std::exception("Java object is not a int");
            return _value.i;
        }
        jlong as_long() const
        {
            if (_type != jlong_value) throw std::exception("Java object is not a long");
            return _value.j;
        }
        jfloat as_float() const
        {
            if (_type != jfloat_value) throw std::exception("Java object is not a float");
            return _value.f;
        }
        jdouble as_double() const
        {
            if (_type != jdouble_value) throw std::exception("Java object is not a double");
            return _value.d;
        }
        std::string as_string() const
        {
            if (_type != jobject_value) throw std::exception("Java object is not a java.lang.String");
            return jstring_str((jstring)native());
        }

        local_ref<jobject> ref() { return _ref; }
        
        // Returns the JVM native jobject associated with this object.
        jobject native() const { return _value.l; }

        // Returns true if the object is a null reference.  Note that this 
        // will also return true for Java integers that happen to be zero.
        bool is_null() { return _value.l == nullptr; }

        // Returns the size of the array, if this object contains a Java 
        // array.  Otherwise, an exception is thrown.
        jsize array_size();

        array_element operator[](size_t index);

        // Returns a list of methods this object supports.  See 
        // java.lang.Class.getMethods() method for more information.
        //method_list get_methods();

        // These functions call Java methods on the object given the method 
        // name and a number of arguments.  An exception is thrown if an 
        // appropriate method is not found.
        object call(const char* method_name);
        object call(const char* method_name, object a1);
        object call(const char* method_name, object a1, object a2);
        object call(const char* method_name, object a1, object a2, object a3);
    };

    // This class is used for updating elements in a Java primitive (int, 
    // short, etc., non-object types) array.  When accessing primitive 
    // arrays using JNI, the caller gets a pointer to memory that may or may 
    // not have been copied.  Thus, the JNI provides a function to free this 
    // memory (if applicable, a no-op otherwise).  The lifetime of objects 
    // of this class control when this release function is called.  
    // Furthermore, this class tracks whether the element is modified, in 
    // order to tell the JNI whether to commit changes.
    class array_element : public object
    {
        local_ref<jarray> _array;
        size_t _index;

    private:
        template <typename jtype>
        void set(jtype elem)
        {
            auto jobj = (typename jni::type_traits<jtype>::array_type)_array.get();
            auto ptr = jni::get_array_elements<jtype>(jobj, nullptr);
            ptr[_index] = elem;
            jni::release_array_elements<jtype>(jobj, ptr, JNI_COMMIT);
        }

        template <>
        void set<jobject>(jobject elem)
        {
            jni::set_object_array_element((jobjectArray)_array.get(), _index, elem);
        }

    public:
        template <typename jtype>
        array_element(object arr, jtype elem, size_t index)
            : object(elem), _array(arr.ref()), _index(index) {}

        // Modifies the element at the current index
        array_element& operator= (const object& rhs);
    };

    // Creates a new object in the VM by calling it's parameterless c-tor.  
    // Effectively does the same as "new class_name()" in Java.
    object create(const char* class_name);

    object create(const char* class_name, object a1);

    // Creates a new object (non-primitive) array.  This function is not 
    // suitable for creating primitive arrays, although it may be tempting 
    // to pass something like "java/lang/Integer" as the first parameter.  
    // Use create_array<jint>(size) instead.
    object create_array(const char* class_name, size_t size, object initial);

    // Creates a new primitive array.  The type parameter specifies the JNI 
    // type- jint, jboolean, etc.
    template <typename jtype>
    object create_array(size_t size)
    {
        return object(jni::new_array<jtype>(size));
    }
}
