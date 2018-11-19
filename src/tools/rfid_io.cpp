/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Тестовое приложение для отладки команд RFID.
 * \author Величко Ростислав
 * \date   07.12.2017
 */
 
#include <iostream>
#include <memory>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <string>
#include <cstdlib>

#include <unistd.h>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include "Log.hpp"
#include "Message.hpp"
#include "CommandsHandler.hpp"
#include "RfidController.hpp"
#include "SignalDispatcher.hpp"


namespace bpo = boost::program_options;
namespace chr = std::chrono;

typedef utils::SignalDispatcher SignalDispatcher;
typedef robocooler::rfid::Message::Buffer RfidBuffer;
typedef robocooler::rfid::Command::ECommandId RfidCid;
typedef robocooler::driver::RfidController RfidController;
typedef std::shared_ptr<RfidController> PRfidController;
typedef utils::Timer Timer;


class ValuesToHexes {
    RfidCid _cid;
    RfidBuffer _data;
    
public:
    explicit ValuesToHexes(const std::string &command_id_, const std::string &command_data_) {
        /// Преобразовать параметры в HEX.
        if (not command_data_.empty()) {
            /// Удалить пробелы.
            std::string command_data = command_data_;
            command_data.erase(std::remove(command_data.begin(), command_data.end(), ' '), command_data.end());
            /// Разбить строку на подстроки по ",".
            std::vector<std::string> strs;
            boost::split(strs, command_data, boost::is_any_of(","));
            for (std::string &str : strs) {
                LOG(DEBUG) << "d: " << str <<" to uint8_t.";
                _data.push_back(static_cast<uint8_t>(std::stoi(str, nullptr, 16) & 0xff));
            }
        }
        /// Преобразовать команду в HEX.
        LOG(DEBUG) << "cmd: " << command_id_ <<" to uint8_t.";
        _cid = static_cast<RfidCid>(std::stoi(command_id_, nullptr, 16) & 0xff);
    }
    
    operator uint8_t () {
        return static_cast<uint8_t>(_cid);
    }

    operator RfidBuffer () {
        return _data;
    }
};


std::string cmd_codes = "Reader control commands:\n" \
                        "cmd_reset                = 0x70\n" \
                        "cmd_set_uart_baudrate    = 0x71\n" \
                        "cmd_get_firmware_version = 0x72\n" \
                        "cmd_set_reader_address   = 0x73\n" \
                        "cmd_set_work_antenna     = 0x74\n" \
                        "cmd_get_work_antenna     = 0x75\n" \
                        "cmd_set_output_power     = 0x76\n" \
                        "cmd_get_output_power     = 0x77\n" \
                        "cmd_set_frequency_region = 0x78\n" \
                        "cmd_get_frequency_region = 0x79\n" \
                        "18000-6C Commands:\n" \
                        "cmd_inventory = 0x80\n" \
                        "cmd_read      = 0x81\n" \
                        "Buffer control commands:\n" \
                        "cmd_get_inventory_buffer           = 0x90\n" \
                        "cmd_get_and_reset_inventory_buffer = 0x91\n" \
                        "cmd_get_inventory_buffer_tag_count = 0x92\n" \
                        "cmd_reset_inventory_buffer         = 0x93\n";

// ./rfid-io -c 70
// ./rfid-io -c 74 -d 0 && ./rfid-io -c 74 -d 1 && ./rfid-io -c 74 -d 2 && ./rfid-io -c 74 -d 3
// ./rfid-io -c 80
 

int main(int argc, char **argv) {
    LOG_TO_STDOUT;
    try {
        std::string device; 
        std::string command_id; 
        std::string command_data; 
        bpo::options_description desc("Тестовое приложение для отправки команд RFID.\n" \
                                      "Пример запуска: \"./rfid-io -c 74 -d 1\"");
        desc.add_options()
          ("help,h", "Показать список параметров")
          ("device,t", bpo::value<std::string>(&device)->default_value("/dev/ttyUSB0"), "Последовательное устройство")
          ("cmd_id,c", bpo::value<std::string>(&command_id),
                       ("Указать идентификатор отправляемой команды в HEX\n" + cmd_codes).c_str())
          ("cmd_data,d", bpo::value<std::string>(&command_data), "Указать данные отправляемой команды в {HEX,...}")
          ; //NOLINT
        bpo::variables_map vm;
        bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
        bpo::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }
        if (vm.count("cmd_id")) {
            /// Запустить обработчик.
            PRfidController rfidc = std::make_shared<RfidController>(nullptr, device,
                                                                     UPDATE_RECV_DATA_TIMEOUT,
                                                                     BUFFER_READING_NUM_ATTEMPT,
                                                                     READ_ANTENNS_COUNT);
            if (rfidc->isInited()) {
                ValuesToHexes vth(command_id, command_data);
                /// Выполнить команду.
                if (rfidc->execute(vth, vth)) {
                    std::this_thread::sleep_for(chr::milliseconds(1000));
                } else {
                    std::cout << desc << "\n";
                }
            } else {
                LOG(ERROR) << "Can`t init RFID.";
            }
            rfidc.reset();
        } else {
            std::cout << desc << "\n";
        }
    } catch (std::exception &e) {
        LOG(ERROR) << e.what() << "\n";
    }
    return 0;
}
