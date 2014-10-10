
#include "java\exception.h"
#include "java\jvm.h"
#include "java\object.h"

using namespace java;

void java_exception::suspend()
{
    internal::get_thread_local_vm()->env->ExceptionClear();
}

void java_exception::resume()
{
    //internal::get_thread_local_vm()->env->Throw(_t);
}

std::string java_exception::message()
{
    suspend();
    object t(_t);

    std::string ret;
    ret = t.call("getMessage");
    resume();
    return ret;
}


void java_exception::print()
{
    suspend();
    object t(_t);

    std::string ret;
    t.call("printStackTrace");
    resume();
}
