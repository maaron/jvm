
#include "type_traits.h"

using namespace java::jvm;

jni_type_traits<void>::jni_type jni_type_traits<void>::call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args) { return env->CallVoidMethodV(obj, id, args); }
jni_type_traits<void>::jni_type jni_type_traits<void>::call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args) { return env->CallStaticVoidMethodV(cls, id, args); }

jni_type_traits<jobject>::jni_type jni_type_traits<jobject>::call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args) { return env->CallObjectMethodV(obj, id, args); }
jni_type_traits<jobject>::jni_type jni_type_traits<jobject>::call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args) { return env->CallStaticObjectMethodV(cls, id, args); }
        
jni_type_traits<jboolean>::jni_type jni_type_traits<jboolean>::call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args) { return env->CallBooleanMethodV(obj, id, args); }
jni_type_traits<jboolean>::jni_type jni_type_traits<jboolean>::call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args) { return env->CallStaticBooleanMethodV(cls, id, args); }
jni_type_traits<jboolean>::jni_type* jni_type_traits<jboolean>::get_array_elements(JNIEnv* env, array_type arr, jboolean* copy) { return env->GetBooleanArrayElements(arr, copy); }
void jni_type_traits<jboolean>::release_array_elements(JNIEnv* env, array_type arr, jni_type* ptr, jint mode) { return env->ReleaseBooleanArrayElements(arr, ptr, mode); }

jni_type_traits<jbyte>::jni_type jni_type_traits<jbyte>::call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args) { return env->CallByteMethodV(obj, id, args); }
jni_type_traits<jbyte>::jni_type jni_type_traits<jbyte>::call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args) { return env->CallStaticByteMethodV(cls, id, args); }
jni_type_traits<jbyte>::jni_type* jni_type_traits<jbyte>::get_array_elements(JNIEnv* env, array_type arr, jboolean* copy) { return env->GetByteArrayElements(arr, copy); }
void jni_type_traits<jbyte>::release_array_elements(JNIEnv* env, array_type arr, jni_type* ptr, jint mode) { return env->ReleaseByteArrayElements(arr, ptr, mode); }

jni_type_traits<jchar>::jni_type jni_type_traits<jchar>::call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args) { return env->CallCharMethodV(obj, id, args); }
jni_type_traits<jchar>::jni_type jni_type_traits<jchar>::call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args) { return env->CallStaticCharMethodV(cls, id, args); }
jni_type_traits<jchar>::jni_type* jni_type_traits<jchar>::get_array_elements(JNIEnv* env, array_type arr, jboolean* copy) { return env->GetCharArrayElements(arr, copy); }
void jni_type_traits<jchar>::release_array_elements(JNIEnv* env, array_type arr, jni_type* ptr, jint mode) { return env->ReleaseCharArrayElements(arr, ptr, mode); }
        
jni_type_traits<jshort>::jni_type jni_type_traits<jshort>::call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args) { return env->CallShortMethodV(obj, id, args); }
jni_type_traits<jshort>::jni_type jni_type_traits<jshort>::call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args) { return env->CallStaticShortMethodV(cls, id, args); }
jni_type_traits<jshort>::jni_type* jni_type_traits<jshort>::get_array_elements(JNIEnv* env, array_type arr, jboolean* copy) { return env->GetShortArrayElements(arr, copy); }
void jni_type_traits<jshort>::release_array_elements(JNIEnv* env, array_type arr, jni_type* ptr, jint mode) { return env->ReleaseShortArrayElements(arr, ptr, mode); }
        
jni_type_traits<jint>::jni_type jni_type_traits<jint>::call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args) { return env->CallIntMethodV(obj, id, args); }
jni_type_traits<jint>::jni_type jni_type_traits<jint>::call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args) { return env->CallStaticIntMethodV(cls, id, args); }
jni_type_traits<jint>::jni_type* jni_type_traits<jint>::get_array_elements(JNIEnv* env, array_type arr, jboolean* copy) { return env->GetIntArrayElements(arr, copy); }
void jni_type_traits<jint>::release_array_elements(JNIEnv* env, array_type arr, jni_type* ptr, jint mode) { return env->ReleaseIntArrayElements(arr, ptr, mode); }
        
jni_type_traits<jlong>::jni_type jni_type_traits<jlong>::call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args) { return env->CallLongMethodV(obj, id, args); }
jni_type_traits<jlong>::jni_type jni_type_traits<jlong>::call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args) { return env->CallStaticLongMethodV(cls, id, args); }
jni_type_traits<jlong>::jni_type* jni_type_traits<jlong>::get_array_elements(JNIEnv* env, array_type arr, jboolean* copy) { return env->GetLongArrayElements(arr, copy); }
void jni_type_traits<jlong>::release_array_elements(JNIEnv* env, array_type arr, jni_type* ptr, jint mode) { return env->ReleaseLongArrayElements(arr, ptr, mode); }
        
jni_type_traits<jfloat>::jni_type jni_type_traits<jfloat>::call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args) { return env->CallFloatMethodV(obj, id, args); }
jni_type_traits<jfloat>::jni_type jni_type_traits<jfloat>::call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args) { return env->CallStaticFloatMethodV(cls, id, args); }
jni_type_traits<jfloat>::jni_type* jni_type_traits<jfloat>::get_array_elements(JNIEnv* env, array_type arr, jboolean* copy) { return env->GetFloatArrayElements(arr, copy); }
void jni_type_traits<jfloat>::release_array_elements(JNIEnv* env, array_type arr, jni_type* ptr, jint mode) { return env->ReleaseFloatArrayElements(arr, ptr, mode); }
        
jni_type_traits<jdouble>::jni_type jni_type_traits<jdouble>::call_method(JNIEnv* env, jobject obj, jmethodID id, va_list args) { return env->CallDoubleMethodV(obj, id, args); }
jni_type_traits<jdouble>::jni_type jni_type_traits<jdouble>::call_static_method(JNIEnv* env, jclass cls, jmethodID id, va_list args) { return env->CallStaticDoubleMethodV(cls, id, args); }
jni_type_traits<jdouble>::jni_type* jni_type_traits<jdouble>::get_array_elements(JNIEnv* env, array_type arr, jboolean* copy) { return env->GetDoubleArrayElements(arr, copy); }
void jni_type_traits<jdouble>::release_array_elements(JNIEnv* env, array_type arr, jni_type* ptr, jint mode) { return env->ReleaseDoubleArrayElements(arr, ptr, mode); }
