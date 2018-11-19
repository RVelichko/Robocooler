#include <regex>

#include "Strhex2Hex.hpp"

using namespace utils;


Strhex2Hex::Strhex2Hex(const std::string &str_data_) {
    /// Преобразовать в HEX.
    if (not str_data_.empty()) {
        std::regex regex{R"([\s,]+)"};
        std::sregex_token_iterator it{str_data_.begin(), str_data_.end(), regex, -1};
        std::vector<std::string> words{it, {}};
        for (std::string &str : words) {
            /// Удалить лишние пробелы.
            str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
            /// Обработать не пустую строку.
            if (not str.empty()) {
                /// Сохранить hex.
                _buf.push_back(static_cast<uint8_t>(std::stoi(str, nullptr, 16) & 0xff));
            }
        }
    }
}


Strhex2Hex::operator std::vector<uint8_t> () {
    return _buf;
}


Strhex2Hex::operator std::string () {
    std::stringstream ss;
    for (auto b : _buf) {
        ss << "0x" << std::hex << static_cast<uint16_t>(b) << " ";
    }
    return ss.str();
}
