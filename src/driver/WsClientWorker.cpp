#include <exception>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <utility>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/optional/optional.hpp>

#include "Log.hpp"
#include "LogSender.hpp"
#include "JsonExtractor.hpp"
#include "CommandHandler.hpp"
#include "GpioController.hpp"
#include "RfidController.hpp"
#include "WsClientWorker.hpp"


#define RECONNECT_TIMEOUT 1000 ///< Задержка на переподключение в милисекундах.

namespace bpt = boost::property_tree;
namespace ph = std::placeholders;

using namespace robocooler;
using namespace driver;

typedef utils::Timer Timer;


void WsClientWorker::onOpen(Client *client_, ConnectionHdl hdl_) {
    LOG(DEBUG);
    /// Активировать обработчик команд.
    _command_handler = std::make_shared<CommandHandler>(this);

    /// Запустить keepalive.
    _keepalive_timer = std::make_shared<Timer>(KEEPALIVE_TIMER, [this] {
        _keepalive_timer->restart(); ///< перезапустить таймер до очередного опроса доступности сервера.
    });

    /// Отправить команду добавления в комнату.
    std::stringstream ss;
    ss << R"({"H":"PlantHub","A":{"group":"Plant_)" << _cooler_id << R"("},"M":"addToGroup"})";
    LOG(DEBUG) << ss.str();
    _client->send(ss.str());
}


void WsClientWorker::onMessage(ConnectionHdl hdl_, PMessage msg_) {
    std::string msg;
    if (msg_->get_opcode() == websocketpp::frame::opcode::text) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_json_extractor) {
            _json_extractor->onMessage(msg_->get_payload());
        }
    } else {
        LOG(WARNING) << "Msg is not string";
        //websocketpp::utility::to_hex(msg->get_payload());
    }
}


void WsClientWorker::onJson(const std::string &json_) {
    if (_command_handler) {
        _command_handler->handle(json_);
    }
}


void WsClientWorker::onError(Client *client_, ConnectionHdl hdl_) {
    LOG(DEBUG);
    /// Сбросить keepalive таймер.
    _keepalive_timer.reset();
    /// Остановить работу приложения.
    PConnection con = client_->get_con_from_hdl(hdl_);
    std::string server = con->get_response_header("Server");
    std::string error_reason = con->get_ec().message();
    LOG(ERROR) << "SERVER \"" << server << "\": " << error_reason;
    _is_connect_error = true;
    if (_json_extractor) {
        _json_extractor->clear();
    }
    /// Остановить обслуживающие модули.
    _gpio_controller.reset();
    _rfid_controller.reset();
    /// Остановить остановить диспетчер системных сигналов.
    _signal_dispatcher.reset();
    _return_value = 3;
    /// Остановить клмент.
    stop();
}


void WsClientWorker::onClose(Client *client_, ConnectionHdl hdl_) {
    LOG(DEBUG);
    /// Сбросить keepalive таймер.
    _keepalive_timer.reset();
    /// Остановить работу приложения.
    PConnection con = client_->get_con_from_hdl(hdl_);
    std::string server = con->get_response_header("Server");
    std::string error_reason = con->get_ec().message();
    LOG(INFO) << "SERVER \"" << server << "\": Is closed connection.";
    _is_connect_error = true;
    if (_json_extractor) {
        _json_extractor->clear();
    }
    /// Остановить обслуживающие модули.
    _gpio_controller.reset();
    _rfid_controller.reset();
    /// Остановить остановить диспетчер системных сигналов.
    _signal_dispatcher.reset();
    /// Остановить клмент.
    stop();
}


void WsClientWorker::reconnect() {
}


void WsClientWorker::init() {
    LOG(DEBUG);
    /// Инициализировать объект выделения json.
    _json_extractor = std::make_shared<JsonExtractor>(std::bind(&WsClientWorker::onJson, this, ph::_1));
               
    /// Подключить websocket.
    _ws_request = "wss://" + _addr + "/plant";
    LOG(DEBUG) << "\n------------------------------------------------"
               << "\n" << _ws_request
               << "\n------------------------------------------------";
    _client = std::make_shared<WsClient>(this);
}


void WsClientWorker::stop() {
    LOG(DEBUG);
    if (_client) {
        _client.reset();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


WsClientWorker::WsClientWorker(const std::string &usb_device_,
                               const std::string &cooler_id_,
                               const std::string &addr_,
                               size_t reread_timeout_,
                               size_t close_read_num_,
                               size_t attempt_read_num_,
                               bool is_gpio_on_)
    : _attemp_connetion_count(0)
    , _cooler_id(cooler_id_)
    , _addr(addr_)
    , _return_value(0)
    , _is_connect_error(false) {
    LOG(DEBUG);
    _gpio_controller = std::make_shared<GpioController>(this, is_gpio_on_);
    _rfid_controller = std::make_shared<RfidController>(this, usb_device_, reread_timeout_, close_read_num_, attempt_read_num_);
}


WsClientWorker::~WsClientWorker() {
    LOG(DEBUG);
}


void WsClientWorker::send(const std::string &json_str_) {
    LOG(DEBUG);
    _client->send(json_str_);
}


int WsClientWorker::getReturnValue() {
    return _return_value;
}


std::string WsClientWorker::getCoolerId() {
    return _cooler_id;
}


GpioControllerBase* WsClientWorker::getGpioController() {
    return _gpio_controller.get();
}


RfidControllerBase* WsClientWorker::getRfidController() {
    return _rfid_controller.get();
}


CommandHandlerBase* WsClientWorker::getCommandHandler() {
    return _command_handler.get();
}


void WsClientWorker::startClient() {
    LOG(DEBUG);
    if (_rfid_controller and _rfid_controller->isInited() and 
        _gpio_controller and _gpio_controller->isInited()) {
        /// Старт клинетского обработчика.
        init();
        if (not _client->connect(_ws_request)) {
            LOG(FATAL) << "Can`t connect to server.";
            _return_value = 1;
        } else {;
            /// Запустить диспетчер сигналов прерывания работы SIGINT и SIGTERM. 
            LOG(DEBUG) << "Start dispatcher.";
            _signal_dispatcher = std::make_shared<SignalDispatcher>(std::bind(&WsClientWorker::stop, this));
        }
    } else {
        LOG(FATAL) << "Can`t connect to RFID or GPIO.";
        _return_value = 2; 
    }
    LOG(DEBUG) << "Stop ws client worker.";
}
