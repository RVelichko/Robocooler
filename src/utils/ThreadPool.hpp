/*!
 * \brief  Пул потоков с механизмом автобалансировки производительности.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   02.03.2013
 */

#pragma once

#include <vector>
#include <atomic>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

#include "Log.hpp"


namespace utils {

class ThreadPool {
  std::string _name;

  std::vector<std::thread> _workers;
  std::queue<std::function<void()>> _tasks;

  std::mutex              _queue_mutex;
  std::condition_variable _condition_exe;
  std::condition_variable _condition_add;
  std::atomic_bool        _stop;

  std::uint64_t _threads;
  std::uint64_t _gen_tasks;
  std::uint64_t _execd_tasks;
  std::uint64_t _max_tasks;

  std::uint64_t _last_task_dt;

  std::vector<std::shared_ptr<std::uint64_t>> _execd_tasks_counts;

public:
    template<class Func, class... Args>
    void enqueue(Func &&f, Args&&... args) {
        if (not _stop) {
            std::function<void()> task(std::bind(std::forward<Func>(f), std::forward<Args>(args)...));
            {
                std::unique_lock<std::mutex> lock(_queue_mutex);
                ++_gen_tasks;
                _tasks.push([task]() { task(); });
            }
            _condition_exe.notify_one();
            {
                std::unique_lock<std::mutex> lock(_queue_mutex);
                if (not _tasks.empty()) {
                    _condition_add.wait(lock);
                }
            }
        }
    }

    ThreadPool(size_t threads = std::thread::hardware_concurrency(), const std::string &name = "undefined");
    ~ThreadPool();
};
} // utils

