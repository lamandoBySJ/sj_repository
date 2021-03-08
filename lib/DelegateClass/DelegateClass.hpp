#ifndef DELEGATE_CLASS_H
#define DELEGATE_CLASS_H

#include <Arduino.h>
#include <ColorSensorBase.h>
#include <cxxsupport/mstd_type_traits.h>

template <typename Signature>
class DelegateClass;

namespace md{
    struct [[gnu::may_alias]] DelegateClassBase
    {
        struct _model_function_object {
            struct _class;
            void (_class::*_methodfunc)(int);
            void *obj;
        };
        struct alignas(_model_function_object) [[gnu::may_alias]] Store {
            char data[sizeof(_model_function_object)];
        };
        Store _storage;

        DelegateClassBase()=default;
        ~DelegateClassBase()=default;
        void (*_call)(); 
        using Control = void(*)();
        Control &control()
        {
            return _call;
        }

        const Control &control() const
        {
            return _call;
        }
        DelegateClassBase(std::nullptr_t) noexcept : _call(nullptr) { }

        auto call_fn() const -> decltype(_call) 
            {
                return _call;
            }
        void clear() noexcept
        {
            
            control() = nullptr;
        }
        void swap(DelegateClassBase &other) noexcept
        {
            std::swap(_storage, other._storage);
            std::swap(_call, other._call);
        }
        void destroy()
        {

        }
     };
}

template<typename R,typename... ArgTs>
class DelegateClass<R(ArgTs...)> : private md::DelegateClassBase
{
    
public:
    using result_type = R;

    DelegateClass() noexcept : DelegateClassBase(nullptr) { }
    DelegateClass(std::nullptr_t) noexcept : DelegateClass() { }
    DelegateClass(const DelegateClass &other) = default;
    DelegateClass(DelegateClass &&other) = default;
    ~DelegateClass() = default;

    template<typename Obj, typename Method, typename std::enable_if_t<mstd::is_invocable_r<R, Method, Obj, ArgTs...>::value, int> = 0>
    DelegateClass(Obj obj, Method method) : DelegateClassBase()
    {
        generate([obj, method](ArgTs... args) {
            return detail::invoke_r<R>(method, obj, std::forward<ArgTs>(args)...);
        });
    }
    template<typename Fn, typename BoundArg, typename std::enable_if_t<mstd::is_invocable_r<R, Fn, BoundArg, ArgTs...>::value, int> = 0>
    DelegateClass(Fn func, BoundArg arg) : DelegateClassBase()
    {
        generate([func, arg](ArgTs... args) {
            return detail::invoke_r<R>(func, arg, std::forward<ArgTs>(args)...);
        });
    }

    template <typename F,
            typename std::enable_if_t<
                    !detail::can_null_check<F>::value &&
                    mstd::is_invocable_r<R, F, ArgTs...>::value, int> = 0>
    DelegateClass(F f) : DelegateClassBase()
    {
        static_assert(std::is_copy_constructible<F>::value, "Callback F must be CopyConstructible");
        generate(std::move(f));
    }
    template <typename F,
            typename std::enable_if_t<
                    detail::can_null_check<F>::value &&
                    mstd::is_invocable_r<R, F, ArgTs...>::value, int> = 0>
    DelegateClass(F f) : DelegateClassBase()
    {
        static_assert(std::is_copy_constructible<F>::value, "Callback F must be CopyConstructible");
        if (!f) {
            clear();
        } else {
            generate(std::move(f));
        }
    }

    DelegateClass &operator=(const DelegateClass  &that) = default;
    DelegateClass &operator=(DelegateClass &&that) = default;

    template <typename F,
              typename = std::enable_if_t<
                  mstd::is_invocable_r<R, F, ArgTs...>::value &&
                  !mstd::is_same<mstd::remove_cvref_t<F>, DelegateClass>::value>>
    DelegateClass &operator=(F &&f)
    {
        // C++ standard says to use swap, but that's overkill with no exceptions
        // Callback(std::forward<F>(that)).swap(*this);
        this->~DelegateClass();
        new (this) DelegateClass(std::forward<F>(f));
        return *this;
    }
    // *INDENT-ON*

    template <typename F>
    DelegateClass &operator=(std::reference_wrapper<F> f) noexcept
    {
        // C++ standard says to use swap, but that's overkill with no exceptions
        // Callback(f).swap(*this);
        this->~DelegateClass();
        new (this) DelegateClass(f);
        return *this;
    }

    /** Empty a callback
     */
    DelegateClass &operator=(std::nullptr_t) noexcept
    {
        destroy();
        clear();

        return *this;
    }
    R call(ArgTs... args) const
    {
        MBED_ASSERT(bool(*this));
        auto op_call = reinterpret_cast<call_type *>(call_fn());
        return op_call(this, args...);
    }
  
    explicit operator bool() const noexcept
    {
        return control();
    }
private:

    using call_type =R(const DelegateClassBase *, ArgTs...);

    template <typename F, typename = std::enable_if_t<!std::is_lvalue_reference<F>::value>>
    void generate(F&& f)
    {
            //static_assert(std::is_same<decltype(target_call<F>), call_type>::value, "Call type mismatch");

            static_assert(sizeof(DelegateClass) == sizeof(DelegateClassBase), "Callback should be same size as CallbackBase");
            static_assert(std::is_trivially_copyable<DelegateClassBase>::value, "CallbackBase expected to be TriviallyCopyable");
            // Avoid the need for the const ops table - just one function pointer in the Callback itself
            _call = reinterpret_cast<void (*)()>(target_call<F>);
            static_assert(std::is_trivially_copyable<F>::value, "F must be TriviallyCopyable. Turn on Mbed configuration option 'platform.callback-nontrivial' to use more complex function objects");
            static_assert(std::is_trivially_copyable<DelegateClass>::value, "Callback expected to be TriviallyCopyable");

            // Move the functor into storage
            static_assert(sizeof(F) <= sizeof(Store) && alignof(F) <= alignof(Store),
                        "Type F must not exceed the size of the Callback class");
            new (&_storage) (F) (std::move(f));
    }
    template <typename F>
    static R target_call(const DelegateClassBase *p, ArgTs... args)
    {
        // Need for const_cast here correlates to a std::function bug - see P0045 and N4159
        F &f = const_cast<F &>(reinterpret_cast<const F &>(p->_storage));
        return detail::invoke_r<R>(f, std::forward<ArgTs>(args)...);
    }
};

template <typename R, typename... ArgTs>
DelegateClass<R(ArgTs...)> delegate(R(*func)(ArgTs...) = nullptr) noexcept
{
    return DelegateClass<R(ArgTs...)>(func);
}

template <typename R, typename... ArgTs>
DelegateClass<R(ArgTs...)> delegate(const DelegateClass<R(ArgTs...)> &func)
{
    return DelegateClass<R(ArgTs...)>(func);
}

template<typename T, typename U, typename R, typename... ArgTs>
DelegateClass<R(ArgTs...)> delegate(U *obj, R(T::*method)(ArgTs...)) noexcept
{
    return DelegateClass<R(ArgTs...)>(obj, method);
}

template<typename T, typename U, typename R, typename... ArgTs>
DelegateClass<R(ArgTs...)> delegate(U *obj, R(T::*method)(ArgTs...) &) noexcept
{
    return DelegateClass<R(ArgTs...)>(obj, method);
}

struct A
{
    void Fun(){   Serial.println("################### Fun ################"); }
    void Fun1(int i){ Serial.println("################### Fun1 ################" +String(i,DEC));  }
    void Fun2(int i, double j){ Serial.println("################### Fun2 ################" +String(i,DEC)+String(j,DEC)); }
};

#define APP_VERSION_CAT(a,b) a##" "##b 
//#define APP_VERSION APP_VERSION_CAT(__DATA__,__TIME__)
#define APP_VERSION 20210330L
struct GetAppVersion : public std::integral_constant< unsigned int,APP_VERSION> {};

enum class ExceptionType {
        SensorException = 0,
        RTCException,
        FileException,
        OtherException
 };
struct ExceptionCatcher
{
    ExceptionCatcher()=default;
    void PrintTrace(ExceptionType type,String e){ 
        mutex.lock();
        switch (type)
        {
        case ExceptionType::SensorException:
            ExceptionCatcher::exceptionType="SensorException";
            digitalWrite(18,HIGH);
            break;
        case ExceptionType::RTCException:
            ExceptionCatcher::exceptionType="RTCException";
            digitalWrite(23,HIGH);
            break;
        case ExceptionType::FileException:
            ExceptionCatcher::exceptionType="FileException";
            break;
        case ExceptionType::OtherException:
            ExceptionCatcher::exceptionType="OtherException";
            break;
        default:
            ExceptionCatcher::exceptionType="DefaultException";
            break;
        }
        debug("%s:%s\n",ExceptionCatcher::exceptionType.c_str(),e.c_str());
        mutex.unlock();
    }
    
    rtos::Mutex mutex;
    static String exceptionType;
};

#endif
