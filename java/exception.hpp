
#include "exception.h"
#include "jvm.h"
#include "object.h"

using namespace java;

exception::exception(jthrowable t)
    : _t(t), _msg(""), object(t)
{
    suspend();
    _msg = call("getMessage");
    resume();
}

const char* exception::what() const
{
    return _msg.c_str();
}

void exception::suspend()
{
    internal::get_thread_local_vm()->env->ExceptionClear();
}

void exception::resume()
{
    internal::get_thread_local_vm()->env->Throw(_t);
}

void exception::print()
{
    suspend();
    object t(_t);

    std::string ret;
    t.call("printStackTrace");
    resume();
}
