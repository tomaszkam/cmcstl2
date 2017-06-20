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
#ifndef STL2_DETAIL_ITERATOR_OPERATIONS_HPP
#define STL2_DETAIL_ITERATOR_OPERATIONS_HPP

#include <stl2/detail/fwd.hpp>
#include <stl2/detail/iterator/concepts.hpp>
#include <stl2/detail/concepts/object.hpp>

///////////////////////////////////////////////////////////////////////////
// Iterator operations [iterator.operations]
// Not to spec: All constexpr per P0579.
STL2_OPEN_NAMESPACE {
	Iterator{I} class counted_iterator;

	// advance
	struct __advance_fn {
		Iterator{I}
		static constexpr void impl(I& i, difference_type_t<I> n)
		noexcept(noexcept(++i))
		{
			// Pre: 0 <= n && [i,i+n)
			STL2_EXPECT(0 <= n);
			while (n != 0) {
				--n;
				++i;
			}
		}

		Sentinel{S, I}
		static constexpr difference_type_t<I>
		impl(I& i, difference_type_t<I> n, S bound)
		noexcept(noexcept(++i != bound))
		{
			// Pre: 0 == n || (0 < n && [i,bound))
			STL2_EXPECT(0 <= n);
			while (n != 0 && i != bound) {
				++i;
				--n;
			}
			return n;
		}

		Iterator{I}
		constexpr void operator()(I& i, difference_type_t<I> n) const
		noexcept(noexcept(++i))
		{
			// Pre: 0 <= n && [i,i+n)
			impl(i, n);
		}

		BidirectionalIterator{I}
		constexpr void operator()(I& i, difference_type_t<I> n) const
		noexcept(noexcept(++i, --i))
		{
			// Pre: 0 <= n ? [i,i+n) : [i+n,i)
			if (0 <= n) {
				impl(i, n);
			} else {
				do {
					++n;
					--i;
				} while (n != 0);
			}
		}

		RandomAccessIterator{I}
		constexpr void operator()(I& i, difference_type_t<I> n) const
		STL2_NOEXCEPT_RETURN(
			// Pre: 0 <= n ? [i,i+n) : [i+n,i)
			(void)(i += n)
		)

		Sentinel{S, I}
		constexpr void operator()(I& i, S bound) const
		noexcept(noexcept(++i != bound))
		{
			// Pre: [i,bound)
			while (i != bound) {
				++i;
			}
		}

		template <class S, class I>
		requires
			Sentinel<S, I> && Assignable<I&, S&&>
		constexpr void operator()(I& i, S bound) const
		STL2_NOEXCEPT_RETURN(
			(void)(i = std::move(bound))
		)

		template <class S, class I>
		requires
			Sentinel<S, I> && !Assignable<I&, S&&> &&
			SizedSentinel<S, I>
		constexpr void operator()(I& i, S bound) const
		noexcept(noexcept(std::declval<const __advance_fn&>()(i, bound - i)))
		{
			// Pre: [i,bound)
			difference_type_t<I> d = bound - i;
			STL2_EXPECT(0 <= d);
			(*this)(i, d);
		}

		Sentinel{S, I}
		constexpr difference_type_t<I>
		operator()(I& i, difference_type_t<I> n, S bound) const
		STL2_NOEXCEPT_RETURN(
			// Pre: 0 == n || (0 < n && [i,bound))
			impl(i, n, bound)
		)

		template <class S, class I>
		requires
			Sentinel<S, I> && SizedSentinel<S, I>
		constexpr difference_type_t<I>
		operator()(I& i, difference_type_t<I> n, S bound) const
		noexcept(noexcept(
			std::declval<const __advance_fn&>()(i, std::move(bound)),
			std::declval<const __advance_fn&>()(i, bound - i)))
		{
			// Pre: 0 <= n && [i,bound)
			STL2_EXPECT(0 <= n);
			auto d = difference_type_t<I>{bound - i};
			STL2_EXPECT(0 <= d);
			if (d <= n) {
				(*this)(i, std::move(bound));
				return n - d;
			}
			(*this)(i, n);
			return 0;
		}

		BidirectionalIterator{I}
		constexpr difference_type_t<I>
		operator()(I& i, difference_type_t<I> n, I bound) const
		noexcept(noexcept(
			impl(i, n, bound),
			--i != bound))
		{
			// Pre: 0 == n || (0 < n ? [i,bound) : [bound,i))
			if (0 <= n) {
				return impl(i, n, bound);
			}

			do {
				--i;
				++n;
			} while (n != 0 && i != bound);
			return n;
		}

		template <class I>
		requires
			BidirectionalIterator<I> && SizedSentinel<I, I>
		constexpr difference_type_t<I>
		advance(I& i, difference_type_t<I> n, I bound)
		noexcept(noexcept(
			i = std::move(bound),
			std::declval<const __advance_fn&>()(i, bound - i)))
		{
			// Pre: 0 == n ? ([i,bound) || [bound,i)) : (0 < n ? [i,bound) : [bound,i))
			auto d = difference_type_t<I>{bound - i};
			STL2_EXPECT(0 <= n ? 0 <= d : 0 >= d);
			if (0 <= n ? d <= n : d >= n) {
				i = std::move(bound);
				return n - d;
			}
			(*this)(i, n);
			return 0;
		}

		// Not to spec: constexpr per P0579
		template <Iterator I, class Self = __advance_fn>
		constexpr void operator()(counted_iterator<I>& i, difference_type_t<I> n) const
		noexcept(noexcept(std::declval<const Self&>()(std::declval<I&>(), n)));

		// Not to spec: constexpr per P0579
		RandomAccessIterator{I}
		constexpr void operator()(counted_iterator<I>& i, difference_type_t<I> n) const
		noexcept(noexcept(std::declval<I&>() += n));
	};
	// Workaround GCC PR66957 by declaring this unnamed namespace inline.
	inline namespace {
		constexpr auto& advance = detail::static_const<__advance_fn>::value;
	}

	// next
	struct __next_fn {
		Iterator{I}
		constexpr I operator()(I x) const
		STL2_NOEXCEPT_RETURN(
			++x
		)

		Iterator{I}
		constexpr I operator()(I x, difference_type_t<I> n) const
		STL2_NOEXCEPT_RETURN(
			__stl2::advance(x, n),
			x
		)

		Sentinel{S, I}
		constexpr I operator()(I x, S bound) const
		STL2_NOEXCEPT_RETURN(
			__stl2::advance(x, std::move(bound)),
			x
		)

		Sentinel{S, I}
		constexpr I operator()(I x, difference_type_t<I> n, S bound) const
		STL2_NOEXCEPT_RETURN(
			__stl2::advance(x, n, std::move(bound)),
			x
		)
	};
	// Workaround GCC PR66957 by declaring this unnamed namespace inline.
	inline namespace {
		constexpr auto& next = detail::static_const<__next_fn>::value;
	}

	// prev
	struct __prev_fn {
		BidirectionalIterator{I}
		constexpr I operator()(I x) const
		STL2_NOEXCEPT_RETURN(
			--x
		)

		BidirectionalIterator{I}
		constexpr I operator()(I x, difference_type_t<I> n) const
		STL2_NOEXCEPT_RETURN(
			__stl2::advance(x, -n),
			x
		)

		BidirectionalIterator{I}
		constexpr I operator()(I x, difference_type_t<I> n, I bound) const
		STL2_NOEXCEPT_RETURN(
			__stl2::advance(x, -n, std::move(bound)),
			x
		)
	};
	// Workaround GCC PR66957 by declaring this unnamed namespace inline.
	inline namespace {
		constexpr auto& prev = detail::static_const<__prev_fn>::value;
	}

	namespace ext {
		Sentinel{S, I}
		constexpr tagged_pair<tag::count(difference_type_t<I>), tag::end(I)>
		enumerate(I first, S last)
		noexcept(noexcept(++first != last) &&
			std::is_nothrow_move_constructible<I>::value)
		{
			difference_type_t<I> n = 0;
			while (first != last) {
				++n;
				++first;
			}
			return {n, std::move(first)};
		}

		SizedSentinel{S, I}
		constexpr tagged_pair<tag::count(difference_type_t<I>), tag::end(I)>
		enumerate(I first, S last)
		noexcept(noexcept(__stl2::next(std::move(first), std::move(last))) &&
			std::is_nothrow_move_constructible<I>::value)
		{
			auto d = last - first;
			STL2_EXPECT((models::Same<I, S> || d >= 0));
			return {d, __stl2::next(std::move(first), std::move(last))};
		}

		template <class S, class I>
		requires
			Sentinel<S, I> && !SizedSentinel<S, I> && SizedSentinel<I, I>
		constexpr tagged_pair<tag::count(difference_type_t<I>), tag::end(I)>
		enumerate(I first, S last)
		noexcept(noexcept(__stl2::next(first, std::move(last))) &&
			std::is_nothrow_move_constructible<I>::value)
		{
			auto end = __stl2::next(first, std::move(last));
			auto n = end - first;
			return {n, std::move(end)};
		}
	}

	// distance
	Sentinel{S, I}
		// Pre: [first, last)
	constexpr difference_type_t<I> distance(I first, S last)
	STL2_NOEXCEPT_RETURN(
		ext::enumerate(std::move(first), std::move(last)).first
	)

	SizedSentinel{S, I}
		// Pre: [first, last)
	constexpr difference_type_t<I> distance(I first, S last)
	noexcept(noexcept(last - first))
	{
		auto d = last - first;
		STL2_EXPECT(d >= 0);
		return d;
	}

	template <class I>
	requires
		SizedSentinel<I, I>
		// Pre: [first, last) || [last, first)
	constexpr difference_type_t<I> distance(I first, I last)
	STL2_NOEXCEPT_RETURN(
		last - first
	)
} STL2_CLOSE_NAMESPACE

#endif
