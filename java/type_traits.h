#pragma once

#include "jni.h"

namespace java
{

    namespace jvm
    {

        template <typename jtype>
        struct type_traits;

        template <>
        struct type_traits<void>
        {
            typedef void jni_type;

            static jni_type call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args);
            static jni_type call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args);
        };

        template <>
        struct type_traits<jobject>
        {
            typedef jobject jni_type;
            typedef jobjectArray array_type;

            static jni_type call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args);
            static jni_type call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args);
        };

#define decl_primitive_type_traits(jtype) \
        template <> \
        struct type_traits<jtype> \
        { \
            typedef jtype jni_type; \
            typedef jtype##Array array_type; \
            static jni_type call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args); \
            static jni_type call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args); \
            static jni_type* get_array_elements(JNIEnv* env, array_type arr, jboolean* copy); \
            static void release_array_elements(JNIEnv* env, array_type arr, jni_type* ptr, jint mode); \
            static jtype get_field(JNIEnv* env, jobject obj, jfieldID id); \
            static jtype get_static_field(JNIEnv* env, jclass obj, jfieldID id); \
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