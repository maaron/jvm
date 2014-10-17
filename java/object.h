#pragma once

#include <memory>

namespace java
{
    class method_list;
    class clazz;

    // Enum of the various native types used by JNI.  java_void is used to 
    // represent the "return value" for Java methods that return void.
    enum value_type
    { 
        java_boolean, 
        java_byte, 
        java_char,
        java_short,
        java_int,
        java_long,
        java_float,
        java_double,
        java_object,
        java_void
    };

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

    public:
        // This class is used for updating elements in a Java primitive (int, 
        // short, etc., non-object types) array.  When accessing primitive 
        // arrays using JNI, the caller gets a pointer to memory that may or may 
        // not have been copied.  Thus, the JNI provides a function to free this 
        // memory (if applicable, a no-op otherwise).  The lifetime of objects 
        // of this class control when this release function is called.  
        // Furthermore, this class tracks whether the element is modified, in 
        // order to tell the JNI whether to commit changes.
        class array_element
        {
            local_ref<jarray> _ref;
            void* _ptr;
            bool _modified;
            value_type _type;
            size_t _index;

        public:
            array_element(object arr, size_t index);

            ~array_element();

            array_element& operator= (const object& rhs);

            operator object();
        };

        // Turns the local object reference into a global reference, 
        // allowing it to be used across different threads or native method 
        // contexts.  This does nothing if the java::object does not contain
        // a Java reference type (i.e., no-op for primitive types).
        void make_global();

        // This returns an object that serves as a "null reference" in Java
        static object null() { return object((jobject)nullptr); }

        // These constructors create new Java objects of the various types.
        object()
            : _type(java_void)
        {
            _value.l = nullptr;
        }
        object(jobject native) 
            : _type(java_object), _ref(native)
        {
            _value.l = native;
        }
        object(jboolean native) 
            : _type(java_boolean)
        {
            _value.z = native;
        }
        object(jbyte native) 
            : _type(java_byte)
        {
            _value.b = native;
        }
        object(jchar native) 
            : _type(java_char)
        {
            _value.c = native;
        }
        object(jdouble native) 
            : _type(java_double)
        {
            _value.d = native;
        }
        object(jfloat native) 
            : _type(java_float)
        {
            _value.f = native;
        }
        object(jint native) 
            : _type(java_int)
        {
            _value.i = native;
        }
        object(jlong native) 
            : _type(java_long)
        {
            _value.j = native;
        }
        object(jshort native) 
            : _type(java_short)
        {
            _value.s = native;
        }

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
            if (_type != java_boolean) throw std::exception("Java object is not a boolean");
            return _value.z == JNI_TRUE;
        }
        jbyte as_byte() const
        {
            if (_type != java_byte) throw std::exception("Java object is not a byte");
            return _value.b;
        }
        jchar as_char() const
        {
            if (_type != java_char) throw std::exception("Java object is not a char");
            return _value.c;
        }
        jshort as_short() const
        {
            if (_type != java_short) throw std::exception("Java object is not a short");
            return _value.s;
        }
        jint as_int() const
        {
            if (_type != java_int) throw std::exception("Java object is not a int");
            return _value.i;
        }
        jlong as_long() const
        {
            if (_type != java_long) throw std::exception("Java object is not a long");
            return _value.j;
        }
        jfloat as_float() const
        {
            if (_type != java_float) throw std::exception("Java object is not a float");
            return _value.f;
        }
        jdouble as_double() const
        {
            if (_type != java_double) throw std::exception("Java object is not a double");
            return _value.d;
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

    // Creates a new object in the VM by calling it's parameterless c-tor.  
    // Effectively does the same as "new class_name()" in Java.
    object create(const char* class_name);

    object create(const char* class_name, object a1);

    // Creates a new object (non-primitive) array
    object create_array(const char* class_name, size_t size, object initial);
}
