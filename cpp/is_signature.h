#pragma once

#include <type_traits>

template <class T>
struct is_signature : std::false_type
{
};

template <class R, class... Args>
struct is_signature<R(Args...)> : std::true_type
{
};

template <class R, class... Args>
struct is_signature<R(Args...) &> : std::true_type
{
};

template <class R, class... Args>
struct is_signature<R(Args...) &&> : std::true_type
{
};

template <class R, class... Args>
struct is_signature<R(Args...) noexcept> : std::true_type
{
};

template <class R, class... Args>
struct is_signature<R(Args...) & noexcept> : std::true_type
{
};

template <class R, class... Args>
struct is_signature<R(Args...) && noexcept> : std::true_type
{
};

template <class T>
inline constexpr bool is_signature_v = is_signature<T>::value;

template <class T>
concept signature = is_signature_v<T>;

