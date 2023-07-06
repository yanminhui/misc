#pragma once

#include <concepts>
#include <iostream>

namespace detail {

template <class T, class Mutex>
class call_proxy 
{
public:
    call_proxy(T* p, Mutex& mtx) noexcept
        : p_{p}, mtx_{mtx}
    {
    }

    call_proxy(const call_proxy&) = delete;
    call_proxy(call_proxy&& rhs) = delete;
    call_proxy& operator=(const call_proxy&) = delete;
    call_proxy& operator=(call_proxy&&) = delete;

    T* operator->() noexcept
    { 
        return p_;
    }

    ~call_proxy()
    {
        if (p_) {
            mtx_.unlock();
        }
    }

private: 
    T* p_ = nullptr;
    Mutex& mtx_;
};

} // namespace detail

template <class T, class Mutex = typename T::mutex_type>
class monitor_ptr
{
    using proxy_type = detail::call_proxy<T, Mutex>;

public:
    monitor_ptr(T* p, Mutex& mtx) noexcept
        : p_{p}, mtx_{mtx}
    {
    }

    template <class U>
    requires requires(U u) {
        {u.monitor()} -> std::same_as<monitor_ptr>;
    }
    monitor_ptr(U& u)
        : monitor_ptr{u.monitor()}
    {
    }

    explicit operator bool() const noexcept
    {
        return !!p_;
    }

    proxy_type operator->() 
    {
        mtx_.lock();
        return {p_, mtx_}; 
    }

private:
    T* p_ = nullptr;
    Mutex& mtx_;
};

template <class U>
monitor_ptr(U&) -> monitor_ptr<U>;

template <class T>
using monitor_ptr_t = typename T::monitor_ptr_t;

/*
class monitor_example
{
public:
    struct mutex_type
    {
        void lock()
        {
            std::cout << "> lock" << std::endl;
        }
        void unlock()
        {
            std::cout << "> unlock" << std::endl;
        }
    };

    using monitor_ptr_type = monitor_ptr<monitor_example>;

    monitor_ptr_type monitor() noexcept
    {
        return {this, mtx_};
    }

    void print() const noexcept
    {
        std::cout << "print" << std::endl;
    }

private:
    mutex_type mtx_;
};

int main()
{
    monitor_example obj;
    monitor_ptr mp = obj;
    for (std::size_t i = 0; mp && i != 2; ++i) {
        mp->print();
    }
}
*/

