#include <sstream>

#include "ThreadPool.hpp"
#include "Log.hpp"


using namespace utils;


ThreadPool::ThreadPool(size_t threads, const std::string &name)
    : _name(name)
    , _stop(false)
    , _threads(threads)
    , _gen_tasks(0)
    , _execd_tasks(0)
    , _max_tasks(0) {
    if (not threads) {
        threads = 1;
        LOG(DEBUG) << "Set num threads to default.";
    }

    for (std::size_t i = 0; i < threads; ++i) {
        _execd_tasks_counts.push_back(std::make_shared<std::uint64_t>(0));
    }

    for (std::size_t i = 0; i < threads; ++i) {
        std::uint64_t *p_execd_tasks = _execd_tasks_counts[i].get();
        _workers.emplace_back([this, p_execd_tasks]() {
            while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(_queue_mutex);

                while (not _stop and _tasks.empty()) {
                    _condition_add.notify_one();
                    _condition_exe.wait(lock);
                }

                if (_stop and _tasks.empty()) {
                    return;
                }

                task = _tasks.front();
                _tasks.pop();

                if (_max_tasks < (_gen_tasks - _execd_tasks)) {
                    _max_tasks = _gen_tasks - _execd_tasks;
                }
            }

            task();
            {
                std::unique_lock<std::mutex> lock(_queue_mutex);
                ++_execd_tasks;
            }
            ++*p_execd_tasks;
            _condition_add.notify_one();
            }
        });
    }
}


ThreadPool::~ThreadPool() {
    _stop = true;
    _condition_add.notify_one();
    _condition_exe.notify_all();

    std::stringstream counts_ss;

    for(size_t i = 0; i < _workers.size(); ++i) {
        _workers[i].join();
        counts_ss << "\n\tthread[" << i << "] = " << _execd_tasks_counts[i].get()[0] << ";";
    }
    LOG(DEBUG)
        << "Pool `" << _name << "` statistik:\n"
        << "\tloaded tasks   = " << _gen_tasks << ";\n"
        << "\texecd tasks    = " << _execd_tasks << ";\n"
        << "\tmax work tasks = " << _max_tasks << ";"
        << counts_ss.str();
}
