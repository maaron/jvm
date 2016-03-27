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
		args.add_option("-Djava.class.path=C:/Users/maaron/Source/Repos/jvm/ConsoleApplication1/foo.jar");
		//args.add_option("-verbose");
		args.ignore_unrecognized(false);
		java::vm jvm(args);

		try
		{
#if 0
			nih.make_global();
			nih.call("getConstructors");

			auto ctors = nih.get_constructors();
			for (auto ctor : ctors)
			{
				std::cout << "ctor name: " << ctor.name() << std::endl;
			}
			
			auto methods = nih.get_methods();
			for (auto m : methods)
			{
				std::cout << "method name: " << m.name() << std::endl;
			}
#endif		
			java::clazz comparableIface("java/lang/Comparable");

			auto proxy = java::create_proxy(comparableIface, [](java::method m, java::object args) -> java::object
			{
				std::cout << "Proxy handler called for method " << m.name() << std::endl;
				return 123L;
			});

			auto compare_result = proxy.call("compareTo", java::create("java/lang/Object"));
			std::cout << "compareTo result is " << compare_result.as_int() << std::endl;

			auto a = java::create("foo/A");
			std::vector<java::clazz> classes;
			classes.push_back(java::object("asdf").get_clazz());

			auto i_class = java::clazz("foo/I");
			
			auto a_method = a.get_clazz().lookup_method("m", classes);
			auto i_method = i_class.lookup_method("m", classes);

			auto i_result = java::jni::call_method<jint>(a.native(), i_method.id(), java::object("asdf").native());
			auto a_result = java::jni::call_method<jint>(a.native(), a_method.id(), java::object("asdf").native());

			auto result = a.call("m", "asdf").as_int();
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

