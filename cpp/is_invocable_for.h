#pragma once

#include <type_traits>

template <class F, class Signature>
struct is_invocable_for : std::false_type
{
};

template <class R, class F, class... Args>
struct is_invocable_for<F, R(Args...)> : std::is_invocable_r<R, F, Args...>
{
};


template <class R, class F, class... Args>
struct is_invocable_for<F, R(Args...) &> : std::is_invocable_r<R, F, Args...>
{
};

template <class R, class F, class... Args>
struct is_invocable_for<F, R(Args...) &&> : std::is_invocable_r<R, F, Args...>
{
};

template <class R, class F, class... Args>
struct is_invocable_for<F, R(Args...) noexcept> : std::is_invocable_r<R, F, Args...>
{
};

template <class R, class F, class... Args>
struct is_invocable_for<F, R(Args...) & noexcept> : std::is_invocable_r<R, F, Args...>
{
};

template <class R, class F, class... Args>
struct is_invocable_for<F, R(Args...) && noexcept> : std::is_invocable_r<R, F, Args...>
{
};

template <class F, class Signature>
inline constexpr bool is_invocable_for_v = is_invocable_for<F, Signature>::value;

template <class F, class Signature>
concept invocable_for = is_invocable_for_v<F, Signature>;

