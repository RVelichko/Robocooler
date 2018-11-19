/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Реализация обработчика команд.
 * \author Величко Ростислав
 * \date   06.27.2017
 */

#pragma once

#include <string>
#include <memory>

#include <boost/property_tree/ptree.hpp>

#include "Bases.hpp"
#include "Timer.hpp"


#define INVENTORY_TIMEOUT 5000
#define CLOSED_TIMEOUT 3000


namespace bpt = boost::property_tree;

namespace robocooler {
namespace driver {

typedef utils::Timer Timer;
typedef std::shared_ptr<Timer> PTimer;


class CommandHandler :
    public CommandHandlerBase {
    WorkerBase *_worker;
    PTimer _stop_inventory_timer;
    PTimer _close_inventory_timer;

    /**
     * \brief Метод обработки "H" == "PlantHub" команд.
     * \param pt_ Содержит готовый к обработке JSON.
     */
    void handlePlantHub(const bpt::ptree &pt_);

    /**
     * \brief Метод обработки "A" команд администратора.
     * \param pt_ Готовый к обработке json.
     */
    void handleA(const bpt::ptree &pt_);

    /**
     * \brief Метод передаёт настройки в RFID модуль.
     * \param pt_ Готовый к обработке json.
     */
    void setRfidConfig(const bpt::ptree &pt_);

    /**
     * \brief Метод Преобразует массив меток в строку метод для вставки в JSON.
     * \param labels_ Массив меток.
     */
    std::string getLabelsString(const std::vector<std::string> &labels_);
            
    /**
     * \brief Метод передаёт настройки процесса опроса антенн в RFID модуль.
     * \param pt_ json формата "A":{"requestsNumber":4,"delay":1234}.
     */
    void setRequestsSettings(const bpt::ptree &pt_);

    /**
     * \brief Метод запускает процесс выявления плохо определяемых меток.
     * \param pt_ json формата "A":{"iterationsNumber":0}.
     */
    void findBrokenLabels(const bpt::ptree &pt_);

public:
    /**
     * \brief Конструктор обработчика команд инициализирует клиентский воркер.
     * \param worker_ Указатель на обработчик клиентских запросов.
     */
    explicit CommandHandler(WorkerBase *worker_);
    virtual ~CommandHandler();
    
    /**
     * \brief Метод отправляет кипелайв в ответ на сообщения и кипелайвы от сервера.
     * \param json_ Принятый json на обработку.
     */
    void handle(const std::string &json_);
    
    /**
     * \brief Метод отправляет добавленные и изъятые продукты.
     * \param out_ Массив идентификаторов изъятых продуктов.
     * \param in_ Массив идентификаторов добавленных продуктов.
     */
    void sendProducts(const std::vector<std::string> &out_, const std::vector<std::string> &in_);

    /**
     * \brief Метод отправляет текущие видимые метки.
     * \param cur_ Массив идентификаторов продуктов.
     */
    void sendCurProducts(const std::vector<std::string> &cur_);

    /**
     * \brief Метод перезапускает таймер завершения инвентаризации.
     */
    void restartStopInventoryTimeout();
};
} // driver
} // robocooler

