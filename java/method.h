#pragma once

#include "jvm.h"
#include <vector>

namespace java
{
    class clazz;

    // This class encapsulates a Java method, either static or non-static.  
    // Reflection is used to gather method details from the JVM.
    class method
    {
        jclass _class;
        jmethodID _id;
        jobject _methodObj;

    public:
        method();
        
        method(jclass cls, jobject methodObj);

        // Returns the native JVM jmethodID for this method
        jmethodID id() { return _id; }

        // Returns the name of the method as a string
        std::string name() const;

        // Returns the number of arguments the method accepts
        jsize num_args();

        bool is_args_assignable(const std::vector<clazz>& classes) const;
        /*
        // These functions are used to determine whether the specified 
        // classes can be used to call this method.  They check that the 
        // method accepts the correct number of arguments and that each class 
        // is assignable to the corresponding type that the method accepts.
        bool matches_args()
        {
            auto method_class = jvm::get_object_class(_methodObj);
            auto getParameterTypes = jvm::get_method_id(method_class, "getParameterTypes", "()[Ljava/lang/Class;");
            auto parameter_types = (jobjectArray)jvm::call_method<jobject>(_methodObj, getParameterTypes);
            return jvm::get_array_length(parameter_types) == 0;
        }
        bool matches_args(jclass a1)
        {
            auto method_class = jvm::get_object_class(_methodObj);
            auto getParameterTypes = jvm::get_method_id(method_class, "getParameterTypes", "()[Ljava/lang/Class;");
            auto parameter_types = (jobjectArray)jvm::call_method<jobject>(_methodObj, getParameterTypes);
            if (jvm::get_array_length(parameter_types) != 1) return false;

            return jvm::is_assignable_from(a1, (jclass)jvm::get_object_array_element(parameter_types, 0));
        }
        bool matches_args(jclass a1, jclass a2)
        {
            auto method_class = jvm::get_object_class(_methodObj);
            auto getParameterTypes = jvm::get_method_id(method_class, "getParameterTypes", "()[Ljava/lang/Class;");
            auto parameter_types = (jobjectArray)jvm::call_method<jobject>(_methodObj, getParameterTypes);
            if (jvm::get_array_length(parameter_types) != 2) return false;

            return 
                jvm::is_assignable_from(a1, (jclass)jvm::get_object_array_element(parameter_types, 0)) &&
                jvm::is_assignable_from(a2, (jclass)jvm::get_object_array_element(parameter_types, 1));
        }
        bool matches_args(jclass a1, jclass a2, jclass a3)
        {
            auto method_class = jvm::get_object_class(_methodObj);
            auto getParameterTypes = jvm::get_method_id(method_class, "getParameterTypes", "()[Ljava/lang/Class;");
            auto parameter_types = (jobjectArray)jvm::call_method<jobject>(_methodObj, getParameterTypes);
            if (jvm::get_array_length(parameter_types) != 3) return false;

            return 
                jvm::is_assignable_from(a1, (jclass)jvm::get_object_array_element(parameter_types, 0)) &&
                jvm::is_assignable_from(a2, (jclass)jvm::get_object_array_element(parameter_types, 1)) &&
                jvm::is_assignable_from(a3, (jclass)jvm::get_object_array_element(parameter_types, 2));
        }
        */

        // Returns the Java type that this method returns as a string.  The 
        // contents of the string match what the java.lang.Class.getName() 
        // method returns.
        std::string return_type();
    };

    // This class is used to iterate methods of a Java class by wrapping an 
    // index into a Java java.lang.reflect.Method[] array.
    class method_iterator : public std::iterator<std::forward_iterator_tag, method>
    {
        jclass _class;
        jobjectArray _array;
        jsize _index;
        method _current;

    public:
        method_iterator(jclass cls, jobjectArray a, jsize i)
            : _class(cls), _array(a), _index(i), _current()
        {
        }

        bool operator== (const method_iterator& rhs)
        {
            return _array == rhs._array && _index == rhs._index;
        }

        bool operator!= (const method_iterator& rhs) { return !this->operator==(rhs); }

        method_iterator operator++ ()
        {
            advance();
            return *this;
        }

        method_iterator operator++ (int)
        {
            method_iterator tmp(*this);
            advance();
            return tmp;
        }

        method& operator* ()
        {
            get();
            return _current;
        }

        method* operator-> ()
        {
            get();
            return &_current;
        }

    private:
        void get();

        void advance()
        {
            _index++;
        }
    };

    // This class encapsulates a list of methods for a class, obtained via 
    // reflection by wrapping a java.lang.reflect.Method[] array.
    class method_list
    {
        jclass _class;
        jobjectArray _array;
        jsize _size;

    public:
        method_list(jclass cls, jobjectArray a);

        method_iterator begin();

        method_iterator end();
    };

}