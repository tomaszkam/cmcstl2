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
#ifndef STL2_DETAIL_RANGE_BEGIN_END_SIZE_HPP
#define STL2_DETAIL_RANGE_BEGIN_END_SIZE_HPP

#include <stl2/detail/fwd.hpp>
#include <stl2/detail/concepts/core.hpp>
#include <stl2/detail/concepts/object.hpp>
#include <stl2/detail/iterator/concepts.hpp>

STL2_OPEN_NAMESPACE {
	///////////////////////////////////////////////////////////////////////////
	// begin [iterator.range.begin]
	//
	namespace __begin {
		// Poison pill for std::begin. (See the detailed discussion at
		// https://github.com/ericniebler/stl2/issues/139)
		void begin(auto&) = delete;

		template <class R>
		constexpr bool has_member = false;
		template <class R>
			requires requires(R& r) {
				requires Iterator<__f<decltype(r.begin())>>;
			}
		constexpr bool has_member<R> = true;

		template <class R>
		constexpr bool has_non_member = false;
		template <class R>
		requires
			requires(R& r) {
				requires Iterator<__f<decltype(begin(r))>>;
			}
		constexpr bool has_non_member<R> = true;

		struct fn {
			template <class R, std::size_t N>
			constexpr R* operator()(R (&array)[N]) const noexcept {
				return array;
			}
			// Prefer member if it returns Iterator.
			template <class R>
			requires has_member<R>
			constexpr Iterator
			operator()(R& r) const
			STL2_NOEXCEPT_RETURN(
				r.begin()
			)
			// Use ADL if it returns Iterator.
			template <class R>
			requires !has_member<R> && has_non_member<R>
			constexpr Iterator
			operator()(R& r) const
			STL2_NOEXCEPT_RETURN(
				begin(r)
			)
			template <_IsNot<is_array> R>
			[[deprecated]] constexpr Iterator
			operator()(const R&& r) const
			noexcept(noexcept(declval<const fn&>()(r)))
			requires has_member<const R> || has_non_member<const R>
			{
				return (*this)(r);
			}
		};
	}
	// Workaround GCC PR66957 by declaring this unnamed namespace inline.
	inline namespace {
		constexpr auto& begin = detail::static_const<__begin::fn>::value;
	}

	///////////////////////////////////////////////////////////////////////////
	// end [iterator.range.end]
	//
	namespace __end {
		// Poison pill for std::end. (See the detailed discussion at
		// https://github.com/ericniebler/stl2/issues/139)
		void end(auto&) = delete;

		template <class R>
		constexpr bool has_member = false;
		template <class R>
		requires
			requires(R& r) {
				requires Sentinel<__f<decltype(r.end())>, decltype(__stl2::begin(r))>;
			}
		constexpr bool has_member<R> = true;

		template <class R>
		constexpr bool has_non_member = false;
		template <class R>
		requires
			requires(R& r) {
				requires Sentinel<__f<decltype(end(r))>, decltype(__stl2::begin(r))>;
			}
		constexpr bool has_non_member<R> = true;

		struct fn {
			template <class R, std::size_t N>
			constexpr R* operator()(R (&array)[N]) const noexcept {
				return array + N;
			}
			// Prefer member if it returns Sentinel.
			template <class R>
			requires has_member<R>
			constexpr auto operator()(R& r) const
			STL2_NOEXCEPT_RETURN(
				r.end()
			)
			// Use ADL if it returns Sentinel.
			template <class R>
			requires !has_member<R> && has_non_member<R>
			constexpr auto operator()(R& r) const
			STL2_NOEXCEPT_RETURN(
				end(r)
			)
			template <_IsNot<is_array> R>
			[[deprecated]] constexpr auto operator()(const R&& r) const
			noexcept(noexcept(declval<const fn&>()(r)))
			requires has_member<const R> || has_non_member<const R>
			{
				return (*this)(r);
			}
		};
	}
	// Workaround GCC PR66957 by declaring this unnamed namespace inline.
	inline namespace {
		constexpr auto& end = detail::static_const<__end::fn>::value;
	}

	///////////////////////////////////////////////////////////////////////////
	// Range [ranges.range]
	//
	template <class T>
	using iterator_t = decltype(__stl2::begin(declval<T&>()));

	template <class T>
	using sentinel_t = decltype(__stl2::end(declval<T&>()));

	template <class T>
	concept bool Range =
		requires { typename sentinel_t<T>; };

	namespace models {
		template <class>
		constexpr bool Range = false;
		__stl2::Range{R}
		constexpr bool Range<R> = true;
	}

	///////////////////////////////////////////////////////////////////////////
	// size [range.primitives.size]
	//
	namespace __size {
		// Poison pill for std::size. (See the detailed discussion at
		// https://github.com/ericniebler/stl2/issues/139)
		void size(const auto&) = delete;

		template <class R>
		constexpr bool has_member = false;
		template <class R>
		requires
			requires(const R& r) {
				requires Integral<__f<decltype(r.size())>>;
			}
		constexpr bool has_member<R> = true;

		template <class R>
		constexpr bool has_non_member = false;
		template <class R>
		requires
			requires(const R& r) {
				requires Integral<__f<decltype(size(r))>>;
			}
		constexpr bool has_non_member<R> = true;

		template <class R>
		constexpr bool has_difference = false;
		template <class R>
		requires
			requires(const R& r) {
				requires SizedSentinel<decltype(__stl2::end(r)), decltype(__stl2::begin(r))>;
			}
		constexpr bool has_difference<R> = true;

		struct fn {
			template <class T, std::size_t N>
			constexpr std::size_t operator()(T(&)[N]) const noexcept {
				return N;
			}
			// Prefer member
			template <class R>
			requires has_member<R>
			constexpr auto operator()(const R& r) const
			STL2_NOEXCEPT_RETURN(
				r.size()
			)
			// Use non-member
			template <class R>
			requires !has_member<R> && has_non_member<R>
			constexpr auto operator()(const R& r) const
			STL2_NOEXCEPT_RETURN(
				size(r)
			)
			template <class R>
			requires !has_member<R> && !has_non_member<R> && has_difference<R>
			constexpr auto operator()(const R& r) const
			STL2_NOEXCEPT_RETURN(
				__stl2::end(r) - __stl2::begin(r)
			)
		};
	}
	// Workaround GCC PR66957 by declaring this unnamed namespace inline.
	inline namespace {
		constexpr auto& size = detail::static_const<__size::fn>::value;
	}

	///////////////////////////////////////////////////////////////////////////
	// SizedRange [ranges.sized]
	//
	template <class R>
	constexpr bool disable_sized_range = false;

	template <class R>
	concept bool SizedRange =
		Range<R> &&
		!disable_sized_range<__uncvref<R>> &&
		requires(const remove_reference_t<R>& r) {
			{ __stl2::size(r) } -> Integral;
			{ __stl2::size(r) } -> difference_type_t<iterator_t<R>>;
		};

	namespace models {
		template <class>
		constexpr bool SizedRange = false;
		__stl2::SizedRange{R}
		constexpr bool SizedRange<R> = true;
	}

	///////////////////////////////////////////////////////////////////////////
	// dangling [dangling.wrap]
	//
	template <CopyConstructible T>
	class dangling {
		T value;
	public:
		constexpr dangling()
		noexcept(is_nothrow_default_constructible<T>::value)
		requires DefaultConstructible<T>
		: value{}
		{}
		constexpr dangling(T t)
		noexcept(is_nothrow_move_constructible<T>::value)
		: value(__stl2::move(t))
		{}
		constexpr T get_unsafe() const&
		noexcept(is_nothrow_copy_constructible<T>::value)
		{
			return value;
		}
		constexpr T get_unsafe() &&
		noexcept(is_nothrow_move_constructible<T>::value)
		{
			return __stl2::move(value);
		}
	};

	template <Range R>
	using safe_iterator_t =
		meta::if_<is_lvalue_reference<R>, iterator_t<R>, dangling<iterator_t<R>>>;
} STL2_CLOSE_NAMESPACE

#undef STL2_TREAT_RVALUES_AS_CONST

#endif
