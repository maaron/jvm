#pragma once

#include "..\java.h"
#include <functional>

namespace java
{
	typedef std::function<object(method, object)> invocation_handler_func;

	object create_proxy(clazz iface, invocation_handler_func handler);
}
