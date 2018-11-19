/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Тестовое приложение для отладки объмена посредствам tty.
 * \author Величко Ростислав
 * \date   17.08.2017
 */

#include <iostream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include "Log.hpp"
#include "TtyIo.hpp"


class Strhex2Hex {
    std::vector<uint8_t> _buf;

public:
    explicit Strhex2Hex(const std::string &str_data_) {
        LOG(DEBUG);
        /// Преобразовать в HEX.
        if (not str_data_.empty()) {
            std::vector<std::string> strs;
            boost::split(strs, str_data_, boost::is_any_of(" "));
            for (std::string &str : strs) {
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

    operator std::vector<uint8_t> () {
        return _buf;
    }

    operator std::string () {
        std::stringstream ss;
        for (auto b : _buf) {
            ss << "0x" << std::hex << static_cast<uint16_t>(b) << " ";
        }
        return ss.str();
    }
};


#define DEFAULT_PORT_SPEED 115200
#define DEFAULT_RCV_BUFFER_SIZE 256

namespace bpo = boost::program_options;

/// tty-io -d "a0 03 ff 72  ec"

int main(int argc, char **argv) {
    LOG_TO_STDOUT;
    try {
        std::string tty;
        size_t speed;
        size_t rcv_buffer_size;
        bool is_parity;
        bool is_blocking;
        bool is_loopback;
        std::string hex_data_str;
        bpo::options_description desc("Тестовое приложение для проверки работы RFID. Пример: tty-io -d \"a0 03 ff 72  ec\"");
        desc.add_options()
          ("help,h", "Показать список параметров")
          ("tty,t", bpo::value<std::string>(&tty)->default_value("/dev/ttyUSB0"), "Последовательное устройство.")
          ("speed,s", bpo::value<size_t>(&speed)->default_value(DEFAULT_PORT_SPEED), "Указать скорость объмена данными.")
          ("is_parity,p", bpo::bool_switch(&is_parity)->default_value(false), "Включить проверку чётности единичных бит.")
          ("is_blocking,b", bpo::bool_switch(&is_blocking)->default_value(false), "Включить блокирующийся режим чтения порта.")
          ("is_loopback,l", bpo::bool_switch(&is_loopback)->default_value(false), "Включить кольцевую проверку.")
          ("rcv_buffer_size,r", bpo::value<size_t>(&rcv_buffer_size)->default_value(DEFAULT_RCV_BUFFER_SIZE),
                                "Указать размер принимаемого буфера.")
          ("hex_data_str,d", bpo::value<std::string>(&hex_data_str), "Указать отправляемые данные {HEX_STR,...}")
          ; //NOLINT
        bpo::variables_map vm;
        bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
        bpo::notify(vm);

        if (vm.count("help") or not vm.count("hex_data_str")) {
            std::cout << desc << "\n";
            return 0;
        }
        LOG(DEBUG) << "Start tty io.";
        /// Активировать порт.
        LOG(DEBUG) << tty << " : " << speed;
        utils::TtyIo tty_io(tty, B115200);

        Strhex2Hex s2h(hex_data_str);
        int writed = tty_io.write(s2h);
        if (writed not_eq ERROR_LEN) {
            LOG(DEBUG) << "Write: " << writed << ": " << std::string(s2h);
        } else {
            LOG(DEBUG) << tty_io.what();
        }

        LOG(DEBUG) << "Read";
        int rlen = 0;
        do {
            uint8_t rbuf[80] = {0};
            rlen = tty_io.read(rbuf, sizeof(rbuf));
            if (rlen not_eq ERROR_LEN) {
                uint8_t *p;
                std::stringstream ss;
                for (p = rbuf; rlen-- > 0; p++) {
                    LOG(DEBUG) << std::hex << static_cast<uint16_t>(*p);
                }
            } else {
                LOG(DEBUG) << tty_io.what();
            }
        } while (rlen);
    } catch (std::exception &e) {
        LOG(ERROR) << e.what() << "\n";
    }
    return 0;
}
