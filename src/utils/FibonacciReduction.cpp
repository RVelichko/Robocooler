/*!
 * \brief  Приведение числа к ближайшему целому числу из ряда Фибоначчи.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   17.08.2016
 */

#include <algorithm>

#include "FibonacciReduction.hpp"

using namespace utils;


FibonacciReduction::FibonacciReduction() {
    _fnums.push_back(0);
    _fnums.push_back(1);
    /// сумма (0 + 1) - является избыточной для задачи приведения
    _fnums.push_back(2);
}


size_t FibonacciReduction::reduction(size_t in) {
    auto it = (--_fnums.end());
    if (*it < in) { /// Добавить числа если входное значение превосходит последнее вычесленное
        do {
            _fnums.push_back(*it + *(--it));
            it = (--_fnums.end());
        } while (*it < in);
    } else { /// Найти число Фи, ближайшее большее входного числа
        it = find_if(_fnums.begin(), _fnums.end(), [in](size_t f) {
            return in < f;
        });
    }
    size_t res = *it; /// Запомнить верхнее число Фи относительно переданного
    double k = (static_cast<double>(res) + static_cast<double>(*(--it))) / 2.0; /// Определить границу приведения к ближайшему Фи
    if (static_cast<double>(in) < k) { /// Если ближе к меньшему Фи - выбрать меньшее
        res = *it;
    }
    return res;
}
