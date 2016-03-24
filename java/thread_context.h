#pragma once

#include "clazz.h"

namespace java
{
	class thread_context
	{
	private:
		JNIEnv* _env;
		
		clazz jstring_class;
		clazz jint_class;
		clazz class_class;

	public:
		thread_context(JNIEnv* env)
			: _env(env) {}

		clazz& get_jstring_class();
		clazz& get_jint_class();
		clazz& get_class_class();
	};
}