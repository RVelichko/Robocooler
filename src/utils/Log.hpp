/*!
 * \brief  Система логирования.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   02.03.2013
 */

#pragma once

#include <sys/time.h>

#include <cstdint>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <typeinfo>
#include <tuple>
#include <array>
#include <memory>
#include <thread>
#include <atomic>

#include "Singleton.hpp"


namespace utils {

static const uint32_t LOG_FILE_DEPTH = 1024 * 1024;


class Log {
public:
    typedef std::function<void(const char*)> ExtOutFunc;
    typedef struct timeval Timeval;

    enum class Level {
        _test,
        _debug,
        _trace,
        _info,
        _warning,
        _error,
        _fatal,
        _quantity
    };
    
private:
    typedef std::tuple<Level, std::string, std::string, Timeval> QueueTask;
    typedef std::queue<QueueTask> Queue;
    typedef std::array<bool, static_cast<size_t>(Level::_quantity)> ToggleArray;

    
    ToggleArray _toggle_levels;

    std::ofstream _file;
    
    size_t _file_number;
    size_t _file_size;
    size_t _file_line_number;
    
    Queue _queue;
    std::mutex _mutex;
    std::condition_variable _cond;
    
    std::shared_ptr<std::thread> _thread;
    std::atomic_bool _is_run;

    ExtOutFunc _ext_out_func;
    
    bool _is_log_out;
    bool _is_log_out_file;
    bool _full_out_to_ext_func;
    size_t _log_file_depth;
    
    virtual void execute();
    virtual void handleCancel();
    
    virtual void onStop()
    {}
    
    void open();
    void close();

public:
    Log();
    ~Log();
    
    void init(bool is_log_out_, bool is_log_out_file_, size_t log_file_depth_ = LOG_FILE_DEPTH);
    void initExtFunc(ExtOutFunc &&ext_out_func_, bool full_out);
    void print(const Level& level_, const std::string& module_, const std::string& message_);
    
    void start();
    void stop();

    void toggle(const Level& level_, bool is_on_);

    bool isLogOutFile() const;
};


struct LogSequence {
    struct Head {
        struct Next {
            std::stringstream *_stream;
            
            template<class Type>
            explicit Next(std::stringstream *stream_, const Type &value_)
                : _stream(stream_) {
                (*_stream) << value_;
            }
            
            Next(const Next &next_);
            
            template<class Type>
            Next operator << (const Type &value_) {
                return Next(_stream, value_);
            }
        };
    
        std::shared_ptr<std::stringstream> _stream;
        Log::Level _level;
        std::string _module;
        
        template<class Type>
        explicit Head(const Log::Level &level_, const std::string &module_, const Type &value_)
            : _stream(new std::stringstream)
            , _level(level_)
            , _module(module_) {
            (*_stream) << value_;
        }
        
        Head(const Head &head_);
        
        ~Head() {
            Singleton<Log>::getShared()->print(_level, _module, _stream->str().c_str());
        }
        
        template<class Type>
        Next operator << (const Type &value_) {
            return Next(_stream.get(), value_);
        }
    };
    
    Log::Level _level;
    std::string _module;
    
    LogSequence(const Log::Level &level_, const std::string &module_);
    
    template<class Type>
    Head operator << (const Type &value_) {
        return Head(_level, _module, value_);
    }
};

} /// utils


#define AFTERX(name, x) name ## x
#define XAFTERX(name, x) AFTERX(name, x)

#ifndef _MSC_VER
#define __FUNC__ __PRETTY_FUNCTION__
#else // _MSC_VER
#define __FUNC__ __FUNCTION__
#endif // _MSC_VER

#define METHOD (__FUNC__)
#define MODULE typeid(*this).name()

#define LOG_TO_FUNC(func, full_out) utils::Singleton<utils::Log>::getShared()->initExtFunc((func), (full_out));
#define LOG_TO_STDOUT utils::Singleton<utils::Log>::getShared()->init(true, false);

#define IS_LOG_TO_FILE utils::Singleton<utils::Log>::getShared()->isLogOutFile()

#define LOGM(level, method) utils::LogSequence XAFTERX(log_, __LINE__)((level), (method)); XAFTERX(log_, __LINE__) << ""
#define LOG(level) LOGM((level), METHOD)

#define LOG_TOGGLE(level, is_on) utils::Singleton<utils::Log>::getShared()->toggle((level), (is_on));

#define TEST   utils::Log::Level::_test
#define DEBUG   utils::Log::Level::_debug
#define TRACE   utils::Log::Level::_trace
#define INFO    utils::Log::Level::_info
#define WARNING utils::Log::Level::_warning
#define ERROR   utils::Log::Level::_error
#define FATAL   utils::Log::Level::_fatal
