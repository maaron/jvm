#pragma once

#include "jni.h"

namespace java
{

    namespace jvm
    {

        template <typename jtype>
        struct jni_type_traits;

        template <>
        struct jni_type_traits<void>
        {
            typedef void jni_type;

            static jni_type call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args);
            static jni_type call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args);
        };

        template <>
        struct jni_type_traits<jobject>
        {
            typedef jobject jni_type;
            typedef jobjectArray array_type;

            static jni_type call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args);
            static jni_type call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args);
        };
        
        template <>
        struct jni_type_traits<jboolean>
        {
            typedef jboolean jni_type;
            typedef jbooleanArray array_type;

            static jni_type call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args);
            static jni_type call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args);
            static jni_type* get_array_elements(JNIEnv* env, array_type arr, jboolean* copy);
            static void release_array_elements(JNIEnv* env, array_type arr, jni_type* ptr, jint mode);
        };

        template <>
        struct jni_type_traits<jbyte>
        {
            typedef jbyte jni_type;
            typedef jbyteArray array_type;
        
            static jni_type call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args);
            static jni_type call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args);
            static jni_type* get_array_elements(JNIEnv* env, array_type arr, jboolean* copy);
            static void release_array_elements(JNIEnv* env, array_type arr, jni_type* ptr, jint mode);
        };

        template <>
        struct jni_type_traits<jchar>
        {
            typedef jchar jni_type;
            typedef jcharArray array_type;
        
            static jni_type call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args);
            static jni_type call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args);
            static jni_type* get_array_elements(JNIEnv* env, array_type arr, jboolean* copy);
            static void release_array_elements(JNIEnv* env, array_type arr, jni_type* ptr, jint mode);
        };

        template <>
        struct jni_type_traits<jshort>
        {
            typedef jshort jni_type;
            typedef jshortArray array_type;
        
            static jni_type call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args);
            static jni_type call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args);
            static jni_type* get_array_elements(JNIEnv* env, array_type arr, jboolean* copy);
            static void release_array_elements(JNIEnv* env, array_type arr, jni_type* ptr, jint mode);
        };

        template <>
        struct jni_type_traits<jint>
        {
            typedef jint jni_type;
            typedef jintArray array_type;
        
            static jni_type call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args);
            static jni_type call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args);
            static jni_type* get_array_elements(JNIEnv* env, array_type arr, jboolean* copy);
            static void release_array_elements(JNIEnv* env, array_type arr, jni_type* ptr, jint mode);
        };

        template <>
        struct jni_type_traits<jlong>
        {
            typedef jlong jni_type;
            typedef jlongArray array_type;
        
            static jni_type call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args);
            static jni_type call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args);
            static jni_type* get_array_elements(JNIEnv* env, array_type arr, jboolean* copy);
            static void release_array_elements(JNIEnv* env, array_type arr, jni_type* ptr, jint mode);
        };

        template <>
        struct jni_type_traits<jfloat>
        {
            typedef jfloat jni_type;
            typedef jfloatArray array_type;
        
            static jni_type call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args);
            static jni_type call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args);
            static jni_type* get_array_elements(JNIEnv* env, array_type arr, jboolean* copy);
            static void release_array_elements(JNIEnv* env, array_type arr, jni_type* ptr, jint mode);
        };

        template <>
        struct jni_type_traits<jdouble>
        {
            typedef jdouble jni_type;
            typedef jdoubleArray array_type;
        
            static jni_type call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args);
            static jni_type call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args);
            static jni_type* get_array_elements(JNIEnv* env, array_type arr, jboolean* copy);
            static void release_array_elements(JNIEnv* env, array_type arr, jni_type* ptr, jint mode);
        };

    }

}