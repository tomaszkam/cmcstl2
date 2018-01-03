// cmcstl2 - A concept-enabled C++ standard library
//
//  Copyright Eric Niebler 2017
//  Copyright Casey Carter 2017
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/caseycarter/cmcstl2
//
#ifndef STL2_VIEW_ALL_HPP
#define STL2_VIEW_ALL_HPP

#include <type_traits>
#include <stl2/detail/fwd.hpp>
#include <stl2/detail/concepts/core.hpp>
#include <stl2/detail/concepts/object.hpp>
#include <stl2/detail/memory/addressof.hpp>
#include <stl2/detail/range/access.hpp>
#include <stl2/detail/range/concepts.hpp>
#include <stl2/detail/view/view_closure.hpp>
#include <stl2/view/view_interface.hpp>

STL2_OPEN_NAMESPACE {
	namespace ext {
		template <Range Rng>
		requires !(_Is<Rng, std::is_reference> || View<Rng>)
		class ref_view : public view_interface<ref_view<Rng>> {
			Rng* rng_ = nullptr;
		public:
			constexpr ref_view() noexcept = default;
			constexpr ref_view(Rng& rng) noexcept
			: rng_{detail::addressof(rng)} {}

			constexpr iterator_t<Rng> begin() const
			STL2_NOEXCEPT_RETURN(
				__stl2::begin(*rng_)
			)
			friend constexpr iterator_t<Rng> begin(ref_view r)
			STL2_NOEXCEPT_RETURN(
				__stl2::begin(*r.rng_)
			)

			constexpr sentinel_t<Rng> end() const
			STL2_NOEXCEPT_RETURN(
				__stl2::end(*rng_)
			)
			friend constexpr sentinel_t<Rng> end(ref_view r)
			STL2_NOEXCEPT_RETURN(
				__stl2::end(*r.rng_)
			)

			constexpr bool empty() const
			noexcept(noexcept(__stl2::empty(*rng_)))
			requires detail::CanEmpty<Rng>
			{ return __stl2::empty(*rng_); }

			constexpr auto size() const
			noexcept(noexcept(__stl2::size(*rng_)))
			requires SizedRange<Rng>
			{ return __stl2::size(*rng_); }

			constexpr auto data() const
			noexcept(noexcept(__stl2::data(*rng_)))
			requires ContiguousRange<Rng>
			{ return __stl2::data(*rng_); }
		};
	} // namespace ext

	namespace view {
		namespace __all {
			struct fn : detail::__pipeable<fn> {
				template <Range Rng>
				requires View<__f<Rng>>
				constexpr View operator()(Rng&& rng) const
				noexcept(std::is_nothrow_constructible_v<__f<Rng>, Rng>)
				{ return std::forward<Rng>(rng); }

				// Not to spec: ref_view
				template <Range Rng>
				requires !View<std::remove_cv_t<Rng>>
				constexpr View operator()(Rng& rng) const noexcept
				{ return ext::ref_view{rng}; }
			};
		}

		inline constexpr __all::fn all {};
	}

	namespace ext {
		template <ViewableRange Rng>
		using all_view = decltype(view::all(std::declval<Rng>()));
	}

	// Work-around for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=82507
	template <class V, class R>
	concept bool _ConstructibleFromRange =
		requires { typename ext::all_view<R>; } &&
		View<V> && Constructible<V, ext::all_view<R>>;
} STL2_CLOSE_NAMESPACE

#endif
