#pragma once

#include <exception>
#include <string>
#include <sstream>
#include "..\java.h"

namespace java
{
	class nosuchmethod_exception : public std::exception
	{
		std::string _message;

		std::string join_arg_types(const std::vector<clazz>& types)
		{
			if (types.size() == 0) return "";
			else if (types.size() == 1) return " taking an argument of " + types[0].name();
			else
			{
				std::ostringstream stream;
				stream << " taking arguments of";
				auto it = types.begin();
				for (; it != types.end() - 2; it++)
					stream << ' ' << it->name() << ',';

				stream << ' ' << (it++)->name();
				stream << " and " << (it++)->name();
				return stream.str();
			}
		}

	public:
		nosuchmethod_exception(clazz c, const char* methodName, const std::vector<clazz>& classes)
			: _message("No such method '" + std::string(methodName) 
				+ "' found in class '" + c.name() + "'" 
				+ join_arg_types(classes))
		{
		}
	};
}