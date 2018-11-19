/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Базовый класс обработки клиентского подключения.
 * \author Величко Ростислав
 * \date   06.21.2017
 */

#pragma once

#include <string>


#define UNLOCK_TIMEOUT 10000          ///< Предельное время ожидания завершения ответа на команду.
#define ALL_TAGS_RECV_TIMEOUT 30000  ///< Таймаут ожидания всех меток после запроса содержимого.
#define RECV_DATA_TIMEOUT 700        ///< Предельное время ожидания очередной метки.
#define READ_ANTENNS_COUNT 1         ///< Количество чтений антенн до сравнения буферов.
#define UPDATE_RECV_DATA_TIMEOUT 1   ///< Таймаут после выполнения очередного опроса буфера.
#define BUFFER_READING_NUM_ATTEMPT 3
#define PERIODIC_INVENTORY_TIMEOUT 10000


namespace robocooler {
namespace driver {


class CommandHandlerBase {
public:
    CommandHandlerBase()
    {}

    virtual ~CommandHandlerBase()
    {}

    /**
     * \brief Абстрактный метод отправляет кипелайв в ответ на сообщения и кипелайвы от сервера.
     * \param json_ Принятый json на обработку.
     */
    virtual void handle(const std::string &json_) = 0;

    /**
     * \brief Абстрактный метод отправляет добавленные и изъятые продукты.
     * \param out_ Массив идентификаторов изъятых продуктов.
     * \param in_ Массив идентификаторов добавленных продуктов.
     */
    virtual void sendProducts(const std::vector<std::string> &out_, const std::vector<std::string> &in_) = 0;

    /**
     * \brief Абстрактный метод отправляет текущие видимые метки.
     * \param cur_ Массив идентификаторов продуктов.
     */
    virtual void sendCurProducts(const std::vector<std::string> &cur_) = 0;
};


class GpioControllerBase {
public:
    GpioControllerBase() 
    {}
    
    virtual ~GpioControllerBase() 
    {}
        
    /**
     * \brief Абстрактный метод открывает левую дверь.
     */
    virtual void openLeftDoor(size_t mlscs_ = 0) = 0;

    /**
     * \brief Абстрактный метод закрывает левую дверь.
     */
    virtual void closeLeftDoor(size_t mlscs_ = 0) = 0;

    /**
     * \brief Абстрактный метод открывает правую дверь.
     */
    virtual void openRightDoor(size_t mlscs_ = 0) = 0;

    /**
     * \brief Абстрактный метод закрывает правую дверь.
     */
    virtual void closeRightDoor(size_t mlscs_ = 0) = 0;

    /**
     * \brief Абстрактный метод закрывает правую дверь.
     */
    virtual bool isOpened() = 0;
};


class RfidControllerBase {
public:
    RfidControllerBase() 
    {}
    
    virtual ~RfidControllerBase() 
    {}
        
    /**
     * \brief Абстрактный метод запускает инвенторизацию.
     * \param need_result_ Флаг обязательной отправки на сарвер результата сравнения буферов.
     */
    virtual void startInventory(bool need_result_ = false) = 0;

    /**
     * \brief Абстрактный метод останавливает инвенторизацию.
     */
    virtual void stopInventory() = 0;

    /**
     * \brief Метод выполняет последовательный опрос RFID антенн и уточняет список меток.
     * \param count_  Количество опросов антенн.
     * \param with_counter_  Отправлять метки вместе с счётчиками меток.
     */
    virtual void inventory(size_t count_, bool with_counter_ = false) = 0;

    /**
     * \brief Метод выполняет команду по идентификатору и буферу параметров.
     * \param cmd_id_ Идентификатор команды.
     * \param data_buf_ Параметры команды.
     */
    virtual bool execute(uint8_t cmd_id_, const std::vector<uint8_t> &data_buf_ = std::vector<uint8_t>()) = 0;

    /**
     * \brief Метод возвращающий текущие настройки антенн.
     */
    virtual std::string getAntSettings() = 0;

    /**
     * \brief Метод отправляет текущий буфер метов.
     */
    virtual void accumulateBuffer() = 0;

    /**
     * \brief Метод устанавливает количество циклов опросаов антенн.
     * \param read_antenns_count_ Количество циклов чтений антенн.
     */
    virtual void setReadAntennsCount(size_t read_antenns_count_) = 0;

    /**
     * \brief Метод устанавливает количество опросов для итогового результата.
     * \param read_antenns_count_ Количество циклов чтений антенн.
     */
    virtual void setBufReadNumAttempt(size_t buf_read_num_attempt_) = 0;

    /**
     * \brief Метод возвращает текущее количество опросов для итогового результата.
     * \return Количество циклов чтений антенн.
     */
    virtual size_t getBufReadNumAttempt() = 0;

    /**
     * \brief Метод выполняет команду по идентификатору и буферу параметров.
     * \param cmd_id_ Задержка между иттерациями чтения антенн.
     */
    virtual void setReadAntennsTimeout(size_t timeout_) = 0;

    /**
     * \brief Метод запускает процесс выявления плохо читаемых меток.
     * \param iterations_num_ Количество иттераций при тестировании.
     */
    virtual void findBrokenLabels(size_t iterations_num_) = 0;
};

    
class WorkerBase {
public:
    WorkerBase() 
    {}
    
    virtual ~WorkerBase() 
    {}

    /**
     * \brief Метод отправляет строку JSIN по websocket.
     */
    virtual void send(const std::string &json_str_) = 0;

    /**
     * \brief Абстрактный метод, возвращающий идентификатор холодильника.
     */
    virtual std::string getCoolerId() = 0;

    /**
     * \brief Абстрактный метод, возвращающий обработчик команд сервера.
     */
    virtual CommandHandlerBase* getCommandHandler() = 0;

    /**
     * \brief Абстрактный метод, возвращающий контроллер дверей.
     */
    virtual GpioControllerBase* getGpioController() = 0;

    /**
     * \brief Абстрактный метод, возвращающий контроллер RFID.
     */
    virtual RfidControllerBase* getRfidController() = 0;
};
} /// driver
} /// robocooler
