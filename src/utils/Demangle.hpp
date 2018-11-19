/*!
 * \brief  Объект для преобразования имени функции из формата компилятора к обычному виду.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   02.03.2013
 */

#pragma once

#include <string>

namespace utils {

class Demangle {
  std::string _demangle;

public:
  Demangle(const std::string &name);

  const std::string& string();
};
} // utils
