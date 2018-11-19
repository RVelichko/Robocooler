/*!
 * \brief  Потокозащищённая обёртка для уникального объекта.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   02.03.2013
 */

#pragma once

#include <memory>
#include <thread>
#include <mutex>

#include <boost/noncopyable.hpp>


namespace utils {

template<class T>
class Singleton : private boost::noncopyable {
public:
    static std::shared_ptr<T>& getShared() {
        static std::shared_ptr<T> only_one_instance;
        static std::mutex mutex;
        if (not only_one_instance.get()) {
            std::lock_guard<std::mutex> lock(mutex);
            if (not only_one_instance) {
                only_one_instance.reset(new T());
            }
        }
        return only_one_instance;
    }
};
} // utils
