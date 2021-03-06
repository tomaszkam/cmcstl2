// cmcstl2 - A concept-enabled C++ standard library
//
//  Copyright Casey Carter 2016
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/caseycarter/cmcstl2
//
#include <stl2/view/take_exactly.hpp>
#include <stl2/view/iota.hpp>
#include <stl2/view/filter.hpp>
#include <stl2/view/subrange.hpp>
#include <stl2/detail/iterator/istream_iterator.hpp>
#include <memory>
#include <vector>
#include <sstream>
#include "../simple_test.hpp"

namespace ranges = __stl2;

namespace {
	template<class I, class S>
	struct my_subrange : ranges::subrange<I, S> {
		my_subrange() = default;
		my_subrange(I i, S s)
		: ranges::subrange<I, S>{i, s} {}
		I begin() { return this->my_subrange::subrange::begin(); }
		S end() { return this->my_subrange::subrange::end(); }
	};
}

int main()
{
	using namespace ranges;

	{
		auto rng = view::iota(0) | view::ext::take_exactly(10);
		static_assert(View<decltype(rng)>);
		static_assert(!SizedRange<iota_view<int>>);
		static_assert(Range<const decltype(rng)>);
		CHECK_EQUAL(rng, {0,1,2,3,4,5,6,7,8,9});
	}

	{
		auto rng = view::iota(0, 100) | view::ext::take_exactly(10);
		static_assert(View<decltype(rng)>);
		static_assert(Range<const decltype(rng)>);
		CHECK_EQUAL(rng, {0,1,2,3,4,5,6,7,8,9});
	}

	{
		auto rng = view::iota(0, 9) | view::ext::take_exactly(10);
		static_assert(View<decltype(rng)>);
		static_assert(Range<const decltype(rng)>);
		CHECK_EQUAL(rng, {0,1,2,3,4,5,6,7,8,9});
	}

	{
		auto evens = [](int i){return i%2 == 0;};
		std::stringstream sin{"0 1 2 3 4 5 6 7 8 9"};
		my_subrange is{istream_iterator<int>{sin}, istream_iterator<int>{}};
		static_assert(InputRange<decltype(is)>);
		auto rng = is | view::filter(evens) | view::ext::take_exactly(3);
		static_assert(View<decltype(rng)>);
		static_assert(!Range<const decltype(rng)>);
		CHECK_EQUAL(rng, {0,2,4});
	}

	{
		auto odds = [](int i){return i%2 == 1;};
		std::stringstream sin{"0 1 2 3 4 5 6 7 8 9"};
		my_subrange is{istream_iterator<int>{sin}, istream_iterator<int>{}};
		auto pipe = view::filter(odds) | view::ext::take_exactly(3);
		auto rng = is | pipe;
		static_assert(View<decltype(rng)>);
		static_assert(!Range<const decltype(rng)>);
		CHECK_EQUAL(rng, {1,3,5});
	}

	return ::test_result();
}
