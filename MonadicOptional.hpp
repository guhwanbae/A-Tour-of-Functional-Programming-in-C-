#pragma once

#include <functional>
#include <optional>
#include <type_traits>

namespace gu {
namespace infix {

namespace detail {

template <class T>
struct is_optional_impl : std::false_type {};
template <class T>
struct is_optional_impl<std::optional<T>> : std::true_type {};
template <class T>
using is_optional = is_optional_impl<std::decay_t<T>>;

template <class F>
struct Pipable : F {
  constexpr Pipable(F &&f) noexcept(noexcept(F{std::forward<F>(f)}))
      : F{std::forward<F>(f)} {}
};

template <class Optional, class F>
constexpr auto operator|(Optional &&o, Pipable<F> &&f) {
  static_assert(is_optional<Optional>::value,
                "Given argument must be std::optional");
  return std::invoke(f, std::forward<Optional>(o));
}

template <class Optional, class F>
constexpr auto and_then(Optional &&o, F &&f) {
  using T = typename std::decay_t<Optional>::value_type;
  using Ret = std::invoke_result_t<F, T>;
  static_assert(is_optional<Ret>::value,
                "Return type of F must be std::optional");

  if (o.has_value()) {
    return std::invoke(std::forward<F>(f), std::forward<T>(o.value()));
  } else {
    return Ret{std::nullopt};
  }
}

template <class Optional, class F>
constexpr auto transform(Optional &&o, F &&f) {
  using T = typename std::decay_t<Optional>::value_type;
  using Ret = std::invoke_result_t<F, T>;

  if (o.has_value()) {
    return std::make_optional(
        std::invoke(std::forward<F>(f), std::forward<T>(o.value())));
  } else {
    return std::optional<Ret>{std::nullopt};
  }
}

template <class Optional, class F>
constexpr auto or_else(Optional &&o, F &&f) {
  using T = typename std::decay_t<Optional>::value_type;

  if (o.has_value()) {
    return o;
  } else {
    std::invoke(std::forward<F>(f));
    return std::optional<T>{std::nullopt};
  }
}

/// Experimental
template <class Optional, class F>
constexpr auto filter(Optional &&o, F &&f) {
  using T = typename std::decay_t<Optional>::value_type;

  if (o.has_value() && std::invoke(std::forward<F>(f), o.value())) {
    return o;
  } else {
    return std::optional<T>{std::nullopt};
  }
}
/// Experimental

}  // namespace detail

template <class F>
constexpr auto and_then(F &&f) {
  return detail::Pipable{[&](auto &&o) {
    return detail::and_then(std::forward<decltype(o)>(o), std::forward<F>(f));
  }};
}

template <class F>
constexpr auto transform(F &&f) {
  return detail::Pipable{[&](auto &&o) {
    return detail::transform(std::forward<decltype(o)>(o), std::forward<F>(f));
  }};
}

template <class F>
constexpr auto or_else(F &&f) {
  return detail::Pipable{[&](auto &&o) {
    return detail::or_else(std::forward<decltype(o)>(o), std::forward<F>(f));
  }};
}

/// Experimental
template <class F>
constexpr auto filter(F &&f) {
  return detail::Pipable{[&](auto &&o) {
    return detail::filter(std::forward<decltype(o)>(o), std::forward<F>(f));
  }};
}
/// Experimental

}  // namespace infix
}  // namespace gu
