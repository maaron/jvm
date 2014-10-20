
#include "java\type_traits.h"

using namespace java::jni;

type_traits<void>::jni_type type_traits<void>::call_methodv(JNIEnv* env, jobject obj, jmethodID id, va_list args) { return env->CallVoidMethodV(obj, id, args); }
type_traits<void>::jni_type type_traits<void>::call_static_methodv(JNIEnv* env, jclass cls, jmethodID id, va_list args) { return env->CallStaticVoidMethodV(cls, id, args); }

type_traits<jobject>::jni_type type_traits<jobject>::call_methodv(JNIEnv* env, jobject obj, jmethodID id, va_list args)
{
    auto ret = env->CallObjectMethodV(obj, id, args);
#ifdef DEBUG_REFS
    _refs.push_back(ret);
#endif
    return ret;
}
type_traits<jobject>::jni_type type_traits<jobject>::call_static_methodv(JNIEnv* env, jclass cls, jmethodID id, va_list args)
{
    auto ret = env->CallStaticObjectMethodV(cls, id, args);
#ifdef DEBUG_REFS
    _refs.push_back(ret);
#endif
    return ret;
}

#define def_primitive_type_traits(jtype, cap_name) \
type_traits<jtype>::jni_type type_traits<jtype>::call_methodv(JNIEnv* env, jobject obj, jmethodID id, va_list args) { return env->Call##cap_name##MethodV(obj, id, args); } \
type_traits<jtype>::jni_type type_traits<jtype>::call_static_methodv(JNIEnv* env, jclass cls, jmethodID id, va_list args) { return env->CallStatic##cap_name##MethodV(cls, id, args); } \
type_traits<jtype>::jni_type* type_traits<jtype>::get_array_elements(JNIEnv* env, array_type arr, jboolean* copy) { return env->Get##cap_name##ArrayElements(arr, copy); } \
void type_traits<jtype>::release_array_elements(JNIEnv* env, array_type arr, jni_type* ptr, jint mode) { return env->Release##cap_name##ArrayElements(arr, ptr, mode); } \
type_traits<jtype>::jni_type type_traits<jtype>::get_field(JNIEnv* env, jobject obj, jfieldID id) { return env->Get##cap_name##Field(obj, id); } \
type_traits<jtype>::jni_type type_traits<jtype>::get_static_field(JNIEnv* env, jclass obj, jfieldID id) { return env->GetStatic##cap_name##Field(obj, id); } \
type_traits<jtype>::array_type type_traits<jtype>::new_array(JNIEnv* env, size_t size) { return env->New##cap_name##Array(size); }

def_primitive_type_traits(jboolean, Boolean)
def_primitive_type_traits(jbyte, Byte)
def_primitive_type_traits(jchar, Char)
def_primitive_type_traits(jshort, Short)
def_primitive_type_traits(jint, Int)
def_primitive_type_traits(jlong, Long)
def_primitive_type_traits(jfloat, Float)
def_primitive_type_traits(jdouble, Double)
