/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Класс управляющей обслуживанием модуля RFID.
 * \author Величко Ростислав
 * \date   07.12.2017
 */

#pragma once

#include <memory>
#include <vector>
#include <atomic>
#include <string>
#include <utility>
#include <thread>
#include <map>
#include <condition_variable>

#include "Bases.hpp"
#include "Timer.hpp"
#include "CommandsHandler.hpp"
#include "TtyIo.hpp"

namespace robocooler {
namespace driver {

typedef std::thread Thread;
typedef std::atomic_bool AtomicBool;
typedef std::shared_ptr<Thread> PThread;
typedef std::shared_ptr<utils::TtyIo> PTtyIo;
typedef robocooler::rfid::Command RfidCmd;
typedef robocooler::rfid::Command::ECommandId RfidCid;
typedef robocooler::rfid::Message::Buffer RfidBuffer;
typedef robocooler::rfid::CommandsHandler RfidCommandsHandler;
typedef robocooler::rfid::Command::AntSettings RfidCas;
typedef std::shared_ptr<RfidCommandsHandler> PRfidCommandsHandler;
typedef std::shared_ptr<std::condition_variable> PCondition;
typedef std::map<RfidCid, PCondition> MapWaitCmdResults;
typedef MapWaitCmdResults::iterator WaitCmdResultIter;
typedef RfidCmd::EWorkAntenna RfidWorkAntenna; 
typedef std::map<std::string, RfidCmd::ReadCmdData> MapReadDatas;
typedef MapReadDatas::iterator ReadDataIter;
typedef utils::Timer Timer;
typedef std::shared_ptr<Timer> PTimer;


class RfidController 
    : public RfidControllerBase {
    std::mutex _mutex;                           ///< Объект синхронизации потока обслуживания последовательного порта.
    WorkerBase *_worker;                         ///< Объект контроллер клиентского подключения к серверу.
    bool _is_inited;                             ///< Флаг true - если последовательный порт к RFID модулю инициализирован.
    AtomicBool _is_runing;                       ///< Флаг true - если запущен процесс обслуживания порта RFID модуля.
    MapWaitCmdResults _cmd_conditions;           ///< Дерево объектов синхронизации для ожидания результата запроса.
    PThread _thread;                             ///< Поток обслуживания последовательного порта.
    PThread _inv_thread;                         ///< Поток обслуживания процесса инвенторизации.
    PThread _acm_inv_thread;                     ///< Поток обслуживания процесса получения текущего содержимого.
    AtomicBool _is_inventory;                    ///< Атомарный флаг процесса инвенторизации.
    AtomicBool _is_inventory_run;                ///< Атомарный флаг процесса инвенторизации.
    PRfidCommandsHandler _rfid_handler;          ///< Обработчик RFID протокола.
    size_t _read_count;                          ///< Количество опросов антенн при старт-стопной инвентаризации.
    RfidCas _ant_sets;                           ///< Текущие настройки антенн.
    PTtyIo _tty_io;                              ///< Последовательный порт.
    PTimer _periodic_inventory_timeout;          ///< Таймер процесса закрытой инвенторизации.
    PTimer _notify_timer;                        ///< Таймер аварийного сброса ожидающих мютексов.

    MapReadDatas _read_data;       ///< Буфер полученных меток при инициализации или при предыдущем чтении.
    MapReadDatas _cur_read_data;   ///< Буфер меток, считываемых при текущем запросе.
    MapReadDatas _buffered_data;   ///< Буфер меток, ожидаемых из rfid после команды запроса меток.
    MapReadDatas _accumulate_data; ///< Буфер меток, накапливаемых в процессе инвенторизации.
    MapReadDatas _prob_read_data;  ///< Буфер когда либо считанных меток для вычисления вероятности появления.

    size_t _reread_timeout; ///< Таймаут перезапуска опроса антенн [миллисекунты].
    size_t _close_read_num; ///< Количество обходов антенн после закрытия дверей.
    size_t _attempt_read_num; ///< Количество обходов антенна при открытых дверях.

    /**
     * \brief Метод уведомляет о завершении ожидания потока в условной переменной синхронизации.
     * \param cmd_id_ Идентификатор команды, связанной с условной переменной синхронизации.
     */ 
    void notifyOneCmdResult(RfidCid cmd_id_);

    /**
     * \brief Метод переводит условную переменной синхронизации в состояние ожидания.
     * \param cmd_id_ Идентификатор команды, связанной с условной переменной синхронизации.
     */ 
    void waitCmdResult(RfidCid cmd_id_);

    /**
     * \brief Метод переводит условную переменной синхронизации в состояние ожидания, для внешнего мютекса.
     * \param cmd_id_ Идентификатор команды, связанной с условной переменной синхронизации.
     * \param lock_  Внешний lock.
     * \param unlock_timout_  Таймаут, после которого произойдёт принудительный сброс таймера. 
     * \return false, если сработал таймер принудительного завершения ожидания; true, если таймер не сработал.
     */
    bool extLockWaitCmdResult(RfidCid cmd_id_, std::unique_lock<std::mutex>& lock_, size_t unlock_timout_);

    /**
     * \brief Метод обслуживания подсистемы объмена с RFID монтроллером.
     */ 
    void runSerial();
    
    /**
     * \brief Метод сравнивает полученный буфер меток с сохранённымфиксирует разницу и обновляет буфер хранения.
     * \param need_result_ Флаг обязательной отправки на сарвер результата сравнения буферов.
     */ 
    void compareBuffers(bool need_result_ = false);

    /**
     * \brief Метод отдаёт текущий накопленный буфер меток на сервер.
     */ 
    void currentBuffer();

    /**
     * \brief Метод отдаёт текущий буфер меток на сервер.
     */
    void verifyBuffer();

    /**
     * \brief Метод выполняет чтение буфера меток и сброс.
     */
    void readFromBufferAndReset();
    
    /**
     * \brief Метод выполняет последовательную активацию антенн, инвенторизацию и чтение меток из буфера.
     */
    void bufferReadProcess();

    /**
     * \brief Метод выполняет фиксацию принятой метки.
     * \param read_data_ Структура с данными метки.
     */
    void onReadData(const RfidCmd::ReadCmdData &read_data_);

    /**
     * \brief Метод запускает периодический опрос внутреннего содержимого.
     */
    void startPeriodicInventory();

public:
    /**
     * \brief Конструктор контролера RFID инициализирует USB объмен с устройством.
     * \param worker_           Объект контроллер клиентского подуключения к серверу.
     * \param device_           Имя последовательного устройства.
     * \param reread_timeout_   Таймаут перезапуска опроса антенн [миллисекунты].
     * \param close_read_num_   Количество обходов антенн после закрытия дверей.
     * \param attempt_read_num_ Количество обходов антенна при открытых дверях.
     */
    RfidController(WorkerBase *worker_,
                   const std::string &device_,
                   size_t reread_timeout_,
                   size_t close_read_num_,
                   size_t attempt_read_num_);
    virtual ~RfidController();

    /**
     * \brief Метод вщзвращает true, если устройство RFID подключено и готово к работе.
     */ 
    bool isInited();

    /**
     * \brief Метод выполняет команду по идентификатору и буферу параметров.
     * \param cmd_id_ Идентификатор команды.
     * \param data_buf_ Параметры команды.
     */ 
    bool execute(uint8_t cmd_id_, const std::vector<uint8_t> &data_buf_ = std::vector<uint8_t>()) override;
    
    /**
     * \brief Метод выполняет последовательный опрос RFID антенн и уточняет список меток.
     * \param count_  Количество опросов антенн.
     * \param with_counter_  Отправлять метки вместе с счётчиками меток.
     */
    void inventory(size_t count_, bool with_counter_ = false) override;

    /**
     * \brief Метод выполняет запуск последовательного опроса RFID антенн и уточняет список меток.
     * \param need_result_ Флаг обязательной отправки на сарвер результата сравнения буферов.
     */
    void startInventory(bool need_result_ = false) override;

    /**
     * \brief Метод завершает выполнение последовательного опроса RFID антенн и сравнивает прочитанные метки.
     */ 
    void stopInventory() override;

    /**
     * \brief Метод отправляет текущий буфер метов.
     */
    void accumulateBuffer() override;

    /**
     * \brief Метод текущий вероятностный буфер меток.
     */
    MapReadDatas getProbBuffer();

    /**
     * \brief Метод возвращающий текущие настройки антенн в виде JSON.
     */
    std::string getAntSettings();

    /**
     * \brief Метод устанавливает количество циклов опросаов антенн.
     * \param read_antenns_count_ Количество циклов чтений антенн.
     */
    void setReadAntennsCount(size_t read_antenns_count_) override;

    /**
     * \brief Метод устанавливает количество опросов для итогового результата.
     * \param read_antenns_count_ Количество циклов чтений антенн.
     */
    void setBufReadNumAttempt(size_t buf_read_num_attempt_) override;

    /**
     * \brief Метод возвращает текущее количество опросов для итогового результата.
     * \return Количество циклов чтений антенн.
     */
    size_t getBufReadNumAttempt() override;

    /**
     * \brief Метод выполняет команду по идентификатору и буферу параметров.
     * \param cmd_id_ Задержка между иттерациями чтения антенн.
     */
    void setReadAntennsTimeout(size_t timeout_) override;

    /**
     * \brief Метод запускает процесс выявления плохо читаемых меток.
     * \param iterations_num_ Количество иттераций при тестировании.
     */
    void findBrokenLabels(size_t iterations_num_) override;
};
} /// namespace robocooler
} /// namespace driver
