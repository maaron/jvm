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
        jmethodID _id;
        local_ref<jobject> _methodObj;

    public:
        method();
        
        method(local_ref<jobject> methodObj);

        // Returns the native JVM jmethodID for this method
        jmethodID id() { return _id; }

        // Returns the name of the method as a string
        std::string name() const;

        // Returns the number of arguments the method accepts
        jsize num_args();

        // Checks whether the supplied classes can be assigned to the 
        // arguments for this method.  The size of the classes array must 
        // match the number of arguments, and each class must either be null 
        // (always considered assignable to anything) or otherwise 
        // assignable to the corresponding argument type.
        bool is_args_assignable(const std::vector<clazz>& classes) const;

        // Returns the Java type that this method returns as a string.  The 
        // contents of the string match what the java.lang.Class.getName() 
        // method returns.
        std::string return_type();
    };

    // This class is used to iterate methods of a Java class by wrapping an 
    // index into a Java java.lang.reflect.Method[] array.
    class method_iterator : public std::iterator<std::forward_iterator_tag, method>
    {
        local_ref<jclass> _class;
        local_ref<jobjectArray> _methods;
        jsize _index;
        method _current;

    public:
        method_iterator(local_ref<jclass> cls, local_ref<jobjectArray> methods, jsize i)
            : _class(cls), _methods(methods), _index(i), _current()
        {
        }

        bool operator== (const method_iterator& rhs)
        {
            return _methods == rhs._methods && _index == rhs._index;
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
        local_ref<jclass> _class;
        local_ref<jobjectArray> _methods;
        jsize _size;

    public:
        method_list(local_ref<jclass> cls, local_ref<jobjectArray> methods);

        method_iterator begin();

        method_iterator end();
    };

}