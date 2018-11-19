/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Тестовое приложение для отладки считывания RFID меток.
 * \author Величко Ростислав
 * \date   07.26.2017
 */
 
#include <iostream>
#include <memory>
#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <string>
#include <thread>
#include <cstdlib>
#include <chrono>

#include <unistd.h>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include "Log.hpp"
#include "Message.hpp"
#include "CommandsHandler.hpp"
#include "RfidController.hpp"
#include "Timer.hpp"
#include "SignalDispatcher.hpp"


namespace bpo = boost::program_options;
namespace chr = std::chrono;

typedef utils::SignalDispatcher SignalDispatcher;
typedef robocooler::rfid::Message::Buffer RfidBuffer;
typedef robocooler::rfid::Command::ECommandId RfidCid;
typedef robocooler::driver::RfidController RfidController;
typedef robocooler::driver::MapReadDatas MapReadDatas;
typedef robocooler::driver::ReadDataIter ReadDataIter;
typedef std::shared_ptr<RfidController> PRfidController;
typedef utils::Timer Timer;


#define DEFAULT_REREAD_TIMEOUT 5000
#define DEFAULT_READ_COUNT 7


int main(int argc, char **argv) {
    LOG_TO_STDOUT;
    try {
        std::string device; 
        size_t read_timeout;
        size_t read_count; 
        bool is_reset;
        bool is_warning;
        bool is_error;
        bool is_trace;
        bool is_debug;
        bool is_info;
        bpo::options_description desc("Тестовое приложение для отладки считывания RFID меток.\n" \
                                      "Пример: rfid-reader -t 5000 -i");
        desc.add_options()
          ("help,h", "Показать список параметров")
          ("device,d", bpo::value<std::string>(&device)->default_value("/dev/ttyUSB0"), "Последовательное устройство.")
          ("reset,r",  bpo::bool_switch(&is_reset)->default_value(false),  "Сбросить RFID модуль.")
          ("read_count,c", bpo::value<size_t>(&read_count)->default_value(DEFAULT_READ_COUNT), 
                           "Указать количество прочтений из буфера RFID за запрос.")
          ("read_timeout,t", bpo::value<size_t>(&read_timeout), "Указать промежуток времени между опросами.")
          ("is_warning,w", bpo::bool_switch(&is_warning)->default_value(false), "Выводить WARNING логи.")
          ("is_error,e", bpo::bool_switch(&is_error)->default_value(false), "Выводить ERROR логи.")
          ("is_trace,p,", bpo::bool_switch(&is_trace)->default_value(false), "Выводить TRACE логи.")
          ("is_debug,b", bpo::bool_switch(&is_debug)->default_value(false), "Выводить DEBUG логи.")
          ("is_info,i", bpo::bool_switch(&is_info)->default_value(false), "Выводить INFO логи.")
          ; //NOLINT
        bpo::variables_map vm;
        bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
        bpo::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }

        /// Установить уровни логирования.
        LOG_TOGGLE(DEBUG, is_debug);
        LOG_TOGGLE(WARNING, is_warning);
        LOG_TOGGLE(TRACE, is_trace);
        LOG_TOGGLE(ERROR, is_error);
        LOG_TOGGLE(INFO, is_info);

        /// Запустить обработчик.
        PRfidController rfidc = std::make_shared<RfidController>(nullptr, device,
                                                                 UPDATE_RECV_DATA_TIMEOUT,
                                                                 BUFFER_READING_NUM_ATTEMPT,
                                                                 READ_ANTENNS_COUNT);
        if (rfidc->isInited()) {
            /// Сбросить модуль, если требуется.
            if (is_reset) {
                rfidc->execute(static_cast<uint8_t>(RfidCid::cmd_reset));
            }
            /// Выполнить запуск инвенторизации.
            rfidc->startInventory();
            if (vm.count("read_timeout")) {
                LOG(INFO) << "Timer test: " << read_timeout << " mlsec";
                /// Запустить диспетчер сигналов прерывания работы SIGINT и SIGTERM.
                std::shared_ptr<Timer> timer = std::make_shared<Timer>(read_timeout, [&] {
                    LOG(INFO) << "Stop by timer";
                    rfidc->stopInventory();
                });
                timer->wait();
                LOG(INFO) << "Wait complete";
            } else {
                LOG(INFO) << "Long test";
                /// Запустить диспетчер сигналов прерывания работы SIGINT и SIGTERM.
                SignalDispatcher([&] {
                    rfidc->stopInventory();
                });
            }
        } else {
            LOG(ERROR) << "Can`t init RFID.";
        }
        rfidc.reset();
    } catch (std::exception &e) {
        LOG(ERROR) << e.what() << "\n";
    }
    return 0;
}
