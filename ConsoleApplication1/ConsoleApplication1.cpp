// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <java.hpp>
#include <iostream>

int main()
{
	try
	{
		java::load_jvmdll("C:\\Program Files (x86)\\Java\\jdk1.8.0_25\\jre\\bin\\server\\jvm.dll");

		java::vm jvm;

		try
		{
			java::clazz String("java/lang/String");
			auto name = String.call("getName");
			auto name_string = name.as_string();
		}
		catch (java::exception& je)
		{
			std::cout << "Java exception: " << je.to_string() << std::endl;
		}
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

    return 0;
}

