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
        clazz get_clazz() const;
        
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

        bool is_void() const;

        bool is_null() const;
        bool is_ref() const;

        bool is_bool() const;
        bool as_bool() const;
		
        bool is_byte() const;
        jbyte as_byte() const;
		
        bool is_char() const;
        jchar as_char() const;
		
        bool is_short() const;
        jshort as_short() const;
		
        bool is_int() const;
        jint as_int() const;
		
        bool is_long() const;
        jlong as_long() const;
		
        bool is_float() const;
        jfloat as_float() const;
		
        bool is_double() const;
        jdouble as_double() const;
		
        bool is_string() const;
        std::string as_string() const;

        bool is_array() const;

        // Don't confuse this with as_string().  This calls the 
        // Object.toString() method, whereas as_string() assumes the object 
        // is itself a java.lang.String.
        std::string to_string() const;

        local_ref<jobject> ref() { return _ref; }
        
        // Returns the JVM native jobject associated with this object.
        jobject native() const { return _value.l; }

        // Returns true if the object is a null reference.  Note that this 
        // will also return true for Java integers that happen to be zero.
        bool is_null() { return _value.l == nullptr; }

        // Returns the size of the array, if this object contains a Java 
        // array.  Otherwise, an exception is thrown.
        jsize array_size();

        // Returns a proxy object that can be used to set or get a value at a 
        // particular index.  If the object is not an array type, an exception 
        // is thrown.
        array_element operator[](size_t index);

		// This returns a boxed object if the current value is a primitive type.  
		// Otherwise returns a copy of the current object (i.e., a new reference to 
		// the same java object).
		object box() const;

        // These functions call Java methods on the object given the method 
        // name and a number of arguments.  An exception is thrown if an 
        // appropriate method is not found.
        object call(const char* method_name);
        object call(const char* method_name, object a1);
        object call(const char* method_name, object a1, object a2);
        object call(const char* method_name, object a1, object a2, object a3);
        object call(const char* method_name, object a1, object a2, object a3, object a4);
        object call(const char* method_name, object a1, object a2, object a3, object a4, object a5);
        object call(const char* method_name, object a1, object a2, object a3, object a4, object a5, object a6);
        object call(const char* method_name, object a1, object a2, object a3, object a4, object a5, object a6, object a7);
        object call(const char* method_name, object a1, object a2, object a3, object a4, object a5, object a6, object a7, object a8);
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
