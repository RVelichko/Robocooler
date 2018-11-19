/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Реализация обработчика клиентского подключения к серверу.
 * \author Величко Ростислав
 * \date   06.21.2017
 */

/**
 * http://52.178.205.225:8081/
 * PCmyfriend@gmail.com
 * Dark_312
 */ 

#pragma once

#include <mutex>
#include <string>
#include <stack>
#include <memory>

#include "Timer.hpp"
#include "SignalDispatcher.hpp"
#include "Bases.hpp"
#include "WsClient.hpp"


#define KEEPALIVE_TIMER 3000

namespace robocooler {
namespace driver {

class LogSender;
class JsonExtractor;
class CommandHandler;
class GpioController;
class RfidController;

typedef utils::Timer Timer;
typedef std::shared_ptr<Timer> PTimer;
typedef std::shared_ptr<LogSender> PLogSender;
typedef std::shared_ptr<JsonExtractor> PJsonExtractor;
typedef std::shared_ptr<GpioController> PGpioController;
typedef std::shared_ptr<RfidController> PRfidController;
typedef std::shared_ptr<CommandHandler> PCommandHandler;
typedef std::shared_ptr<WsClient> PWsClient;
typedef utils::SignalDispatcher SignalDispatcher;
typedef std::shared_ptr<SignalDispatcher> PSignalDispatcher;


class WsClientWorker 
    : public WorkerBase
    , public std::enable_shared_from_this<WsClientWorker> 
    , private boost::noncopyable {
    std::mutex _mutex; ///< Единый клентский объект синхронизации.

    uint32_t _attemp_connetion_count; ///< Счётчик попыток подключений.

    std::string _cooler_id;   ///< Идентификатор холодильника.
    std::string _addr;         ///< Адрес для постоянного подключения без параметров.
    int _port;                ///< Порт серверного подключения.
    int _return_value;        ///< Переменная равна 0, если приложение завершилось штатно.

    PTimer _keepalive_timer; ///< Таймер периодических опросов сервера.
    
    PJsonExtractor _json_extractor;       ///< Объект извлечения json из входного потока.
    PWsClient _client;                    ///< Объект websocket клиентского подключения.
    PLogSender _log_sender;               ///< Объект контролирующий отправку логов на сервер.
    PCommandHandler _command_handler;     ///< Обработчик серверных команд.
    PGpioController _gpio_controller;     ///< Обработчик команд управления дверями.
    PRfidController _rfid_controller;     ///< Обработчик команд управления RFID модулем.
    PSignalDispatcher _signal_dispatcher; ///< Обработчик системных сигналов.

    std::string _ws_request; ///< Строка с адресом подключения.

    bool _is_connect_error; ///< Флаг неудачного подключения.

    /**
     * \brief Метод инициализации подключения.
     */
    void init();
    
    /**
     * \brief Метод выполняет переподключение при ошибке соединения.
     */
    void reconnect();

    /**
     * \brief Метод выполняет отправку сообщения о завершении работы.
     */
    void stop();

    /**
     * \brief Метод принимает выделенный из потока json.
     */
    void onJson(const std::string &json_);
    
public:
    /**
     * \brief Конструктор обработчика инициализирует объект синхронизации в базовый класс.
     * \param usb_device_       Имя устройства usb для подключени RFID.
     * \param cooler_id_        Идентификатор холодильника.
     * \param addr_             Адрес подключения без параметров.
     * \param reread_timeout_   Таймаут перезапуска опроса антенн [миллисекунты].
     * \param close_read_num_   Количество обходов антенн после закрытия дверей.
     * \param attempt_read_num_ Количество обходов антенна при открытых дверях.
     * \param is_gpio_on_       Флаг режима обслуживания GPIO.
     */
    explicit WsClientWorker(const std::string &usb_device_,
                            const std::string &cooler_id_,
                            const std::string &addr_,
                            size_t reread_timeout_,
                            size_t close_read_num_,
                            size_t attempt_read_num_,
                            bool is_gpio_on_);
    virtual ~WsClientWorker();

    /**
     * \brief Метод возвращает переменную с результатом завершения работы приложения.
     */
    int getReturnValue();

    /**
     * \brief Метод отправляет строку JSIN по websocket.
     */
    virtual void send(const std::string &json_str_);

    /**
     * \brief Метод вызывается при успешном подключении по websocket.
     */
    virtual void onOpen(Client *client_, ConnectionHdl hdl_);

    /**
     * \brief Метод вызывается при приёме строки данных из установленного канала подключения, и выделяет json посылки.
     * \param msg_ Часть принятых данных.
     */
    virtual void onMessage(ConnectionHdl hdl_, PMessage msg_);

    /**
     * \brief Метод вызывается при ошибке подключения.
     * \param ec_ Информация об ошибке.
     */
    virtual void onError(Client *client_, ConnectionHdl hdl_);

    /**
     * \brief Метод вызывается при отключении со стороны сервера.
     * \param status_ Статус отключения от сети.
     * \param reason_ Причина отключения.
     */
    virtual void onClose(Client *client_, ConnectionHdl hdl_);

    /**
     * \brief Метод возвращает идентификатор холодильника.
     */
    virtual std::string getCoolerId();

    /**
     * \brief Метод возвращает указатель на контроллер дверей.
     */
    virtual GpioControllerBase* getGpioController();

    /**
     * \brief Метод возвращает указатель на контроллер RFID.
     */
    virtual RfidControllerBase* getRfidController();

    /**
     * \brief Метод возвращает указатель на контроллер клиентских команд.
     */
    virtual CommandHandlerBase* getCommandHandler();
    
    /**
     * \brief Метод запускает поток клинета и остаётся в нём до окончания работы.
     */
    void startClient();
};
} // driver
} // robocooler

