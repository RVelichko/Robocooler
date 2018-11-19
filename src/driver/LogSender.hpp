/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Класс, выполняющий отправку файлов логов на сервер.
 * \author Величко Ростислав
 * \date   06.21.2017
 */

#pragma once

#include <memory>
#include <thread>
#include <functional>
#include <string>
#include <atomic>

#include <boost/filesystem.hpp>

#include "Timer.hpp"


#define CHECK_LOG_SEND_TIMEOUT 5000

namespace bfs = boost::filesystem;

namespace robocooler {
namespace driver {

class LogSender {
    typedef std::shared_ptr<utils::Timer> PTimer;
    typedef std::shared_ptr<std::thread> PThread;

public:
    typedef std::function<void(const std::string)> SendFunc; ///< Функтор отправки файла по имени.

private:
    PTimer _timer;
    bfs::path _path;
    SendFunc _send_func;

    void checkFileForSend();

public:
    explicit LogSender(const std::string& path_, const LogSender::SendFunc& send_func_);
    virtual ~LogSender();
};
} /// driver
} /// robocooler
