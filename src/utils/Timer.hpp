/*!
 * \brief  Обёртка для работы с таймером.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   02.03.2013
 */

#pragma once

#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <algorithm>
#include <condition_variable>

namespace utils {

namespace chr = std::chrono;

/// Шаг асинхронного ожидания истечения заданного времени в милисекундах
static const size_t ASYNC_THRED_STEP = 10; // mlsec


/**
 * Таймер с корректной обработкой и остановкой потока.
 */
class Timer {
    typedef std::thread Thread;
    typedef std::shared_ptr<Thread> PThread;
    typedef std::condition_variable Condition;
    typedef std::shared_ptr<Condition> PCondition;

    std::atomic_bool _is_restart;         ///< Атомарный флаг перезапуска.
    std::atomic_bool _is_run;             ///< Атомарный флаг.
    PThread _thread;                      ///< Поток таймера.
    std::mutex _mutex;                    ///< Объект синхронизации доступа к стартовому времени таймера.
    chr::steady_clock::time_point _start; ///< Начальное время таймера.
    PCondition _wait_condition;           ///< Условная переменная синхронизации для реализации ожидания срабатывания таймера.
    size_t _mlsleep;                       ///< Промежуток времени, обслужываемый таймером.

public:
    /**
     * Шаблонный конструктор объекта асинхронного таймера с периодическим освобождением потока.
     * \param  mlsleep_  Интервал задержки до выполненения переданной функции в милисекундах.
     * \param  callback_ Выполняемая функция.
     * \param  аrgs_     Изменяемый набор параметров выполняемой функции.
     */
    template<class Callable, class... Arguments>
    Timer(size_t mlsleep_, Callable &&callback_, Arguments&&... args_)
        : _mlsleep(mlsleep_) {
        /// Функтор с переменным числом параметров.
        auto task(std::bind(std::forward<Callable>(callback_), std::forward<Arguments>(args_)...));
        /// Запуск асинхронного потока.
        _thread = std::shared_ptr<Thread>(new Thread([=] {
            /// Инициализация запуска.
            _is_run = true;
            _is_restart = true;
            /// Получить время отсчёта таймера.
            _start = chr::steady_clock::now();
            /// Запустить цикл таймера до вызова деструктора.
            while (_is_run) {
                /// Заснуть на время не больше пошагового интервала.
                std::this_thread::sleep_for(chr::milliseconds(std::min(ASYNC_THRED_STEP, mlsleep_)));
                /// Проверить готовность таймера.
                chr::nanoseconds diff;
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    diff = chr::duration_cast<chr::nanoseconds>(chr::steady_clock::now() - _start);
                }
                if (_is_restart) {
                    size_t diff_timeout = static_cast<size_t>(std::chrono::duration_cast<chr::milliseconds>(diff).count());
                    if (mlsleep_ <= diff_timeout) {
                        /// Сбросить атомарный флага перезапуска.
                        _is_restart = false;
                        /// Выполнить задачу.
                        task();
                        /// Просигнализировать ожидающую переменную.
                        if (_wait_condition) {
                            _wait_condition->notify_one();
                        }
                    }
                }
            }
        }), [this](Thread *p_) {
            p_->join();
        });
    }

    ~Timer() {
        /// Корректное уничтожение запущенного потока.
        _is_restart = false;
        _is_run = false;
    }

    void executeNow() {
        if (_thread) {
            std::unique_lock<std::mutex> lock(_mutex);
            _start += chr::milliseconds(_mlsleep);
        }
    }

    void wait() {
        /// Ожидание завершения выполнения потока.
        if (_thread) {
            /// Проинициализировать условную переменную по требования.
            _wait_condition = std::make_shared<std::condition_variable>();
            /// Активировать ожидание нотификации.
            std::unique_lock<std::mutex> lock(_mutex);
            _wait_condition->wait(lock);
        }
    }

    void restart() {
        if (_thread) {
            /// Активировать атомарный флага перезапуска.
            _is_restart = true;
            /// обновить время запуска таймера.
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _start = chr::steady_clock::now();
            }
        }
    }
};
} /// utils
