/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Класс управляющей обслуживанием контроллера GPIO.
 * \author Величко Ростислав
 * \date   06.21.2017
 */

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <atomic>

#include "Bases.hpp"
#include "Timer.hpp"

#define DOOR_TIMEOUT 5000
#define SENSOR_CHECK_TIMEOUT 100

namespace robocooler {
namespace driver {

typedef utils::Timer Timer;
typedef std::shared_ptr<Timer> PTimer;
typedef std::shared_ptr<std::thread> PThread;

class GpioController 
    : public GpioControllerBase {
public:
    class Door {
        std::atomic<bool> _is_opened; ///< Флаг состояни открытой двери.
        int _gpio_pin; ///< GPIO пин открытия/закрытия двери.
        int _is_opened_gpio_pin; ///< GPIO пин датчика открытия двери.
        int _is_closed_gpio_pin; ///< GPIO пин датчика закрытия двери.

    public:
        Door(int gpio_pin_, int is_opened_gpio_pin_, int is_closed_gpio_pin_);
        virtual ~Door();

        /**
         * \brief Метод инициализирует порты GPIO в режим OUTPUT для управления актуатором и INPUT для датчиков.
         */ 
        void init();
        
        /**
         * \brief Метод открывает дверь в течении заданного таймаута, если таймаут 0 - то открывает до отсечки.
         * \param mlscs_ Асинхронная задержка в милисекундах до сброса состояния.
         */ 
        void open(int mlscs_ = 500);
        
        /**
         * \brief Метод закрывает дверь в течении заданного таймаута, если таймаут 0 - то открывает до отсечки.
         * \param mlscs_ Асинхронная задержка в милисекундах до сброса состояния.
         */ 
        void close(int mlscs_ = 500);
        
        /**
         * \brief Метод возвращает состояние двери.
         */ 
        bool isOpened();

        /**
         * \brief Метод возвращает номер пина концевика закрытой двери.
         */
        int getClosingPin();
    };
    

    /**
     * \brief Класс фиксирующий срабатывание датчика препятствий при закрывании двери.
     */
    class ObstacleSensor {
        int _sensor_gpio_pin; ///< Номер GPIO пина, с сигналом препятствия.
        int _open_mlscs; ///< Задержка после срабатывания датчика.
        int _deinit_mlscs; ///< Задержка после команды на закрытие двери.
        PTimer _open_timer; ///< Таймер активного состояния датчика.
        PTimer _deinit_timer; ///< Таймер завершения авктивного состояния.
        std::atomic<bool> _is_active; ///< Флаг активного состояния.
        std::shared_ptr<Door> _active_door;

    public:
        /**
         * \brief  Конструктор инициализирует временные промежутки.
         * \param  sensor_gpio_pin_  Номер пина закрытия двери.
         * \param  secure_mlscs_  Время, ожидания срабатывания датчика препятствия.
         * \param  open_mlscs_  Время после срабатывания датчика.
         */
        ObstacleSensor(int sensor_gpio_pin_ = 2, int open_mlscs_ = 3000, int _deinit_mlscs = 10000);
        ~ObstacleSensor();

        /**
         * \brief Метод инициализации пина GPIO.
         * \param  sensor_gpio_pin_  Номер пина с сигналом от датчика препятствия.
         */
        void init();

        /**
         * \brief Метод выполняет фиксацию срабатывания датчика препятствий и открывает дверь на заданное в конструкторе время.
         */
        void initSecuredDoor(std::shared_ptr<GpioController::Door> door_);

        /**
         * \brief Метод выполняет открытие двери при срабатывании датчика препятствия.
         */
        void onAlarm();

        /**
         * \brief Метод возвращает true, если объект обрабатывает закрытие двери, false - в обратном случае.
         */
        bool isActive();

        /**
         * \brief Метод возвращает пни сенсора.
         */
        int getAlarmPin();
    };

private:
    std::mutex _mutex;
    bool _is_inited;
    std::shared_ptr<ObstacleSensor> _sensor; ///< Объект контроля датчика препятствия.
    std::shared_ptr<Door> _last_opened_door;
    std::shared_ptr<Door> _left_door;  ///< Объект контроля работы с актуатором левой двери.
    std::shared_ptr<Door> _right_door; ///< Объект контроля работы с актуатором правой двери.
    WorkerBase *_worker;
    bool _is_gpio_on;
  
public:
    static GpioController *_gpio_ctrl;

    /**
     * \brief Конструктор контролера GPIO инициализирует номера пинов, для задействованных устройств.
     * \param worker_     Основной клас обслуживания устройств.
     * \param is_gpio_on_ Флаг вкл./выкл. обслуживания GPIO.
     */
    GpioController(WorkerBase *worker_, bool is_gpio_on_);
    virtual ~GpioController();
    
    /**
     * \brief Метод метод вызывается при срабатывании концевика двери.
     */
    void onDoorClosing();

    /**
     * \brief Метод метод вызывается при срабатывании концевика левой двери.
     */
    void onLeftDoorClosing();

    /**
     * \brief Метод метод вызывается при срабатывании концевика правой двери.
     */
    void onRightDoorClosing();

    /**
     * \brief Метод метод вызывается при срабатывании концевика правой двери.
     */
    void onAlarm();

    /**
     * \brief Метод состояния устройства GPIO.
     */ 
    bool isInited();

    /**
     * \brief Метод возвращает true - если одна из дверей открыта, false в обратном случае.
     */
    bool isOpened();

    /**
     * \brief Метод открывает левую дверь в течении переданного времени, если mlscs_ == 0 - то до отсечки.
     * \param mlscs_ Время выполнения процесс в милисекундах.
     */ 
    void openLeftDoor(size_t mlscs_ = 0);

    /**
     * \brief Метод закрывает левую дверь в течении переданного времени, если mlscs_ == 0 - то до отсечки.
     * \param mlscs_ Время выполнения процесс в милисекундах.
     */ 
    void closeLeftDoor(size_t mlscs_ = 0);

    /**
     * \brief Метод открывает правую дверь в течении переданного времени, если mlscs_ == 0 - то до отсечки.
     * \param mlscs_ Время выполнения процесс в милисекундах.
     */ 
    void openRightDoor(size_t mlscs_ = 0);

    /**
     * \brief Метод закрывает правую дверь в течении переданного времени, если mlscs_ == 0 - то до отсечки.
     * \param mlscs_ Время выполнения процесс в милисекундах.
     */ 
    void closeRightDoor(size_t mlscs_ = 0);
};
} /// namespace robocooler
} /// namespace driver

