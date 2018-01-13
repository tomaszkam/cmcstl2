// cmcstl2 - A concept-enabled C++ standard library
//
//  Copyright Eric Niebler 2017
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/caseycarter/cmcstl2
//
#include <stl2/view/bounded.hpp>
#include <stl2/view/counted.hpp>
#include "../test_iterators.hpp"

int main() {
	using namespace __stl2;
	int rg[] = {0,1,2,3,4,5,6,7,8,9};
	auto x = view::counted(input_iterator(rg), 5) | view::bounded;
	//static_assert(models::MoveConstructible<decltype(x)>);
}
