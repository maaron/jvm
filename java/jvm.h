#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "jni.h"

#include "java\type_traits.h"
#include "java\exception.h"
#include <vector>

namespace java
{
    class java_exception;

    namespace internal
    {
        struct jvm_info { JavaVM* jvm; JNIEnv* env; };

        // This function allocates TLS in order store a single Java VM
        // instance.  This allows a thread to associate a VM with itself so
        // that further calls to the java objects in this namespace can
        // interact with the VM without passing around a pointer to it.
        // However, this has the limitation that a thread can only create a
        // single VM.  This could be expanded by allocating a vector of VM's
        // instead, and providing a function to switch the thread's "current"
        // VM.
        DWORD get_tls_index();

        void free_tls_index();

        jvm_info* get_thread_local_vm();

        void set_thread_local_vm(jvm_info* info);
    }

    // The functions in this namespace are exception-throwing wrappers 
    // around the functions exposed by the JNI.
    namespace jvm
    {
        jclass find_class(const char* name);

        jclass get_object_class(jobject obj);

        jmethodID get_method_id(jclass clazz, const char* name, const char* sig);

        jmethodID get_static_method_id(jclass clazz, const char* name, const char* sig);

        jsize get_array_length(jarray a);
        
        template <typename jtype>
        jtype* get_array_elements(typename jni_type_traits<jtype>::array_type arr, jboolean* isCopy)
        {
            auto env = internal::get_thread_local_vm()->env;
            auto ptr = jni_type_traits<jtype>::get_array_elements(env, arr, nullptr);
            if (env->ExceptionCheck()) throw java_exception(env->ExceptionOccurred());
            return ptr;
        }

        template <typename jtype>
        void release_array_elements(typename jni_type_traits<jtype>::array_type arr, jtype* ptr, int mode)
        {
            auto env = internal::get_thread_local_vm()->env;
            auto fptr = jni_type_traits<jtype>::release_array_elements;
            jni_type_traits<jtype>::release_array_elements(env, arr, ptr, mode);
            if (env->ExceptionCheck()) throw java_exception(env->ExceptionOccurred());
        }

        jobject get_object_array_element(jobjectArray a, jsize i);

        void set_object_array_element(jobjectArray a, jsize i, jobject value);

        jstring new_string_utf(const char* data);

        const char* get_string_utf_chars(jstring jstr);

        void release_string_utf_chars(jstring jstr, const char* data);

        jobject allocate_object(jclass cls);

        jobject new_object(jclass cls, jmethodID ctor, ...);

        jobjectArray new_object_array(jclass cls, jsize length, jobject initial);

        bool is_assignable_from(jclass src, jclass target);

        jmethodID from_reflected_method(jobject methodObj);

        jobject to_reflected_method(jclass cls, jmethodID id, jboolean is_static);

        template <typename jtype>
        jtype call_static_method(jclass cls, jmethodID method, ...)
        {
            auto env = internal::get_thread_local_vm()->env;
            va_list args;
            va_start(args, method);
            auto ret = jni_type_traits<jtype>::call_static_method(env, cls, method, args);
            if (env->ExceptionOccurred()) throw java_exception(env->ExceptionOccurred());
            va_end(args);
            return ret;
        }
        template <>
        void call_static_method<void>(jclass cls, jmethodID method, ...);

        template <typename jtype>
        jtype call_method(jobject obj, jmethodID method, ...)
        {
            auto env = internal::get_thread_local_vm()->env;
            va_list args;
            va_start(args, method);
            auto ret = jni_type_traits<jtype>::call_method(env, obj, method, args);
            if (env->ExceptionCheck()) throw java_exception(env->ExceptionOccurred());
            va_end(args);
            return ret;
        }
        template <>
        void call_method<void>(jobject obj, jmethodID method, ...);

    }

    enum vm_version
    {
#ifdef JNI_VERSION_1_1
        jni_1_1 = JNI_VERSION_1_1,
#endif
#ifdef JNI_VERSION_1_2
        jni_1_2 = JNI_VERSION_1_2,
#endif
#ifdef JNI_VERSION_1_4
        jni_1_4 = JNI_VERSION_1_4,
#endif
#ifdef JNI_VERSION_1_6
        jni_1_6 = JNI_VERSION_1_6,
#endif
        // Don't use this- it is just here to make the C++ syntax work.
        unknown = 0
    };

    // This class is used to encapsulate the parameters that are passed to 
    // the JVM on start-up.
    class vm_args
    {
        std::vector<std::string> _opts;
        bool _ignore;
        vm_version _version;

    public:
        vm_args()
            : _ignore(false), _version(jni_1_6)
        {
        }

        void add_option(const std::string& opt)
        {
            _opts.push_back(opt);
        }

        void ignore_unrecognized(bool ignore) { _ignore = ignore; }
        bool ignore_unrecognized() const { return _ignore; }

        void version(vm_version v) { _version = v; }
        vm_version version() const { return _version; }

        const std::vector<std::string>& options() const { return _opts; }
    };

    // This class is used to initialize a JVM instance and associate it with 
    // the current thread.  Upon desctruction of this object, the JVM wil 
    // also be shutdown.  Thus, the object must stay alive for the duration 
    // of any other function calls (including constructors) from the java::* 
    // namespace, even though the user need not pass references or pointers 
    // to this object to any of those functions.  Internally, a pointer to the
    // JVM (JavaVM*) is maintained in thread-local storage.  This implies that
    // the functions in this namespace can also be called from the same thread
    // that initialized the JVM via this class.
    class vm
    {
        void init(const vm_args& args)
        {
            JavaVMInitArgs internal_args;

            internal_args.ignoreUnrecognized  = args.ignore_unrecognized();
            internal_args.version = args.version();

            auto& opts = args.options();
            
            std::vector<JavaVMOption> internal_opts;
            for (auto it = opts.begin(); it != opts.end(); it++)
            {
                JavaVMOption opt;
                opt.extraInfo = nullptr;
                opt.optionString = const_cast<char*>(it->c_str());
                internal_opts.push_back(opt);
            }

            internal_args.nOptions = opts.size();
            internal_args.options = internal_opts.data();

            auto info = new internal::jvm_info();

            jint status = JNI_CreateJavaVM(&info->jvm, (void**)&info->env, &internal_args);
            if (status != JNI_OK)
            {
                throw std::exception("JNI_CreateJavaVM failed");
            }

            internal::set_thread_local_vm(info);
        }

    public:
        // Creates and initializes a new JVM instance using the specified 
        // arguments.
        vm(const vm_args& args)
        {
            init(args);
        }

        // Creates and initializes a new JVM instance using a default set of 
        // arguments.
        vm()
        {
            init(vm_args());
        }

        // Destroys the object and the JVM instance along with it.
        ~vm()
        {
            auto info = internal::get_thread_local_vm();
            if (info != nullptr)
            {
                info->jvm->DestroyJavaVM();
                delete info;
            }

            internal::free_tls_index();
        }
    };

    // This function converts a Java jstring into a std::string
    std::string jstring_str(jstring jstr);
    
    // Returns the name of a jclass as a string
    std::string get_class_name(jclass cls);

    // This function returns the return type of a java.lang.reflect.Method 
    // object as a string
    std::string get_return_type(jobject methodObj);

}
