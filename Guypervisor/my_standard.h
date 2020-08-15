#pragma once

namespace guystd {
    enum class align_val_t : size_t {};
}
/*
namespace guystd {
    
    // Integral constant
    template <class T, T v>
    struct integral_constant {
        static constexpr T value = v;
        typedef T value_type;
        typedef integral_constant<T, v> type;
        constexpr operator T() const noexcept { return v; }
        constexpr T operator()() const noexcept { return v; }
    };
       
    // Acquire type without reference
    template <class T> 
    struct remove_reference {
        typedef T type;
    };

    template <class T>
    struct remove_reference <T&> {
        typedef T type;
    };

    template <class T>
    struct remove_reference <T&&> {
        typedef T type;
    };

    // True / false types
    typedef integral_constant<bool, true> true_type;
    typedef integral_constant<bool, false> false_type;

    // is_rvalue_reference
    template <class T>
    struct is_rvalue_reference : false_type {};
    
    template <class T>
    struct is_rvalue_reference <T&&> : true_type {};

    // is_lvalue_reference
    template <class T>
    struct is_lvalue_reference: false_type {};

    template <class T>
    struct is_lvalue_reference <T&> : true_type {};

    template <class T>
    struct is_lvalue_reference <T&&> : false_type {};

    // Is refernce struct
    template<class T>
    struct is_reference : integral_constant < bool, is_lvalue_reference<T>::value || is_rvalue_reference<T>::value > {};


    // Conditional struct
    template <bool F, class T, class F>
    struct conditional {
        typedef T type;
    };

    template <class T, class F>
    struct conditional <false, T, F> {
        typedef F type;
    };


    // Default deleter
    template <class T>
    class default_deleter
    {
    public:
        void operator()(int x)
        {
            delete T;
        }
    };

    // Unique pointer
    template <class T, class D = default_deleter<T>>
    class unique_ptr
    {
    public:
        constexpr unique_ptr() noexcept;
        constexpr unique_ptr(nullptr_t) noexcept;
        explicit unique_ptr(void* p) noexcept;
        unique_ptr(void* p,
            typename conditional<is_reference<D>::value, D, const D&> del) noexcept;
        unique_ptr(void* p,
            typename remove_reference<D>::type&& del) noexcept;
        unique_ptr(unique_ptr&& x) noexcept;

        template <class U, class E> 
            unique_ptr(unique_ptr<U, E>&& x) noexcept;

        unique_ptr(const unique_ptr&) = delete;
        
        typedef T element_type;
        typedef D deleter_type;
        remove_reference<D>::type pointer;
    };

    // template <class T, class D> class unique_ptr<T[], D>;
}*/
