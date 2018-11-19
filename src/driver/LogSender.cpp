#include "Log.hpp"
#include "LogSender.hpp"


using namespace robocooler;
using namespace driver;


void LogSender::checkFileForSend() {
    /// Проверить наличие 2 - х файлов логов.
    std::vector<bfs::path> paths;
    bfs::directory_iterator end;
    for (bfs::directory_iterator i(_path); i not_eq end; ++i) {
        if (bfs::is_regular_file(*i)) {
            paths.push_back(*i);
        }
    }
    if (paths.size() >= 2) {
        bfs::path older;
        time_t lwt = time(NULL);
        /// Выбрать старший.
        for (auto f : paths) {
            if (bfs::last_write_time(f) < lwt and f.extension().string() == ".log") {
                older = f;
                lwt = bfs::last_write_time(f);
            }
        }
        if (not older.empty()) {
            /// Отправить файл логов на сервер.
            _send_func(older.string());
            /// Удалить отправленный файл.
            bfs::remove(older);
        }
    }
}


LogSender::LogSender(const std::string& path_, const LogSender::SendFunc& send_func_)
    : _path(path_)
    , _send_func(send_func_) {
    if (IS_LOG_TO_FILE and not path_.empty() and send_func_ and bfs::exists(_path) and bfs::is_directory(_path)) {
        _timer = std::make_shared<utils::Timer>(CHECK_LOG_SEND_TIMEOUT, [&,this] {
            checkFileForSend();
            _timer->restart();
        });
    } else {
        LOG(WARNING) << "can`t init file sender.";
    }
}


LogSender::~LogSender() {
    if (_timer) {
        /// Завершить работу таймера.
        _timer.reset();
        /// Выполнить стандартную отправку.
        checkFileForSend();
        /// Найти текущий файл логов.
        bfs::directory_iterator end;
        for (bfs::directory_iterator i(_path); i not_eq end; ++i) {
            bfs::path last(*i);
            if (last.extension().string() == ".log") {
                /// Отправить текущий файл логов на сервер.
                _send_func(last.string());
                /// Удалить текущий файл.
                bfs::remove(last);
            }
        }
    }
}
