/*!
 * \brief  Приведение числа к ближайшему целому числу из ряда Фибоначчи.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   17.08.2016
 */


#include <vector>
#include <iostream>

#pragma once

namespace utils {

class FibonacciReduction {
    std::vector<size_t> _fnums;

public:
    FibonacciReduction();

    size_t reduction(size_t in);

    void test() {
        for (const auto &f : _fnums) {
            std::cout << " - " << f << "\n" << std::flush;
        }
    }
};
} /// utils
