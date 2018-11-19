/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Реализация приложения - драйвера устройств.
 * \author Величко Ростислав
 * \date   06.20.2017
 */
 
#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <functional>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <memory>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <curl/curl.h>

#include "Log.hpp"
#include "WsClientWorker.hpp"

// Для GDB: handle SIGILL nostop

#define DEFAULT_HTTP_URL "nasladdinapi.azurewebsites.net"
#define DEFAULT_PORT "443"
#define DEFAULT_PATH "plant"
#define DEFAULT_USB_DEVICE "/dev/ttyUSB0"

namespace bfs = boost::filesystem;
namespace bpo = boost::program_options;

typedef robocooler::driver::WsClientWorker WsClientWorker;
typedef std::shared_ptr<WsClientWorker> PWsClientWorker;


int main(int argc, char **argv) {
    LOG_TO_STDOUT;
    int ret = 0;
    try {
        std::string usb_device;
        std::string cooler_id;
        std::string url;
        std::string port;
        std::string path;
        bool is_device_off_mode;
        bool is_gpio_off_mode;
        size_t reread_timeout;
        size_t close_read_num;
        size_t attempt_read_num;
        bpo::options_description desc("Драйвер обслуживания устройств холодильника.");
        desc.add_options()
            ("help,h", "Показать список параметров")
            ("dev_off",  bpo::bool_switch(&is_device_off_mode), "Устанавливает режим работы с отключёнными устройствами.")
            ("gpio_off",  bpo::bool_switch(&is_gpio_off_mode)->default_value(false), "Устанавливает режим работы с отключённым GPIO.")
            ("usb_device,t", bpo::value<std::string>(&usb_device)->default_value(DEFAULT_USB_DEVICE), "Порт подключения RFID.")
            ("url,u", bpo::value<std::string>(&url)->default_value(DEFAULT_HTTP_URL),
            "Рест адрес инициализации подключения к серверу")
            ("port,p", bpo::value<std::string>(&port)->default_value(DEFAULT_PORT), "Указать порт сервера.")
            ("path,d", bpo::value<std::string>(&path)->default_value(DEFAULT_PATH), "Указать патч API сервера.")
            ("cooler_id,i", bpo::value<std::string>(&cooler_id)->default_value("1"), "Указать идентификатор холодильника.")
            ("reread_timeout,s", bpo::value<size_t>(&reread_timeout)->default_value(UPDATE_RECV_DATA_TIMEOUT),
                                 "Таймаут перезапуска опроса антенн [миллисекунты].")
            ("close_read_num,k", bpo::value<size_t>(&close_read_num)->default_value(BUFFER_READING_NUM_ATTEMPT),
                                 "Количество обходов антенн после закрытия дверей.")
            ("attempt_read_num,l", bpo::value<size_t>(&attempt_read_num)->default_value(READ_ANTENNS_COUNT),
                                   "Количество обходов антенна при открытых дверях.")
            ; //NOLINT
        bpo::variables_map vm;
        bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
        bpo::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }
        /// Доабавить порт, если указан.
        if (not port.empty()) {
            url += ":" + port;
        }
        LOG(TRACE) << "PlantHub_" << cooler_id << ": " << url;
        PWsClientWorker ws_worker = std::make_shared<WsClientWorker>(usb_device, cooler_id, url,
                                                                     reread_timeout, close_read_num, attempt_read_num,
                                                                     (not is_gpio_off_mode));
        ws_worker->startClient();
        ret = ws_worker->getReturnValue();
    } catch (std::exception &e) {
        LOG(ERROR) << e.what() << "\n";
    }
    return ret;
}
