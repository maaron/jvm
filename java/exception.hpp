
#include "java\exception.h"
#include "java\jvm.h"
#include "java\object.h"

namespace java
{

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
        internal::get_env()->ExceptionClear();
    }

    void exception::resume()
    {
        internal::get_env()->Throw(_t);
    }

    void exception::print()
    {
        suspend();
        object t(_t);

        std::string ret;
        t.call("printStackTrace");
        resume();
    }

}