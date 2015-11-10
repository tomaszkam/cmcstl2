// cmcstl2 - A concept-enabled C++ standard library
//
//  Copyright Casey Carter 2015
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/caseycarter/cmcstl2
//
#ifndef STL2_DETAIL_CONCEPTS_FUNCTION_HPP
#define STL2_DETAIL_CONCEPTS_FUNCTION_HPP

#include <stl2/type_traits.hpp>
#include <stl2/detail/fwd.hpp>
#include <stl2/detail/concepts/compare.hpp>
#include <stl2/detail/concepts/core.hpp>
#include <stl2/detail/concepts/object.hpp>
#include <stl2/detail/functional/invoke.hpp>

///////////////////////////////////////////////////////////////////////////
// Function Concepts [concepts.lib.function]
//
STL2_OPEN_NAMESPACE {
  ///////////////////////////////////////////////////////////////////////////
  // Function [concepts.lib.functions.function]
  // Not to spec: Named callable, and accepts callables.
  //
  template <class F, class...Args>
  constexpr bool __callable = false;
  template <class F, class...Args>
    requires requires (F& f, Args&&...args) {
      __stl2::invoke(f, (Args&&)args...);
    }
  constexpr bool __callable<F, Args...> = true;

  // FIXME: remove this transitional paranoia check.
  template <class T>
  constexpr bool __force_non_reference() {
    static_assert(!is_reference<T>::value);
    return true;
  }

  template <class F, class...Args>
  concept bool Callable() {
    return __force_non_reference<F>() &&
      CopyConstructible<F>() &&
      __callable<F, Args...>;
  }

  namespace models {
    template <class, class...>
    constexpr bool Callable = false;
    __stl2::Callable{F, ...Args}
    constexpr bool Callable<F, Args...> = true;
  }

  ///////////////////////////////////////////////////////////////////////////
  // RegularFunction [concepts.lib.functions.regularfunction]
  // Not to spec: named RegularCallable
  //
  template <class F, class...Args>
  concept bool RegularCallable() {
    return Callable<F, Args...>();
  }

  namespace models {
    template <class, class...>
    constexpr bool RegularCallable = false;
    __stl2::RegularCallable{F, ...Args}
    constexpr bool RegularCallable<F, Args...> = true;
  }

  ///////////////////////////////////////////////////////////////////////////
  // Predicate [concepts.lib.functions.predicate]
  //
  template <class F, class...Args>
  concept bool Predicate() {
    return RegularCallable<F, Args...>() &&
      Boolean<result_of_t<F&(Args...)>>();
  }

  namespace models {
    template <class, class...>
    constexpr bool Predicate = false;
    __stl2::Predicate{F, ...Args}
    constexpr bool Predicate<F, Args...> = true;
  }

  ///////////////////////////////////////////////////////////////////////////
  // WeakRelation
  // Extension: Equivalent to Relation, but does not require common_type to
  //            be specialized.
  //
  namespace ext {
    template <class R, class T>
    concept bool WeakRelation() {
      return Predicate<R, T, T>();
    }

    template <class R, class T, class U>
    concept bool WeakRelation() {
      return WeakRelation<R, T>() &&
        WeakRelation<R, U>() &&
        Predicate<R, T, U>() &&
        Predicate<R, U, T>();
    }
  }

  namespace models {
    template <class R, class T, class U = T>
    constexpr bool WeakRelation = false;
    __stl2::ext::WeakRelation{R, T}
    constexpr bool WeakRelation<R, T, T> = true;
    __stl2::ext::WeakRelation{R, T, U}
    constexpr bool WeakRelation<R, T, U> = true;
  }

  ///////////////////////////////////////////////////////////////////////////
  // Relation [concepts.lib.functions.relation]
  //
  template <class R, class T>
  concept bool Relation() {
    return ext::WeakRelation<R, T>();
  }

  template <class R, class T, class U>
  concept bool Relation() {
    return ext::WeakRelation<R, T, U>() &&
      CommonReference<const T&, const U&>() &&
      ext::WeakRelation<R, CommonType<T, U>>();
  }

  namespace models {
    template <class R, class T, class U = T>
    constexpr bool Relation = false;
    __stl2::Relation{R, T}
    constexpr bool Relation<R, T, T> = true;
    __stl2::Relation{R, T, U}
    constexpr bool Relation<R, T, U> = true;
  }

  ///////////////////////////////////////////////////////////////////////////
  // StrictWeakOrder [concepts.lib.functions.strictweakorder]
  //
  template <class R, class T>
  concept bool StrictWeakOrder() {
    return Relation<R, T>();
  }

  template <class R, class T, class U>
  concept bool StrictWeakOrder() {
    return Relation<R, T, U>();
  }

  namespace models {
    template <class R, class T, class U = T>
    constexpr bool StrictWeakOrder = false;
    __stl2::StrictWeakOrder{R, T}
    constexpr bool StrictWeakOrder<R, T, T> = true;
    __stl2::StrictWeakOrder{R, T, U}
    constexpr bool StrictWeakOrder<R, T, U> = true;
  }
} STL2_CLOSE_NAMESPACE

#endif
