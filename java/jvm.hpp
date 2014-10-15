
#include "java\jvm.h"

namespace java
{

    namespace internal
    {

        DWORD get_tls_index()
        {
            static DWORD tlsIndex = TLS_OUT_OF_INDEXES;

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
        }

        jvm_info* get_thread_local_vm()
        {
            DWORD tlsIndex = get_tls_index();

            LPVOID value = ::TlsGetValue(tlsIndex);
            if (value == 0)
            {
                throw std::exception(::GetLastError() == ERROR_SUCCESS ?
                    "Java VM not initialized for the current thread" :
                    "TlsGetValue failed");
            }

            return (jvm_info*)value;
        }

        void set_thread_local_vm(jvm_info* info)
        {
            DWORD tlsIndex = get_tls_index();

            if (::TlsGetValue(tlsIndex) == 0)
            {
                if (::GetLastError() != ERROR_SUCCESS)
                {
                    throw std::exception("TlsGetValue failed");
                }
            }
            else throw std::exception("Only one VM instance is allowed per thread");

            if (::TlsSetValue(tlsIndex, info) == FALSE)
            {
                auto error = ::GetLastError();
                throw std::exception("TlsGetValue failed");
            }
        }

    }

    namespace jni
    {
        jclass define_class(const char* name, jobject loader, jbyte* data, jsize size)
        {
            auto env = internal::get_thread_local_vm()->env;
            jclass ret = env->DefineClass(name, loader, data, size);
            if (env->ExceptionOccurred()) throw exception(env->ExceptionOccurred());
            return ret;
        }

#ifdef DEBUG_REFS
        std::list<jobject> _refs;
#endif
        void delete_local_ref(jobject obj)
        {
            auto env = internal::get_thread_local_vm()->env;
            env->DeleteLocalRef(obj);
#ifdef DEBUG_REFS
            _refs.remove(obj);
#endif
        }

        template <>
        void call_static_method<void>(jclass cls, jmethodID method, ...)
        {
            auto env = internal::get_thread_local_vm()->env;
            va_list args;
            va_start(args, method);
            type_traits<void>::call_static_methodv(env, cls, method, args);
            if (env->ExceptionOccurred()) throw exception(env->ExceptionOccurred());
            va_end(args);
        }
        
        template <>
        void call_static_methodv<void>(jclass cls, jmethodID method, va_list args)
        {
            auto env = internal::get_thread_local_vm()->env;
            type_traits<void>::call_static_methodv(env, cls, method, args);
            if (env->ExceptionOccurred()) throw exception(env->ExceptionOccurred());
        }

        template <>
        void call_method<void>(jobject obj, jmethodID method, ...)
        {
            auto env = internal::get_thread_local_vm()->env;
            va_list args;
            va_start(args, method);
            type_traits<void>::call_methodv(env, obj, method, args);
            if (env->ExceptionCheck()) throw exception(env->ExceptionOccurred());
            va_end(args);
        }

        template <>
        void call_methodv<void>(jobject obj, jmethodID method, va_list args)
        {
            auto env = internal::get_thread_local_vm()->env;
            type_traits<void>::call_methodv(env, obj, method, args);
            if (env->ExceptionCheck()) throw exception(env->ExceptionOccurred());
        }

        jclass find_class(const char* name)
        {
            jclass cls = internal::get_thread_local_vm()->env->FindClass(name);
            if (cls == nullptr) throw std::exception("FindClass failed");
#ifdef DEBUG_REFS
            _refs.push_back(cls);
#endif
            return cls;
        }

        jclass get_object_class(jobject obj)
        {
            jclass cls = internal::get_thread_local_vm()->env->GetObjectClass(obj);
            if (cls == nullptr) throw std::exception("GetObjectClass failed");
#ifdef DEBUG_REFS
            _refs.push_back(cls);
#endif            
            return cls;
        }

        jmethodID get_method_id(jclass clazz, const char* name, const char* sig)
        {
            jmethodID method = internal::get_thread_local_vm()->env->GetMethodID(clazz, name, sig);
            if (method == nullptr) throw std::exception("GetMethodID failed");
            return method;
        }

        jmethodID get_static_method_id(jclass clazz, const char* name, const char* sig)
        {
            jmethodID method = internal::get_thread_local_vm()->env->GetStaticMethodID(clazz, name, sig);
            if (method == nullptr) throw std::exception("GetMethodID failed");
            return method;
        }

        jsize get_array_length(jarray a)
        {
            return internal::get_thread_local_vm()->env->GetArrayLength(a);
        }

        jobject get_object_array_element(jobjectArray a, jsize i)
        {
            auto obj = internal::get_thread_local_vm()->env->GetObjectArrayElement(a, i);
            if (obj == nullptr) throw std::exception("GetObjectArrayElement failed");
#ifdef DEBUG_REFS
            _refs.push_back(obj);
#endif
            return obj;
        }

        void set_object_array_element(jobjectArray a, jsize i, jobject value)
        {
            auto env = internal::get_thread_local_vm()->env;
            env->SetObjectArrayElement(a, i, value);
            if (env->ExceptionCheck()) throw exception(env->ExceptionOccurred());
        }

        jstring new_string_utf(const char* data)
        {
            jstring jstr = internal::get_thread_local_vm()->env->NewStringUTF(data);
            if (jstr == nullptr) throw std::exception("NewStringUTF failed");
#ifdef DEBUG_REFS
            _refs.push_back(jstr);
#endif
            return jstr;
        }

        const char* get_string_utf_chars(jstring jstr)
        {
            auto data = internal::get_thread_local_vm()->env->GetStringUTFChars(jstr, nullptr);
            if (data == nullptr) throw std::exception("GetStringUTFChars failed");
            return data;
        }

        void release_string_utf_chars(jstring jstr, const char* data)
        {
            internal::get_thread_local_vm()->env->ReleaseStringUTFChars(jstr, data);
        }

        jobject allocate_object(jclass cls)
        {
            jobject obj = internal::get_thread_local_vm()->env->AllocObject(cls);
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
            auto env = internal::get_thread_local_vm()->env;
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
            auto env = internal::get_thread_local_vm()->env;
            auto ret = env->NewObjectArray(length, cls, initial);
            if (env->ExceptionCheck()) throw exception(env->ExceptionOccurred());
#ifdef DEBUG_REFS
            _refs.push_back(ret);
#endif
            return ret;
        }

        bool is_assignable_from(jclass src, jclass target)
        {
            return internal::get_thread_local_vm()->env->IsAssignableFrom(src, target) == JNI_TRUE;
        }

        jmethodID from_reflected_method(jobject methodObj)
        {
            auto method = internal::get_thread_local_vm()->env->FromReflectedMethod(methodObj);
            if (method == nullptr) throw std::exception("FromReflectedMethod failed");
            return method;
        }

        jobject to_reflected_method(jclass cls, jmethodID id, jboolean is_static)
        {
            auto method = internal::get_thread_local_vm()->env->ToReflectedMethod(cls, id, is_static);
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