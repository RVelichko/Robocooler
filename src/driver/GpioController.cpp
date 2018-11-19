#ifdef USE_WIRINGPI
#include <wiringPi.h>
#endif
 
#include "Log.hpp"
#include "CommandHandler.hpp"
#include "GpioController.hpp"

namespace chr = std::chrono;

using namespace robocooler;
using namespace driver;


GpioController* GpioController::_gpio_ctrl = nullptr;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void AlarmInterrupt() {
    LOG(DEBUG);
    if (GpioController::_gpio_ctrl) {
        GpioController::_gpio_ctrl->onAlarm();
    }
}


void NullFunc() {
    LOG(DEBUG);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


GpioController::Door::Door(int gpio_pin_, int is_opened_gpio_pin_, int is_closed_gpio_pin_)
    : _is_opened(false)
    , _gpio_pin(gpio_pin_)
    , _is_opened_gpio_pin(is_opened_gpio_pin_)
    , _is_closed_gpio_pin(is_closed_gpio_pin_) {
    LOG(DEBUG) << "GPIO: " << gpio_pin_ << ", " << is_closed_gpio_pin_;
}


GpioController::Door::~Door() {
    LOG(DEBUG);
}


void GpioController::Door::init() {
    LOG(DEBUG);
    /// Инициализация шины для управления дверями.
    #ifdef USE_WIRINGPI
    pinMode(_gpio_pin, OUTPUT);
    digitalWrite(_gpio_pin, HIGH);
    #endif
    LOG(DEBUG) << "init open/close pin " << _gpio_pin << " is closed pin: " << _is_closed_gpio_pin;
    /// Инициализация шины для получения сигнала состояний дверей.
    #ifdef USE_WIRINGPI
    pinMode(_is_opened_gpio_pin, INPUT);
    pinMode(_is_closed_gpio_pin, INPUT);
    #endif
}


void GpioController::Door::open(int mlscs_) {
    LOG(DEBUG) << "GPIO: " << _gpio_pin;
    if (not _is_opened) {
        #ifdef USE_WIRINGPI
        digitalWrite(_gpio_pin, LOW);
        #endif
        _is_opened = true;
    }
}


void GpioController::Door::close(int mlscs_) {
    LOG(DEBUG) << "GPIO: " << _gpio_pin;
    if (_is_opened) {
        #ifdef USE_WIRINGPI
        digitalWrite(_gpio_pin, HIGH);
        #endif
        _is_opened = false;
    }
}


bool GpioController::Door::isOpened() {
    LOG(DEBUG) << _gpio_pin << " - " << (_is_opened ? "opened" : "closed");
    return _is_opened;
}


int GpioController::Door::getClosingPin() {
    return _is_closed_gpio_pin;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


GpioController::ObstacleSensor::ObstacleSensor(int sensor_gpio_pin_, int open_mlscs_, int deinit_mlscs_)
    : _sensor_gpio_pin(sensor_gpio_pin_)
    , _open_mlscs(open_mlscs_)
    , _deinit_mlscs(deinit_mlscs_)
    , _is_active(false) {
    LOG(DEBUG) << "open timeput: "<< open_mlscs_;
}


GpioController::ObstacleSensor::~ObstacleSensor() {
    LOG(DEBUG);
}


void GpioController::ObstacleSensor::init() {
    LOG(DEBUG) << "init sensor pin: " << _sensor_gpio_pin;
    #ifdef USE_WIRINGPI
    pinMode(_sensor_gpio_pin, INPUT);
    #endif
}


void GpioController::ObstacleSensor::initSecuredDoor(std::shared_ptr<GpioController::Door> door_) {
    if (door_ and not _active_door) {
        LOG(DEBUG) << "Init ALARM";
        _active_door = door_;
        _deinit_timer = std::make_shared<Timer>(_deinit_mlscs, [this]{
            _active_door = std::shared_ptr<GpioController::Door>();
            _is_active = false;
        });
    }
    if (not door_ and _active_door) {
        LOG(DEBUG) << "Deinit ALARM";
        /// Сбросить таймер полного закрытия.
        _deinit_timer.reset();
        _active_door = door_;
    }
}


void GpioController::ObstacleSensor::onAlarm() {
    /// Запуск режима ожидания сигнала от датчика препятствия.
    if (_active_door) {
        LOG(DEBUG) << "ALARM...";
        /// Сбросить таймер полного закрытия.
        _deinit_timer.reset();
        /// Открыть дверь.
        _active_door->open();
        _is_active = false;
        _open_timer = std::make_shared<Timer>(_open_mlscs, [this]{
            LOG(DEBUG) << "Close after ALARM...";
            _active_door->close();
            _deinit_timer = std::make_shared<Timer>(_deinit_mlscs, [this]{
                _active_door = std::shared_ptr<GpioController::Door>();
                _is_active = false;
            });
        });
    }
}


bool GpioController::ObstacleSensor::isActive() {
    return _is_active;
}


int GpioController::ObstacleSensor::getAlarmPin() {
    return _sensor_gpio_pin;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


GpioController::GpioController(WorkerBase *worker_, bool is_gpio_on_)
    : _is_inited(false) 
    , _sensor(std::make_shared<ObstacleSensor>(4))
    , _left_door(std::make_shared<Door>(12, 17, 5))
    , _right_door(std::make_shared<Door>(16, 27, 6))
    , _worker(worker_)
    , _is_gpio_on(is_gpio_on_) {
    LOG(DEBUG);
    /// Инициализация шины GPIO Broadcom пинами.
    #ifdef USE_WIRINGPI
    LOG(TRACE) << "try init device...";
    _is_inited = (wiringPiSetupGpio() not_eq -1); 
    #endif
    if (_is_inited) {
        _left_door->init();
        _right_door->init();
    } else {
        LOG(ERROR) << "Can`t init GPIO device.";
    }
    _gpio_ctrl = this;
    #ifdef USE_WIRINGPI
    /// Проинициализировать прерывания.
    /// INT_EDGE_FALLING (прерывание при смене уровня на пине с высокого на низкий)
    /// INT_EDGE_RISING (прерывание при смене уровня на пине с низкого на высокий)
    /// INT_EDGE_BOTH (прерывание при любой смене уровня на пине)
    wiringPiISR(_sensor->getAlarmPin(), INT_EDGE_RISING, AlarmInterrupt);
    #endif
}


GpioController::~GpioController() {
    LOG(DEBUG);
    _gpio_ctrl = nullptr;
    #ifdef USE_WIRINGPI
    wiringPiISR(_sensor->getAlarmPin(), INT_EDGE_RISING, NullFunc);
    #endif
    _left_door.reset();
    _right_door.reset();
    _sensor.reset();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GpioController::onDoorClosing() {
    std::unique_lock<std::mutex> lock(_mutex);
    if (not _sensor->isActive()) {
        _sensor->initSecuredDoor(std::shared_ptr<Door>());
    }
}


void GpioController::onLeftDoorClosing() {
    std::unique_lock<std::mutex> lock(_mutex);
    if (not _sensor->isActive()) {
        _sensor->initSecuredDoor(std::shared_ptr<Door>());
    }
}


void GpioController::onRightDoorClosing() {
    std::unique_lock<std::mutex> lock(_mutex);
    if (not _sensor->isActive()) {
        _sensor->initSecuredDoor(std::shared_ptr<Door>());
    }
}


void GpioController::onAlarm() {
    /// Сбросить сбросить таймер завершения инвентаризации.
    if (_worker) {
        CommandHandler *cmdh = dynamic_cast<CommandHandler*>(_worker->getCommandHandler());
        if (cmdh) {
            cmdh->restartStopInventoryTimeout();
        }
    }
    std::unique_lock<std::mutex> lock(_mutex);
    _sensor->onAlarm();
}


bool GpioController::isInited() {
    #ifdef USE_WIRINGPI
    return _is_inited;
    #else
    return true;
    #endif
}


bool GpioController::isOpened() {
    return ((_left_door and _left_door->isOpened()) or
            (_right_door and _right_door->isOpened()) or
            (_sensor and _sensor->isActive()));
}


void GpioController::openLeftDoor(size_t mlscs_) {
    LOG(DEBUG);
    if (_is_gpio_on) {
        if (_right_door->isOpened()) {
            _right_door->close();
        }
        _left_door->open(mlscs_);
    }
}


void GpioController::closeLeftDoor(size_t mlscs_) {
    LOG(DEBUG);
    if (_is_gpio_on) {
        _left_door->close(mlscs_);
        _sensor->initSecuredDoor(_left_door);
    }
}


void GpioController::openRightDoor(size_t mlscs_) {
    LOG(DEBUG);
    if (_is_gpio_on) {
        if (_left_door->isOpened()) {
            _left_door->close();
        }
        _right_door->open(mlscs_);
    }
}


void GpioController::closeRightDoor(size_t mlscs_) {
    LOG(DEBUG);
    if (_is_gpio_on) {
        _right_door->close(mlscs_);
        _sensor->initSecuredDoor(_right_door);
    }
}
