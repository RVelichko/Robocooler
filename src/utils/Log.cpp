#include <dirent.h>
#include <sys/stat.h>

#include <thread>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <stdexcept>
#include <iomanip>

#include "Log.hpp"

using namespace utils;

#ifndef LOG_MAX_QUEUE_SIZE
# define LOG_MAX_QUEUE_SIZE std::numeric_limits<std::uint8_t>::max()
#endif


std::string level_name(const Log::Level &value) {
    switch (value) {
        case TEST:    return "TEST";
        case DEBUG:   return "DEBUG";
        case TRACE:   return "TRACE";
        case INFO:    return "INFO";
        case WARNING: return "WARNING";
        case ERROR:   return "ERROR";
        case FATAL:   return "FATAL";
        default: 
            throw std::runtime_error("LOD ERROR: Udeclared level");
    }
}


static std::string TimevalToStr(Log::Timeval timeval_) {
    struct tm * ptm = localtime(&timeval_.tv_sec);
    char buf[32] = {0};
    /// Format: 15-06-2009 20:20:00.123456
    strftime(buf, 32, "%d-%m-%Y %H:%M:%S", ptm);
    return std::string(buf) + "." + std::to_string(timeval_.tv_usec);
}


static std::string LogFileName(size_t i) {
    Log::Timeval tv;
    gettimeofday(&tv, NULL);
    return (TimevalToStr(tv) + "-" + std::to_string(i) + ".log");
}


Log::Log()
    : _file_number(0)
    , _file_size(std::numeric_limits<size_t>::max())
    , _file_line_number(0)
    , _is_run(false)
    , _is_log_out(false)
    , _is_log_out_file(true)
    , _log_file_depth(LOG_FILE_DEPTH) {
    for (size_t l = 0; l < static_cast<size_t>(Level::_quantity); ++l) {
        _toggle_levels[l] = true;
    }
    start();
}


Log::~Log() {
    stop();
    std::cout << "Log is stopped.\n" << std::flush;
}


void Log::init(bool is_log_out, bool is_log_out_file, size_t log_file_depth) {
    _is_log_out      = is_log_out;
    _is_log_out_file = is_log_out_file;
    _log_file_depth  = LOG_FILE_DEPTH;
}


void Log::initExtFunc(ExtOutFunc &&ext_out_func_, bool full_out_) {
    _ext_out_func = std::move(ext_out_func_);
    _full_out_to_ext_func = full_out_;
}

void Log::execute() {
    while (true) {
        QueueTask task;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            while (_queue.empty()) {
                if (not _is_run) {
                    close();
                    return;
                }
                _cond.wait(lock);
            }
            task = _queue.front();
            _queue.pop();
        }
        Level level         = std::get<0>(task);
        std::string module  = std::get<1>(task);
        std::string message = std::get<2>(task);
        Timeval tv          = std::get<3>(task);
        std::stringstream ss;
        ss << (++_file_line_number)
           << ". [" << TimevalToStr(tv) << "]"
           << " [" << level_name(level) << "]"
           << " [" << module << "]";
        if (_is_log_out) {
            if (_ext_out_func) {
                if (_full_out_to_ext_func) {
                    _ext_out_func((ss.str() + " " + message).c_str());
                } else {
                    _ext_out_func(message.c_str());
                }
            }
            std::cout << ss.str() << " " << message << "\n" << std::flush;
        }
        if (_is_log_out_file) {
            if (_file_size >= _log_file_depth) {
                open();
            }
            
            if (_file.is_open()) {
                _file << ss.str();
                _file_size = static_cast<size_t>(_file.tellp());
            }
        }
    }
}


void Log::handleCancel() {
    _cond.notify_one();
}


void Log::open() {
    if (_file.is_open()) {
        close();
    }
    _file.open(LogFileName(_file_number));
}


void Log::close() {
    if (_file.is_open()) {
        _file.close();

        auto now = std::chrono::system_clock::now();
        struct dirent *epdf;
        DIR *dpdf = opendir("./");

        if (NULL not_eq dpdf) {
            epdf = readdir(dpdf);
            while (epdf) {
                if (DT_REG == epdf->d_type) {
                    std::string name = epdf->d_name;
                    std::string ext = name.substr();
                    auto pos = name.find(".");
                    
                    if ((pos not_eq std::string::npos) and (".log" == ext)) {
                        struct stat t_stat;
                        stat(name.c_str(), &t_stat);
                        struct tm *file_time = localtime(&t_stat.st_ctime);
                        auto mod = std::chrono::system_clock::from_time_t(mktime(file_time));
                        
                        if (mod + std::chrono::hours(_log_file_depth) < now) {
                            if (0 not_eq remove(name.c_str())) {
                                std::runtime_error("LOG ERROR: can`t remove file `" + name + "`.");
                            }
                        }
                    }
                }
                epdf = readdir(dpdf);
            }
        }
        ++_file_number;
    }
}


void Log::print(const Log::Level &level_, const std::string &module_, const std::string &message_) {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_is_run and _toggle_levels[static_cast<size_t>(level_)]) {
        //auto now = std::chrono::system_clock::now();
        //auto duration = now.time_since_epoch();
        //auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        Timeval tv;
        gettimeofday(&tv, NULL);

        if (_queue.size() < LOG_MAX_QUEUE_SIZE) {
            _queue.push(std::make_tuple(level_, module_, message_, tv)); // millis));
        } else {
            std::string err_msg = "Log max queue size exceeded! " + std::to_string(_queue.size()) + " messages were dropped.";
            Queue q;
            _queue.swap(q);
            _queue.push(std::make_tuple(ERROR, "LOG::print", err_msg, tv)); //millis));
        }
        _cond.notify_one();
    }
}


void Log::start() {
    if (_thread) {
        stop();
    }
    _is_run = true;
    _thread = std::make_shared<std::thread>(std::bind(&Log::execute, this));
}


void Log::stop() {
     _is_run = false;
    _cond.notify_one();

    if (_thread) {
        _thread->join();
        _thread.reset();
    }
}


void Log::toggle(const Level& level_, bool is_on_) {
    std::unique_lock<std::mutex> lock(_mutex);
    _toggle_levels[static_cast<size_t>(level_)] = is_on_;
}


bool Log::isLogOutFile() const {
    return _is_log_out_file;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


LogSequence::Head::Next::Next(const Next &next_)
  : _stream(next_._stream)
{}


LogSequence::Head::Head(const Head &head_)
  : _stream(head_._stream)
  , _level(head_._level)
  , _module(head_._module)
{}


LogSequence::LogSequence(const Log::Level &level_, const std::string &module_)
  : _level(level_)
  , _module(module_)
{}
