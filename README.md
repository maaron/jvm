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
API.  Also, I've only attempted using Visual Studio 2010 on Windows 7.  Don't
know of any reason why other compilers wouldn't work just as well.


How To Use:
-----------
Here are a list of pre-requisites to get things working:

1. Install a supported JDK- I've only tested with JDK 6, but maybe others work
   as well.

1. Make sure the include and include\win32 folders from the Java Development
   Kit (JDK) are both added to your IDE's #include search path.  These are
   typically located somewhere like C:\Program Files
    (x86)\Java\jdk1.7.0_07\include.

2. Make sure the folder containing the java.h header file from this repository
   is also in your search path.

3. Make sure to link the jvm.lib file (also from the JDK) with your executable.
   It will be found somewhere like, C:\Program Files
    (x86)\Java\jdk1.7.0_07\lib.

4. Make sure the jre\bin\client folder from the JDK (or a JRE) is in the
   executable search path at runtime.  If you have the JDK installed (which you
   must from steps 1 and 3), it will be located somewhere like C:\Program Files
    (x86)\Java\jdk1.7.0_07\jre\bin\client.

Here's a test program to make sure you've got everything setup correctly:

```cpp
#include "java.h"
#include <iostream>

int main()
{
    java::vm jvm;

    java::clazz("java/lang/System").static_field("out").call("println", "Hello, World!");

    return 0;
}
```

