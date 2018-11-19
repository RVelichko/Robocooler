#include <utility>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/optional/optional.hpp>

#include "Log.hpp"
#include "JsonExtractor.hpp"


using namespace robocooler;
using namespace driver;


JsonExtractor::JsonExtractor(const OnJsonFunc &on_json_fn_) 
    : _on_json_fn(on_json_fn_) {
}


void JsonExtractor::onMessage(const std::string &msg_) {
    //LOG(DEBUG) << "I:" << msg;
    typedef std::pair<size_t, size_t> Pair;
    typedef std::string::size_type SizeType;

    try {
        _buf += msg_;
        LOG(DEBUG) << _buf;
        std::vector<uint8_t> tmp(_buf.begin(), _buf.end());
        if (not tmp.empty()) {
            /// Найти крайние парные cкобки.
            std::vector<Pair> json_poses;
            for (size_t pos = 0; pos < tmp.size(); ++pos) {
                if (tmp[pos] == static_cast<uint8_t>('{')) { ///< Найти открывающую скобку.
                    _stack.push(pos);
                } else if (tmp[pos] == static_cast<uint8_t>('}')) { ///< Найти закрывающую скобку.
                    if (not _stack.empty()) {
                        size_t begin = _stack.top();
                        _stack.pop();
                        if (_stack.empty()) { ///< Если стек пустой в данном месте - закрывающая скобка найдена.
                            json_poses.push_back(std::make_pair(begin, pos + 1));
                        }
                    } else {
                        LOG(ERROR) << "Bad JSON data.";
                        clear();
                        break;
                    }
                }
            }
            /// Если найдены полные json - обработать их.
            if (not json_poses.empty()) {
                /// Обработать json - ы.
                for (Pair pair: json_poses) {
                    std::string json(reinterpret_cast<char*>(&tmp[pair.first]), reinterpret_cast<char*>(&tmp[pair.second]));
                    LOG(DEBUG) << "C=" << json;
                    _on_json_fn(json);
                }
                /// Очистить обработанную часть буфера.
                if (json_poses.rbegin()->second <= _buf.size()) {
                    _stack = Stack();
                    _buf = _buf.substr(static_cast<SizeType>(json_poses.rbegin()->second)); 
                    //LOG(DEBUG) << "B=" << _buf;
                }
            }
        }
    } catch (std::exception &e) {
        LOG(ERROR) << e.what();
    }
}


void JsonExtractor::clear() {
    _buf = "";
    _stack = Stack();
}
