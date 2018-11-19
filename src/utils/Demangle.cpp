#include <stdlib.h>
#include <cxxabi.h>

#include <memory>

#include "Demangle.hpp"

using namespace utils;

namespace details {

struct Free {
  template<class T>
  void operator() (T *p) const {
    free(p);
  }
};
} // namespace


Demangle::Demangle(const std::string &name) {
  int status = 0;
  std::unique_ptr<char, details::Free> dem_name(abi::__cxa_demangle(name.c_str(), nullptr, nullptr, &status));
  _demangle = (0 == status) ? dem_name.get() : name;
}


const std::string& Demangle::string() {
  return _demangle;
}
