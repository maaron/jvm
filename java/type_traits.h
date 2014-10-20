#pragma once

#include "jni.h"

namespace java
{

    namespace jni
    {
        // Enum of the various native types used by JNI.  java_void is used to 
        // represent the "return value" for Java methods that return void.
        enum value_type
        {
            jboolean_value,
            jbyte_value,
            jchar_value,
            jshort_value,
            jint_value,
            jlong_value,
            jfloat_value,
            jdouble_value,
            jobject_value,
            void_value
        };

        template <typename jtype>
        struct type_traits;

        template <>
        struct type_traits<void>
        {
            typedef void jni_type;
            const value_type value = void_value;

            static jni_type call_methodv(JNIEnv* env, jobject obj, jmethodID id, va_list args);
            static jni_type call_static_methodv(JNIEnv* env, jclass cls, jmethodID id, va_list args);
        };

        template <>
        struct type_traits<jobject>
        {
            typedef jobject jni_type;
            typedef jobjectArray array_type;
            const value_type value = jobject_value;

            static jni_type call_methodv(JNIEnv* env, jobject obj, jmethodID id, va_list args);
            static jni_type call_static_methodv(JNIEnv* env, jclass cls, jmethodID id, va_list args);
        };

#define decl_primitive_type_traits(jtype) \
        template <> \
        struct type_traits<jtype> \
        { \
            typedef jtype jni_type; \
            typedef jtype##Array array_type; \
            const value_type value = jtype##_value; \
            static jni_type call_methodv(JNIEnv* env, jobject obj, jmethodID id, va_list args); \
            static jni_type call_static_methodv(JNIEnv* env, jclass cls, jmethodID id, va_list args); \
            static jni_type* get_array_elements(JNIEnv* env, array_type arr, jboolean* copy); \
            static void release_array_elements(JNIEnv* env, array_type arr, jni_type* ptr, jint mode); \
            static jtype get_field(JNIEnv* env, jobject obj, jfieldID id); \
            static jtype get_static_field(JNIEnv* env, jclass obj, jfieldID id); \
            static array_type new_array(JNIEnv* env, size_t length); \
        }

        decl_primitive_type_traits(jboolean);
        decl_primitive_type_traits(jbyte);
        decl_primitive_type_traits(jchar);
        decl_primitive_type_traits(jshort);
        decl_primitive_type_traits(jint);
        decl_primitive_type_traits(jlong);
        decl_primitive_type_traits(jfloat);
        decl_primitive_type_traits(jdouble);

    }

}