jvm
===

Collection of C++ classes for using JNI.

This is a header-only library that makes (in my opinion) JNI a little easier to
use.  It integrates more with the C++ STL (i.e., uses std::string and
iterators), and utilizes Java reflection for resolving method names instead of
having to specify the signature explicitly.


Platform Support
----------------
Only works on Windows, due to the use of TlsAlloc and friends from the Windows
API.  Also, I've only built the library with Visual Studio 2010 and 2013.
Don't know of any reason why other compilers wouldn't work just as well.


How To Use:
-----------
Here are a list of pre-requisites to get things working:

1. Install a supported JDK- the only potential imcompatibility I've aware of is
   with JVM's supporting a JNI of version less than 1.2.  I've specifically
   tested the library with JDK 6 and 8.

2. Make sure the include and include\win32 folders from the Java Development
   Kit (JDK) are both added to your IDE's #include search path.  These are
   typically located somewhere like C:\Program Files
    (x86)\Java\jdk1.7.0_07\include.

3. Make sure the folder containing the java.h header file from this repository
   is also in your search path.

4. Make sure to link the jvm.lib file (also from the JDK) with your executable.
   It will be found somewhere like, C:\Program Files
    (x86)\Java\jdk1.7.0_07\lib.

5. Make sure the jre\bin\client folder from the JDK (or a JRE) is in the
   executable search path at runtime.  If you have the JDK installed (which you
   must from steps 1 and 3), it will be located somewhere like C:\Program Files
    (x86)\Java\jdk1.7.0_07\jre\bin\client.

6. Make sure the Windows.h header and the C++ STL headers can be found (unless
   you're modifying the library to work with a non-Windows platform).  This is
   usually automatic with most IDE's (certainly the case for Microsoft
   products).

Here's a test program to make sure you've got everything setup correctly:

```cpp
#include "java.h"
#include "java.hpp"
#include <iostream>

int main()
{
    java::vm jvm;

    java::clazz("java/lang/System").static_field("out").call("println", "Hello, World!");

    return 0;
}
```


Almost-Header-Only-Ness
-----------------------
The library is setup to be distributed as a source-only package, and can be
made a header-only library if it is only used in a single translation unit.
The sample above does this by including both java.h and java.hpp.  If you need
it in more than one translation unit (#include'd in multiple cpp files),
include java.hpp in only one translation unit, and just include java.h
everywhere else it's needed.  Why did I do it this way?  I'm a weirdo for
header-only libraries (I hate debugging linker and build configuration issues
more than I should), but the nature of this library doesn't fit well with the
header-only model (there aren't any templates intended for user consumption,
and classes are highly self-referential implying the need to separate method
definitions from their containing class definitions).  Thus, I landed somewhere
inbetween a header-only and a static library distribution.


Thread Local Storage
--------------------
The library uses thread-local storage (TLS) in order to store the current
JNIEnv pointer that is valid for the thread.  I debated with myself about this
a great deal before deciding to go this way.  I suspect I might be technically
limiting the performance potential by requiring every interaction with the JNI
to perform a system call to get at TLS, but the alternative would have required
passing around some object for every call to java::clazz(), java::create(),
etc., in order to specify the JNIEnv pointer.  I opted for the former to lessen
the verbosity, as performance is limited already by the use of reflection for
looking up method and field ID's.  Also, the JNI interface is already
thread-specific, in that object references (or even the JNIEnv pointer) can't
be used across threads.  The library currently catches this situation with a
nice exception throw.


Memory Managment
----------------
The primary memory managment concern when dealing with the JNI is releasing
global/local references.  This library uses a std::shared_ptr-based method of
automatically calling DeleteLocalRef when java::\* objects go out of scope.
For this reason, the JNI's push/pop local frame functions aren't currently
exposed by the library.  That was another route I could have gone, but requires
some work from the user.  Also, global references aren't currently exposed in a
convenient way, which would be needed in order to make JNI references outlive
calls into native code from Java.
