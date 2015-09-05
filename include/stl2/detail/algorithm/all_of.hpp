// Adapted (stolen) from Range v3 library
//
//  Copyright Andrew Sutton 2014
//  Copyright Casey Carter 2015
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/caseycarter/cmcstl2
//
#ifndef STL2_DETAIL_ALGORITHM_ALL_OF_HPP
#define STL2_DETAIL_ALGORITHM_ALL_OF_HPP

#include <stl2/functional.hpp>
#include <stl2/iterator.hpp>
#include <stl2/detail/fwd.hpp>
#include <stl2/detail/concepts/callable.hpp>

namespace stl2 { inline namespace v1 {
// 20150803: Not to spec: the Destructible requirements are implicit.
template <InputIterator I, Sentinel<I> S, Destructible F, Destructible P = identity>
  requires IndirectCallablePredicate<F, Projected<I, P>>()
bool all_of(I first, S last, F pred, P proj = P{}) {
  auto &&ipred = as_function(pred);
  auto &&iproj = as_function(proj);
  while (first != last && ipred(iproj(*first))) {
    ++first;
  }
  return first == last;
}

// 20150801: Not to spec: the MoveConstructible requirements are implicit.
template <InputRange R, MoveConstructible F, MoveConstructible P = identity>
  requires IndirectCallablePredicate<F, Projected<IteratorType<R>, P>>()
bool all_of(R&& rng, F pred, P proj = P{}) {
  return all_of(stl2::begin(rng), stl2::end(rng), stl2::move(pred), stl2::move(proj));
}
}}

#endif
