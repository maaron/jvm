
#include "jvm.h"
#include "exception.h"

namespace java
{

    namespace internal
    {
        static DWORD tlsIndex = TLS_OUT_OF_INDEXES;

        DWORD get_tls_index()
        {
            // Allocate the TLS slot, if it hasn't been done already.  This 
            // should happen only once, and all threads use the same slot.
            if (tlsIndex == TLS_OUT_OF_INDEXES &&
                (tlsIndex = ::TlsAlloc()) == TLS_OUT_OF_INDEXES)
            {
                throw std::exception("TlsAlloc failed");
            }

            return tlsIndex;
        }

        void free_tls_index()
        {
            auto index = get_tls_index();

            if (::TlsFree(index) == FALSE)
                throw std::exception("TlsFree failed");

            tlsIndex = TLS_OUT_OF_INDEXES;
        }

        LPVOID get_tls_value()
        {
            LPVOID value = ::TlsGetValue(get_tls_index());
            if (value == 0 && ::GetLastError() != ERROR_SUCCESS)
                throw std::exception("TlsGetValue failed");
            return value;
        }

        void set_tls_value(LPVOID value)
        {
            if (::TlsSetValue(get_tls_index(), value) == FALSE)
            {
                auto error = ::GetLastError();
                throw std::exception("TlsGetValue failed");
            }
        }

        JNIEnv* get_env()
        {
            LPVOID value = get_tls_value();
            if (value == 0)
            {
                throw std::exception("Thread not attached to the JVM");
            }

            return (JNIEnv*)value;
        }

    }

    JNI_CreateJavaVM_type p_JNI_CreateJavaVM = nullptr;

    void load_jvmdll(const char* path)
    {
        auto module = ::LoadLibraryA(path);
        if (module == NULL) throw std::exception("Failed to load jvm.dll");

        p_JNI_CreateJavaVM = (JNI_CreateJavaVM_type)::GetProcAddress(module, "JNI_CreateJavaVM");
        if (p_JNI_CreateJavaVM == nullptr) throw std::exception("Failed to initialize JVM library");
    }

    namespace jni
    {
        jclass define_class(const char* name, jobject loader, jbyte* data, jsize size)
        {
            auto env = internal::get_env();
            jclass ret = env->DefineClass(name, loader, data, size);
            if (env->ExceptionOccurred()) throw java::exception(env->ExceptionOccurred());
            return ret;
        }

#ifdef DEBUG_REFS
        std::list<jobject> _refs;
#endif
        void delete_local_ref(jobject obj)
        {
            auto env = internal::get_env();
            env->DeleteLocalRef(obj);
#ifdef DEBUG_REFS
            _refs.remove(obj);
#endif
        }

        void delete_global_ref(jobject obj)
        {
            auto env = internal::get_env();
            env->DeleteGlobalRef(obj);
#ifdef DEBUG_REFS
            _refs.remove(obj);
#endif
        }

        template <>
        void call_static_method<void>(jclass cls, jmethodID method, ...)
        {
            auto env = internal::get_env();
            va_list args;
            va_start(args, method);
            type_traits<void>::call_static_methodv(env, cls, method, args);
            if (env->ExceptionOccurred()) throw exception(env->ExceptionOccurred());
            va_end(args);
        }
        
        template <>
        void call_static_methodv<void>(jclass cls, jmethodID method, va_list args)
        {
            auto env = internal::get_env();
            type_traits<void>::call_static_methodv(env, cls, method, args);
            if (env->ExceptionOccurred()) throw exception(env->ExceptionOccurred());
        }

        template <>
        void call_method<void>(jobject obj, jmethodID method, ...)
        {
            auto env = internal::get_env();
            va_list args;
            va_start(args, method);
            type_traits<void>::call_methodv(env, obj, method, args);
            if (env->ExceptionCheck()) throw exception(env->ExceptionOccurred());
            va_end(args);
        }

        template <>
        void call_methodv<void>(jobject obj, jmethodID method, va_list args)
        {
            auto env = internal::get_env();
            type_traits<void>::call_methodv(env, obj, method, args);
            if (env->ExceptionCheck()) throw exception(env->ExceptionOccurred());
        }

        jclass find_class(const char* name)
        {
            jclass cls = internal::get_env()->FindClass(name);
            if (cls == nullptr) throw std::exception("FindClass failed");
#ifdef DEBUG_REFS
            _refs.push_back(cls);
#endif
            return cls;
        }

        jclass get_object_class(jobject obj)
        {
            jclass cls = internal::get_env()->GetObjectClass(obj);
            if (cls == nullptr) throw std::exception("GetObjectClass failed");
#ifdef DEBUG_REFS
            _refs.push_back(cls);
#endif            
            return cls;
        }

        jmethodID get_method_id(jclass clazz, const char* name, const char* sig)
        {
            jmethodID method = internal::get_env()->GetMethodID(clazz, name, sig);
            if (method == nullptr) throw std::exception("GetMethodID failed");
            return method;
        }

        jmethodID get_static_method_id(jclass clazz, const char* name, const char* sig)
        {
            jmethodID method = internal::get_env()->GetStaticMethodID(clazz, name, sig);
            if (method == nullptr) throw std::exception("GetMethodID failed");
            return method;
        }

        jsize get_array_length(jarray a)
        {
            return internal::get_env()->GetArrayLength(a);
        }

        jobject get_object_array_element(jobjectArray a, jsize i)
        {
            auto obj = internal::get_env()->GetObjectArrayElement(a, i);
            if (obj == nullptr) throw std::exception("GetObjectArrayElement failed");
#ifdef DEBUG_REFS
            _refs.push_back(obj);
#endif
            return obj;
        }

        void set_object_array_element(jobjectArray a, jsize i, jobject value)
        {
            auto env = internal::get_env();
            env->SetObjectArrayElement(a, i, value);
            if (env->ExceptionCheck()) throw exception(env->ExceptionOccurred());
        }

        jstring new_string_utf(const char* data)
        {
            jstring jstr = internal::get_env()->NewStringUTF(data);
            if (jstr == nullptr) throw std::exception("NewStringUTF failed");
#ifdef DEBUG_REFS
            _refs.push_back(jstr);
#endif
            return jstr;
        }

        const char* get_string_utf_chars(jstring jstr)
        {
            auto data = internal::get_env()->GetStringUTFChars(jstr, nullptr);
            if (data == nullptr) throw std::exception("GetStringUTFChars failed");
            return data;
        }

        void release_string_utf_chars(jstring jstr, const char* data)
        {
            internal::get_env()->ReleaseStringUTFChars(jstr, data);
        }

        jobject allocate_object(jclass cls)
        {
            jobject obj = internal::get_env()->AllocObject(cls);
            if (obj == nullptr) throw std::exception("AllocObject failed");
#ifdef DEBUG_REFS
            _refs.push_back(obj);
#endif
            return obj;
        }

        jobject new_object(jclass cls, jmethodID ctor, ...)
        {
            va_list args;
            va_start(args, ctor);
            auto env = internal::get_env();
            auto obj = env->NewObjectV(cls, ctor, args);
            va_end(args);
            if (env->ExceptionCheck()) throw exception(env->ExceptionOccurred());
#ifdef DEBUG_REFS
            _refs.push_back(obj);
#endif
            return obj;
        }

        jobjectArray new_object_array(jclass cls, jsize length, jobject initial)
        {
            auto env = internal::get_env();
            auto ret = env->NewObjectArray(length, cls, initial);
            if (env->ExceptionCheck()) throw exception(env->ExceptionOccurred());
#ifdef DEBUG_REFS
            _refs.push_back(ret);
#endif
            return ret;
        }

        bool is_assignable_from(jclass src, jclass target)
        {
            return internal::get_env()->IsAssignableFrom(src, target) == JNI_TRUE;
        }

        jmethodID from_reflected_method(jobject methodObj)
        {
            auto method = internal::get_env()->FromReflectedMethod(methodObj);
            if (method == nullptr) throw std::exception("FromReflectedMethod failed");
            return method;
        }

        jobject to_reflected_method(jclass cls, jmethodID id, jboolean is_static)
        {
            auto method = internal::get_env()->ToReflectedMethod(cls, id, is_static);
            if (method == nullptr) throw std::exception("ToReflectedMethod failed");
#ifdef DEBUG_REFS
            _refs.push_back(method);
#endif
            return method;
        }

        // This function converts a Java jstring into a std::string
        std::string jstring_str(jstring jstr)
        {
            auto data = jni::get_string_utf_chars(jstr);
            std::string ret(data);
            jni::release_string_utf_chars(jstr, data);
            return ret;
        }

    }

}