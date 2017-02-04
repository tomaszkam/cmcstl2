// cmcstl2 - A concept-enabled C++ standard library
//
//  Copyright Casey Carter 2015, 2017
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/caseycarter/cmcstl2
//
#ifndef STL2_DETAIL_COMPRESSED_PAIR_HPP
#define STL2_DETAIL_COMPRESSED_PAIR_HPP

#include <utility>
#include <stl2/detail/compressed_tuple.hpp>
#include <stl2/detail/fwd.hpp>
#include <stl2/detail/tagged.hpp>
#include <stl2/detail/algorithm/tagspec.hpp>
#include <stl2/detail/concepts/object.hpp>

STL2_OPEN_NAMESPACE {
	namespace ext {
		template <class First, class Second>
		requires
			models::Destructible<First> &&
			models::Destructible<Second>
		struct compressed_pair
		: tagged_compressed_tuple<tag::first(First), tag::second(Second)>
		{
		private:
			using base_t = tagged_compressed_tuple<tag::first(First), tag::second(Second)>;
		public:
			constexpr compressed_pair()
			noexcept(std::is_nothrow_default_constructible_v<base_t>)
			requires DefaultConstructible<base_t>
			: base_t{} {}

			using base_t::base_t;
			using base_t::first;
			using base_t::second;

			template <class F, class S>
			requires
				Constructible<F, const First&> &&
				Constructible<S, const Second&>
			constexpr operator std::pair<F, S> () const {
				return std::pair<F, S>{first(), second()};
			}
		};

		template <class First, class Second>
		requires
			Constructible<__unwrap<First>, First> &&
			Constructible<__unwrap<Second>, Second>
		constexpr auto make_compressed_pair(First&& f, Second&& s)
		STL2_NOEXCEPT_RETURN(
			compressed_pair<__unwrap<First>, __unwrap<Second>>{
				std::forward<First>(f), std::forward<Second>(s)
			}
		)
	} // namespace ext
} STL2_CLOSE_NAMESPACE

namespace std {
	template <class First, class Second>
	struct tuple_size< ::__stl2::ext::compressed_pair<First, Second>>
	: integral_constant<size_t, 2>
	{};

	template <class First, class Second>
	struct tuple_element<0, ::__stl2::ext::compressed_pair<First, Second>> {
		using type = First;
	};

	template <class First, class Second>
	struct tuple_element<1, ::__stl2::ext::compressed_pair<First, Second>> {
		using type = Second;
	};
} // namespace std

#endif
