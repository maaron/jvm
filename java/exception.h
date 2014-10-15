#pragma once

#include "object.h"
#include <string>

namespace java
{

    // This class exposes Java exceptions (java.lang.Throwable) in the JVM 
    // as C++ exceptions.
    class exception 
        : public std::exception,
        public object
    {
        jthrowable _t;
        std::string _msg;

    public:
        exception(jthrowable t);

        // This function clears the exception (and any other exception) that 
        // is currently being thrown in the JVM.  This is useful in order to 
        // call other functions in the java::* namespace without them 
        // throwing further java::exception's.  If it is desired to reinstate
        // the exception in the JVM, call resume().
        void suspend();

        // Rethrows the exception within the Java VM. This is useful in cases 
        // where the caller wants the JVM to handle the exception- perhaps in 
        // some Java code.  Note that this doesn't actually throw a C++ 
        // exception.  It just "undoes" the suspend() call.
        void resume();

        // Prints the stack trace to stderr
        void print();

        // Returns a C string equivalent of what 
        // java.lang.Throwable.getMessage() returns.
        const char* what() const override;
    };

}