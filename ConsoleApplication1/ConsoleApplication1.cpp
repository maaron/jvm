// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <java.hpp>
#include <iostream>
#include <functional>

int main()
{
	try
	{
		java::load_jvmdll("C:\\Program Files (x86)\\Java\\jdk1.8.0_25\\jre\\bin\\server\\jvm.dll");

		java::vm_args args;
		//args.add_option("-Djava.class.path=C:/Users/maaron/Source/Repos/jvm/ConsoleApplication1/foo.jar");
		//args.add_option("-verbose");
		args.ignore_unrecognized(false);
		java::vm jvm(args);

		try
		{	
			java::clazz comparableIface("java/lang/Comparable");

			auto proxy = java::create_proxy(comparableIface, [](java::method m, java::object args) -> java::object
			{
				std::cout << "Proxy handler called for method " << m.name() << std::endl;
				return 123L;
			});

			auto compare_result = proxy.call("compareTo", java::create("java/lang/Object"));
			std::cout << "compareTo result is " << compare_result.as_int() << std::endl;
		}
		catch (java::exception& je)
		{
			je.suspend();
			std::cout << "Java exception: " << je.to_string() << std::endl;
			je.print();
			je.resume();
		}
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

    return 0;
}

