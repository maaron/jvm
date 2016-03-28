
#include "exception.h"
#include "jvm.h"
#include "object.h"

namespace java
{
    exception::exception(jthrowable t)
        : _t(t), _msg(""), object(t)
    {
        suspend();
        auto msg = call("getMessage");
		_msg = msg.is_null() ? "(null)" : msg.as_string();
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