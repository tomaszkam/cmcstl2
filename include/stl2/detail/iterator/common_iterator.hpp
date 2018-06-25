// cmcstl2 - A concept-enabled C++ standard library
//
//  Copyright Eric Niebler 2015
//  Copyright Casey Carter 2015
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/caseycarter/cmcstl2
//
#ifndef STL2_DETAIL_ITERATOR_COMMON_ITERATOR_HPP
#define STL2_DETAIL_ITERATOR_COMMON_ITERATOR_HPP

#include <stl2/type_traits.hpp>
#include <stl2/detail/fwd.hpp>
#include <stl2/detail/variant.hpp>
#include <stl2/detail/concepts/compare.hpp>
#include <stl2/detail/concepts/core.hpp>
#include <stl2/detail/concepts/object.hpp>
#include <stl2/detail/iterator/basic_iterator.hpp>
#include <stl2/detail/iterator/concepts.hpp>
#include <stl2/detail/iterator/operations.hpp>
#include <stl2/detail/memory/addressof.hpp>

///////////////////////////////////////////////////////////////////////////
// common_iterator [common.iterators]
//
STL2_OPEN_NAMESPACE {
	template <Iterator I, Sentinel<I> S>
		requires !Same<I, S>
	class common_iterator;

	namespace __common_iterator {
		template <class T>
		class operator_arrow_proxy {
			T value_;
		public:
			template <class U>
			constexpr explicit operator_arrow_proxy(U&& u)
			noexcept(std::is_nothrow_constructible_v<T, U>)
			requires Constructible<T, U>
			: value_(std::forward<U>(u)) {}

			constexpr const T* operator->() const noexcept {
				return detail::addressof(value_);
			}
		};

		template <class I>
		requires
			Readable<const I> &&
			(std::is_pointer_v<I> ||
				requires(const I& i) { i.operator->(); })
		constexpr I operator_arrow_(const I& i, ext::priority_tag<2>)
		noexcept(std::is_nothrow_copy_constructible_v<I>) {
			return i;
		}

		template <class I>
		requires
			Readable<const I> &&
			_Is<reference_t<const I>, std::is_reference>
		constexpr auto operator_arrow_(const I& i, ext::priority_tag<1>)
		noexcept(noexcept(*i)) {
			auto&& tmp = *i;
			return detail::addressof(tmp);
		}

		template <class I>
		requires
			Readable<const I> &&
			!std::is_reference_v<reference_t<I>> &&
			Constructible<value_type_t<I>, reference_t<I>>
		constexpr auto operator_arrow_(const I& i, ext::priority_tag<0>)
		noexcept(
			std::is_nothrow_move_constructible_v<
				operator_arrow_proxy<value_type_t<I>>> &&
			std::is_nothrow_constructible_v<
				operator_arrow_proxy<value_type_t<I>>, reference_t<I>>) {
			return operator_arrow_proxy<value_type_t<I>>{*i};
		}

		template <class I>
		requires
			Readable<const I> &&
			requires(const I& i) {
				__common_iterator::operator_arrow_(i, ext::max_priority_tag);
			}
		constexpr decltype(auto) operator_arrow(const I& i)
		STL2_NOEXCEPT_RETURN(
			__common_iterator::operator_arrow_(i, ext::max_priority_tag)
		)

		struct access {
			template <_SpecializationOf<common_iterator> C>
			static constexpr decltype(auto) v(C&& c) noexcept {
				return (std::forward<C>(c).v_);
			}
		};

		template <Iterator I, Sentinel<I> S, ConvertibleTo<I> II, ConvertibleTo<S> SS>
		struct convert_visitor {
			constexpr auto operator()(const II& i) const
			STL2_NOEXCEPT_RETURN(
				std::variant<I, S>{std::in_place_type<I>, i}
			)
			constexpr auto operator()(const SS& s) const
			STL2_NOEXCEPT_RETURN(
				std::variant<I, S>{std::in_place_type<S>, s}
			)
		};

		template <Iterator I, Sentinel<I> S, ConvertibleTo<I> II, ConvertibleTo<S> SS>
		struct assign_visitor {
			std::variant<I, S>& v_;

			void operator()(I& lhs, const II& rhs) const
			STL2_NOEXCEPT_RETURN(
				(void)(lhs = rhs)
			)
			void operator()(S& lhs, const SS& rhs) const
			STL2_NOEXCEPT_RETURN(
				(void)(lhs = rhs)
			)
			void operator()(I&, const SS& s) const
			STL2_NOEXCEPT_RETURN(
				(void)v_.template emplace<S>(s)
			)
			void operator()(S&, const II& i) const
			STL2_NOEXCEPT_RETURN(
				(void)v_.template emplace<I>(i)
			)
		};

		template <Iterator I1, class S1, Iterator I2, Sentinel<I1> S2>
		requires Sentinel<S1, I2>

		struct equal_visitor {
			constexpr bool operator()(const I1&, const I2&) const noexcept {
				return true;
			}
			constexpr bool operator()(const I1& i1, const I2& i2) const
			noexcept(noexcept(bool(i1 == i2)))
			requires EqualityComparableWith<I1, I2>
			{
				return i1 == i2;
			}
			template<class L, class R>
			constexpr bool operator()(const L& lhs, const R& rhs) const
			STL2_NOEXCEPT_RETURN(
				bool(lhs == rhs)
			)
			constexpr bool operator()(const S1&, const S2&) const noexcept {
				return true;
			}
		};

		template<class I, class S, SizedSentinel<I> I2, SizedSentinel<I> S2>
		requires SizedSentinel<S, I2>
		struct difference_visitor {
			template<class L, class R>
			constexpr difference_type_t<I2> operator()(
				const L& lhs, const R& rhs) const
			STL2_NOEXCEPT_RETURN(
				static_cast<difference_type_t<I2>>(lhs - rhs)
			)
			constexpr difference_type_t<I2> operator()(
				const S&, const S2&) const noexcept
			{
				return 0;
			}
		};
	}

	template <Iterator I, Sentinel<I> S>
		requires !Same<I, S>
	class common_iterator {
		friend __common_iterator::access;
		std::variant<I, S> v_;
	public:
		using difference_type = difference_type_t<I>;

		constexpr common_iterator() = default;

		constexpr common_iterator(I i)
		noexcept(std::is_nothrow_constructible_v<std::variant<I, S>, I>)
		: v_{std::in_place_type<I>, std::move(i)} {}

		constexpr common_iterator(S s)
		noexcept(std::is_nothrow_constructible_v<std::variant<I, S>, S>)
		: v_{std::in_place_type<S>, std::move(s)} {}

		template <ConvertibleTo<I> II, ConvertibleTo<S> SS>
		constexpr common_iterator(const common_iterator<II, SS>& i)
		noexcept(
			std::is_nothrow_constructible_v<I, const II&> &&
			std::is_nothrow_constructible_v<S, const SS&>)
		: v_{(STL2_EXPECT(!__common_iterator::access::v(i).valueless_by_exception()),
			std::visit(
				__common_iterator::convert_visitor<I, S, II, SS>{},
				__common_iterator::access::v(i)))}
		{}

		template <ConvertibleTo<I> II, ConvertibleTo<S> SS>
		common_iterator& operator=(const common_iterator<II, SS>& i)
		noexcept(
			std::is_nothrow_assignable_v<I&, const II&> &&
			std::is_nothrow_assignable_v<S&, const SS&>)
		{
			STL2_EXPECT(!__common_iterator::access::v(i).valueless_by_exception());
			std::visit(
				__common_iterator::assign_visitor<I, S, II, SS>{v_},
				v_,
				__common_iterator::access::v(i));
			return *this;
		}

		decltype(auto) operator*()
		noexcept(noexcept(*std::declval<I&>())) {
			return *__stl2::__get_unchecked<I>(v_);
		}
		decltype(auto) operator*() const
		noexcept(noexcept(*std::declval<const I&>()))
		requires detail::Dereferenceable<const I> {
			return *__stl2::__get_unchecked<I>(v_);
		}
		decltype(auto) operator->() const
		noexcept(noexcept(__common_iterator::operator_arrow(std::declval<const I&>())))
		requires Readable<const I> {
			return __common_iterator::operator_arrow(__stl2::__get_unchecked<I>(v_));
		}

		common_iterator& operator++()
		noexcept(noexcept(++declval<I&>()))	{
			++__stl2::__get_unchecked<I>(v_);
			return *this;
		}

		decltype(auto) operator++(int)
		noexcept(noexcept((decltype(declval<I&>()++))declval<I&>()++)) {
			return __stl2::__get_unchecked<I>(v_)++;
		}
		common_iterator operator++(int)
		noexcept(noexcept(common_iterator(common_iterator(++declval<common_iterator&>()))))
		requires ForwardIterator<I> {
			STL2_EXPECT(std::holds_alternative<I>(v_));
			auto tmp(*this);
			++__stl2::__get_unchecked<I>(v_);
			return tmp;
		}

		template <class I2, Sentinel<I> S2>
		requires Sentinel<S, I2>
		friend bool operator==(
			const common_iterator& x, const common_iterator<I2, S2>& y)
		{
			STL2_EXPECT(!x.v_.valueless_by_exception());
			STL2_EXPECT(!__common_iterator::access::v(y).valueless_by_exception());
			return std::visit(
				__common_iterator::equal_visitor<I, S, I2, S2>{},
				x.v_, __common_iterator::access::v(y));
		}

		template <class I2, Sentinel<I> S2>
		requires Sentinel<S, I2>
		friend bool operator!=(
			const common_iterator& x, const common_iterator<I2, S2>& y)
		{
			return !(x == y);
		}

		template <SizedSentinel<I> I2, SizedSentinel<I> S2>
		requires SizedSentinel<S, I2>
		friend difference_type_t<I2> operator-(
			const common_iterator& x, const common_iterator<I2, S2>& y)
		{
			STL2_EXPECT(!x.v_.valueless_by_exception());
			STL2_EXPECT(!__common_iterator::access::v(y).valueless_by_exception());
			return std::visit(
				__common_iterator::difference_visitor<I, S, I2, S2>{},
				x.v_, __common_iterator::access::v(y));
		}

		friend rvalue_reference_t<I> iter_move(const common_iterator& i)
			noexcept(noexcept(__stl2::iter_move(std::declval<const I&>())))
			requires InputIterator<I> {
			return __stl2::iter_move(__stl2::__get_unchecked<I>(i.v_));
		}

		template <IndirectlySwappable<I> I2, class S2>
		friend void iter_swap(const common_iterator& x,
			const common_iterator<I2, S2>& y)
			noexcept(noexcept(__stl2::iter_swap(std::declval<const I&>(),
				std::declval<const I2&>()))) {
			__stl2::iter_swap(__stl2::__get_unchecked<I>(x.v_),
				__stl2::__get_unchecked<I2>(__common_iterator::access::v(y)));
		}
	};

	template <Readable I, class S>
	struct value_type<common_iterator<I, S>> {
		using type = value_type_t<I>;
	};
	template <InputIterator I, class S>
	struct iterator_category<common_iterator<I, S>> {
		using type = input_iterator_tag;
	};
	template <ForwardIterator I, class S>
	struct iterator_category<common_iterator<I, S>> {
		using type = forward_iterator_tag;
	};
} STL2_CLOSE_NAMESPACE

#endif
