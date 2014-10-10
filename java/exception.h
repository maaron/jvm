#pragma once

#include "jni.h"
#include <string>

namespace java
{

    // This class exposes exceptions in the JVM as C++ exceptions.
    class java_exception : public std::exception
    {
        jthrowable _t;
        std::string _msg;

    public:
        java_exception(jthrowable t) : _t(t)
        {
        }

        jthrowable throwable() { return _t; }

        // This function clears the exception (and any other exception) that 
        // is currently being thrown in the JVM.  This is useful in order to 
        // call other functions in the java::* namespace without them 
        // throwing further java_exceptions.  If it is desired to reinstate
        // the exception in the JVM, call resume().
        void suspend();

        // Rethrows the exception within the Java VM. This is useful in cases 
        // where the caller wants the JVM to handle the exception- perhaps in 
        // some Java code.  Note that this doesn't actually throw a C++ 
        // exception.  It just "undoes" the ExceptionClear() call made in the 
        // constructor.
        void resume();

        // Returns the message associated with the java.lang.Throwable.  This can
        // fail if there is an active exception in the JVM.  This isn't normally
        // the case when catching java_exception's, though, since the
        // java_exception class clears the exception from the JVM.
        std::string message();

        // Prints the stack trace to stderr
        void print();
    };

}