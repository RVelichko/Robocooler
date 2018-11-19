/*!
 * \brief  Оберка для обхода элементов "tuple".
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   02.03.2013
 */

#pragma once

#include <tuple>

namespace utils {

template<typename Callback, typename... Types>
class ForEachTuple {
  template<int index, typename IterCallback, typename... IterTypes>
  struct IterateTuple {
    void operator() (std::tuple<Types...> &ts, IterCallback callback) {
      callback(std::get<index>(ts));
      IterateTuple<index - 1, IterCallback, IterTypes...>{}(ts, callback);
    }
  };

  template<typename IterCallback, typename... IterTypes>
  struct IterateTuple<0, IterCallback, IterTypes...> {
    void operator() (std::tuple<IterTypes...> &ts, IterCallback callback) {
      callback(std::get<0>(ts));
    }
  };

public:
  ForEachTuple(std::tuple<Types...> &ts, Callback callback) {
    IterateTuple<std::tuple_size<std::tuple<Types...>>::value - 1, Callback, Types...> it;
    it(ts, callback);
  }
};
} // utils
