#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "jni.h"

#include "java\type_traits.h"
#include <vector>

#ifdef DEBUG_REFS
#include <list>
#endif

namespace java
{
    namespace internal
    {
        struct jvm_info { JNIEnv* env; };

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
    // around the functions exposed by the JNI.  See the JNI documentation 
    // for further information.
    namespace jni
    {
        jclass define_class(const char* name, jbyte* data, jsize size);

#ifdef DEBUG_REFS
        extern std::list<jobject> _refs;
#endif
        void delete_local_ref(jobject obj);

        jfieldID get_field_id(jclass cls, const char* name, const char* sig);

        jfieldID get_static_field_id(jclass cls, const char* name, const char* sig);

        template <typename jtype>
        jtype get_field(jobject obj, jfieldID id)
        {
            auto env = internal::get_thread_local_vm()->env;
            auto ret = type_traits<jtype>::get_field(env, obj, id);
            if (env->ExceptionCheck()) throw exception(env->ExceptionOccurred());
            return ret;
        };

        template <typename jtype>
        jtype get_static_field(jobject obj, jfieldID id)
        {
            auto env = internal::get_thread_local_vm()->env;
            auto ret = type_traits<jtype>::get_static_field(env, obj, id);
            if (env->ExceptionCheck()) throw exception(env->ExceptionOccurred());
            return ret;
        };

        jclass find_class(const char* name);

        jclass get_object_class(jobject obj);

        jmethodID get_method_id(jclass clazz, const char* name, const char* sig);

        jmethodID get_static_method_id(jclass clazz, const char* name, const char* sig);

        jsize get_array_length(jarray a);
        
        template <typename jtype>
        jtype* get_array_elements(typename type_traits<jtype>::array_type arr, jboolean* isCopy)
        {
            auto env = internal::get_thread_local_vm()->env;
            auto ptr = type_traits<jtype>::get_array_elements(env, arr, nullptr);
            if (env->ExceptionCheck()) throw exception(env->ExceptionOccurred());
            return ptr;
        }

        template <typename jtype>
        void release_array_elements(typename type_traits<jtype>::array_type arr, jtype* ptr, int mode)
        {
            auto env = internal::get_thread_local_vm()->env;
            auto fptr = type_traits<jtype>::release_array_elements;
            type_traits<jtype>::release_array_elements(env, arr, ptr, mode);
            if (env->ExceptionCheck()) throw exception(env->ExceptionOccurred());
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
            auto ret = type_traits<jtype>::call_static_methodv(env, cls, method, args);
            if (env->ExceptionOccurred()) throw exception(env->ExceptionOccurred());
            va_end(args);
            return ret;
        }
        template <>
        void call_static_method<void>(jclass cls, jmethodID method, ...);

        template <typename jtype>
        jtype call_static_methodv(jclass cls, jmethodID method, va_list args)
        {
            auto env = internal::get_thread_local_vm()->env;
            auto ret = type_traits<jtype>::call_static_methodv(env, cls, method, args);
            if (env->ExceptionOccurred()) throw exception(env->ExceptionOccurred());
            return ret;
        }
        template <>
        void call_static_methodv<void>(jclass cls, jmethodID method, va_list args);

        template <typename jtype>
        jtype call_method(jobject obj, jmethodID method, ...)
        {
            auto env = internal::get_thread_local_vm()->env;
            va_list args;
            va_start(args, method);
            auto ret = type_traits<jtype>::call_methodv(env, obj, method, args);
            if (env->ExceptionCheck()) throw exception(env->ExceptionOccurred());
            va_end(args);
            return ret;
        }
        template <>
        void call_method<void>(jobject obj, jmethodID method, ...);

        template <typename jtype>
        jtype call_methodv(jobject obj, jmethodID method, va_list args)
        {
            auto env = internal::get_thread_local_vm()->env;
            auto ret = type_traits<jtype>::call_methodv(env, obj, method, args);
            if (env->ExceptionCheck()) throw exception(env->ExceptionOccurred());
            return ret;
        }
        template <>
        void call_methodv<void>(jobject obj, jmethodID method, va_list args);

        // This function converts a Java jstring into a std::string
        std::string jstring_str(jstring jstr);

    }

    // This class provides automatic garbage collection for local references 
    // returned by the JNI.  It is implemented using reference counting 
    // internally (std::shared_ptr).
    template <typename jobject_t>
    class local_ref
    {
        typedef jobject_t pointer_type;
        typedef std::shared_ptr<_jobject> shared_ptr_type;

        shared_ptr_type _ref;

    public:
        template <typename t>
        local_ref(const local_ref<t>& other)
        {
            _ref = other.ref();
        }

        shared_ptr_type ref() const { return _ref; }

        local_ref() {}

        local_ref(jobject native) : _ref((pointer_type)native, jni::delete_local_ref) {}

        template <typename rhs_jobject_t>
        bool operator== (const local_ref<rhs_jobject_t>& rhs) const { return static_cast<jobject>(_ref.get()) == static_cast<jobject>(rhs._ref.get()); }

        template <typename rhs_jobject_t>
        bool operator!= (const local_ref<rhs_jobject_t>& rhs) const { return static_cast<jobject>(_ref.get()) != static_cast<jobject>(rhs._ref.get()); }

        pointer_type get() const { return (pointer_type)_ref.get(); }
    };

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
    // that initialized the JVM via this class (unless the caller uses the 
    // attach_thread function or vm_thread class to attach additional 
    // threads).
    class vm
    {
        JavaVM* _jvm;
        bool _is_owner;

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

            jint status = JNI_CreateJavaVM(&_jvm, (void**)&info->env, &internal_args);
            if (status != JNI_OK)
            {
                throw std::exception("JNI_CreateJavaVM failed");
            }

            internal::set_thread_local_vm(info);
        }

    public:
        // Creates and initializes a new JVM instance using the specified 
        // arguments.
        vm(const vm_args& args) : _is_owner(true)
        {
            init(args);
        }

        // Creates and initializes a new JVM instance using a default set of 
        // arguments.
        vm() : _is_owner(true)
        {
            init(vm_args());
        }

        // This constructor can be used by Java extension libraries written 
        // in C++.  
        vm(JNIEnv* env) : _is_owner(false)
        {
            if (env->GetJavaVM(&_jvm) != 0)
                throw std::exception("GetJavaVM failed");

            auto info = new internal::jvm_info();
            info->env = env;
            internal::set_thread_local_vm(info);
        }

        // Destroys the object and the JVM instance along with it, unless 
        // the vm was constructed using a pre-existing JNIEnv pointer.
        ~vm()
        {
            auto info = internal::get_thread_local_vm();
            if (info != nullptr)
            {
                if (_is_owner) _jvm->DestroyJavaVM();
                delete info;
            }

            internal::free_tls_index();
        }

        // Attaches the current thread to this JVM.  Make sure to call 
        // detach_thread to reclaim memory before the thread exits.  This 
        // does not currently support JavaVMAttachArgs.
        void attach_thread()
        {
            auto info = new internal::jvm_info();

            if (_jvm->AttachCurrentThread((void**)&info->env, nullptr) != JNI_OK)
                throw std::exception("AttachCurrentThread failed");

            internal::set_thread_local_vm(info);
        }

        // Detaches the current thread from this JVM, and frees thread-local 
        // memory.
        void detach_thread()
        {
            if (_jvm->DetachCurrentThread() != JNI_OK)
                throw std::exception("DetachCurrentThread failed");

            auto info = internal::get_thread_local_vm();
            if (info != nullptr) delete info;
        }
    };

    // This class is useful for safely attaching and detaching a thread 
    // to/from a JVM.  The constructor attaches and the destructor detaches, 
    // such that if the object is allocated on the stack, the detachment 
    // will be automatic.  The caller is responsible for ensuring that the 
    // JVM (java::vm object) remains valid for the lifetime of the vm_thread
    // object.  This does not currently support JavaVMAttachArgs.
    class vm_thread
    {
        vm& _jvm;

    public:
        vm_thread(vm& jvm) : _jvm(jvm)
        {
            _jvm.attach_thread();
        }

        ~vm_thread()
        {
            _jvm.detach_thread();
        }
    };

}
