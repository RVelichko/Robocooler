#include <iostream>
#include <functional>
#include <atomic>

#include "Log.hpp"
#include "Timer.hpp"
#include "WsClientWorker.hpp"
#include "CommandHandler.hpp"
#include "RfidController.hpp"

using namespace robocooler;
using namespace driver;

namespace ph = std::placeholders;
namespace chr = std::chrono;

typedef utils::Timer Timer;
typedef std::shared_ptr<Timer> PTimer;
typedef RfidCmd::EBaudrate RfidBaudrate;
typedef RfidCmd::ESpektrumRegion RfidESpektrumRegion;
typedef RfidCommandsHandler RfidCmdHdl;
typedef RfidController Ctrl;


void RfidController::notifyOneCmdResult(RfidCid cmd_id_) {
    //G(DEBUG) << RfidCmd::cmdToString(cmd_id_);
    std::unique_lock<std::mutex> lock(_mutex);
    WaitCmdResultIter iter = _cmd_conditions.find(cmd_id_);
    if (iter not_eq _cmd_conditions.end()) {
        PCondition condition_complete = iter->second;
        /// Уведомить условный объект синхронизации.
        condition_complete->notify_all();
    }
}


void RfidController::waitCmdResult(RfidCid cmd_id_) {
    std::unique_lock<std::mutex> lock(_mutex);
    WaitCmdResultIter iter = _cmd_conditions.find(cmd_id_);
    if (iter == _cmd_conditions.end()) {
        PCondition condition_complete = std::make_shared<std::condition_variable>();
        auto insert_iter = _cmd_conditions.insert(std::make_pair(cmd_id_, condition_complete));
        iter = insert_iter.first;
    }
    iter->second->wait(lock);
}


bool RfidController::extLockWaitCmdResult(RfidCid cmd_id_, std::unique_lock<std::mutex>& lock_, size_t unlock_timout_) {
    WaitCmdResultIter iter = _cmd_conditions.find(cmd_id_);
    if (iter == _cmd_conditions.end()) {
        PCondition condition_complete = std::make_shared<std::condition_variable>();
        auto insert_iter = _cmd_conditions.insert(std::make_pair(cmd_id_, condition_complete));
        iter = insert_iter.first;
    } else {
        iter->second->notify_all();
    }
    PCondition condition = iter->second;
    bool is_no_timeout = true;
    _notify_timer = std::shared_ptr<Timer>(new Timer(unlock_timout_, [&] {
        LOG(WARNING) << "\"" << RfidCmd::cmdToString(cmd_id_) << "\" is lock.";
        is_no_timeout = false;
        condition->notify_all();
    }));
    condition->wait(lock_);
    _notify_timer.reset();
    return is_no_timeout;
}


void RfidController::runSerial() {
    while (_is_runing) {
        uint8_t byte = 0;
        int rlen = _tty_io->read(&byte, 1);
        if (rlen and rlen not_eq ERROR_LEN and _rfid_handler) {
            RfidCid cid = _rfid_handler->receivePacket(static_cast<uint8_t>(byte));
            if (RfidCid::cmd_none not_eq cid) {
                notifyOneCmdResult(cid);
            }
        }
        /// Отпустить процессор.
        std::this_thread::sleep_for(chr::milliseconds(1));
    };
}


void RfidController::readFromBufferAndReset() {
    uint16_t tag_count = 0;
    /// Прочитать количество меток, находящихся в буфере.
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_rfid_handler) {
            RfidCmd *rfid_cmd = _rfid_handler->getCommand();
            if (rfid_cmd) {
                rfid_cmd->getInventoryBufferTagCount();
                extLockWaitCmdResult(RfidCid::cmd_get_inventory_buffer_tag_count, lock, UNLOCK_TIMEOUT);
                tag_count = _rfid_handler->getCurTagCount();
                LOG(DEBUG) << "In buffer " << tag_count << " tag counts";
                /// Очистить приёмный буфер.
                _buffered_data.clear();
                /// Читать буфер со сбросом.
                rfid_cmd->getAndResetInventoryBuffer();
            }
        }
    }
    /// Принять метки из буфера по количеству с перезапуском таймера.
    if (not tag_count) {
        LOG(ERROR) << "Tags count is 0.";
    } else {
        uint16_t cur_read_data_size = 0;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            extLockWaitCmdResult(RfidCid::all_tags_receaved, lock, ALL_TAGS_RECV_TIMEOUT);
            LOG(WARNING) << "Tags count: " << tag_count;
            _cur_read_data = _buffered_data;
            cur_read_data_size = _cur_read_data.size();
        };
        if (cur_read_data_size not_eq tag_count) {
            LOG(ERROR) << "Receaved tags count is " << cur_read_data_size << " [" << tag_count << "].";
        } else {
            LOG(DEBUG) << "Receaved tags count is " << cur_read_data_size << " [" << tag_count << "].";
        }
    }
}


void RfidController::bufferReadProcess() {
    LOG(DEBUG);
    PTimer timer;
    /// Выполнить опрос по каждой антенне.
    //for (uint8_t aq = 0; aq < static_cast<uint8_t>(RfidCmd::EWorkAntenna::QUANTITY); ++aq) {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_rfid_handler) {
                RfidCmd *rfid_cmd = _rfid_handler->getCommand();
                if (rfid_cmd) {
                    /// Установить активную антенну.
                    //rfid_cmd->setWorkAntenna(static_cast<RfidWorkAntenna>(aq));
                    //extLockWaitCmdResult(RfidCid::cmd_set_work_antenna, lock, UNLOCK_TIMEOUT);
                    /// Инвенторизировать.
                    rfid_cmd->inventory(0xff);
                    extLockWaitCmdResult(RfidCid::cmd_inventory, lock, UNLOCK_TIMEOUT);
                }
            }
        }
        /// Подождать перед считыванием из следующей антенны.
        std::this_thread::sleep_for(chr::milliseconds(UPDATE_RECV_DATA_TIMEOUT));
    //}
    /// Прочитать буфер.
    readFromBufferAndReset();
}


void RfidController::currentBuffer() {
    LOG(DEBUG);
    /// Вывести все полученные метки.
    std::stringstream cur_ss;
    std::stringstream snd_cur_ss;
    size_t cur_size = 0;
    {
        std::unique_lock<std::mutex> lock(_mutex);
        cur_size = _accumulate_data.size();
        for (auto data : _accumulate_data) {
            snd_cur_ss << "\"" << data.first << "\",";
            cur_ss << "\"" << data.first <<  ":" << data.second._readed_num << "\",";
        }
    }
    std::string prods_str;
    std::string snd_prods_str;
    if (not cur_ss.str().empty()) {
        snd_prods_str = std::string(snd_cur_ss.str().c_str(), (snd_cur_ss.str().size() - 1));
        prods_str = std::string(cur_ss.str().c_str(), (cur_ss.str().size() - 1));
    }
    LOG(INFO) << "ACM:\n-------------------------------------------------------\n"
              << prods_str
              << "\nsize = " << cur_size << "\n"
              <<     "\n_______________________________________________________\n";
    /// Зафиксировать изменения на сервере.
    if (_worker) {
        std::stringstream ss;
        ss << "{\"H\": \"labeledGoods\",\"M\":\"verifyLabelsSynchronization\",\"A\":{\"labels\":[";
        ss << snd_prods_str;
        ss << "],\"plantId\":" << _worker->getCoolerId() << "}}";
        _worker->send(ss.str());
    }
}


void RfidController::verifyBuffer() {
    LOG(DEBUG);
    /// Вывести все полученные метки.
    std::stringstream snd_cur_ss;
    std::stringstream cur_ss;
    size_t cur_size = 0;
    {
        std::unique_lock<std::mutex> lock(_mutex);
        cur_size = _prob_read_data.size();
        for (auto data : _prob_read_data) {
            snd_cur_ss << "\"" << data.first <<  ":" << data.second._readed_num << "\",";
            cur_ss << data.first <<  " : " << data.second._readed_num << "\n";
        }
    }
    std::string snd_prods_str;
    if (not snd_cur_ss.str().empty()) {
        snd_prods_str = std::string(snd_cur_ss.str().c_str(), (snd_cur_ss.str().size() - 1));
    }
    LOG(INFO) << "PROB:\n------------------------------------------------------\n"
              << cur_ss.str()
              << "\nsize = " << cur_size << "\n"
              <<     "\n_______________________________________________________\n";
    /// Зафиксировать изменения на сервере.
    if (_worker) {
        std::stringstream ss;
        ss << "{\"H\": \"labeledGoods\",\"M\":\"periodicVerifyLabels\",\"A\":{\"labels\":[";
        ss << snd_prods_str;
        ss << "],\"plantId\":" << _worker->getCoolerId() << "}}";
        _worker->send(ss.str());
    }
}


void RfidController::compareBuffers(bool need_result_) {
    LOG(DEBUG);
    /// Вывести все полученные метки.
    bool is_compare = need_result_;
    size_t old_count = 0;
    size_t cur_count = 0;
    std::stringstream cur_ss;
    {
        std::unique_lock<std::mutex> lock(_mutex);
        for (auto data : _cur_read_data) {
            cur_ss << data.first <<  ":" << data.second._readed_num << "\n";
        }
        old_count = _read_data.size();
        cur_count = _cur_read_data.size();
    }
    LOG(TRACE) << "CUR:\n-------------------------------------------------------\n"
               << cur_ss.str()
               << "old=" << old_count << "; cur=" << cur_count
               <<    "\n_______________________________________________________\n";
    /// Проверить добавленные метки.
    std::vector<std::string> in_prods;
    std::stringstream add_ss;
    size_t add_count = 0;
    {
        std::unique_lock<std::mutex> lock(_mutex);
        for (auto data : _cur_read_data) {
            ReadDataIter iter = _read_data.find(data.first);
            if (iter == _read_data.end()) {
                ++add_count;
                add_ss << data.first << ":" << data.second._readed_num << "\n";
                in_prods.push_back(data.first);
            }
        }
    }
    if (not add_ss.str().empty()) {
        is_compare = true;
        LOG(INFO) << "ADD:\n-------------------------------------------------------\n"
                   << add_ss.str()
                   << "add=" << add_count
                   <<    "\n_______________________________________________________\n";
    }
    /// Проверить изъятые метки.
    std::vector<std::string> out_prods;
    std::stringstream rem_ss;
    size_t rem_count = 0;
    {
        std::unique_lock<std::mutex> lock(_mutex);
        for (auto data : _read_data) {
            ReadDataIter iter = _cur_read_data.find(data.first);
            if (iter == _cur_read_data.end()) {
                ++rem_count;
                rem_ss << data.first << ":" << data.second._readed_num << "\n";
                out_prods.push_back(data.first);
            }
        }
    }
    if (not rem_ss.str().empty()) {
        is_compare = true;
        LOG(INFO) << "REM:\n-------------------------------------------------------\n"
                   << rem_ss.str()
                   << "rem=" << rem_count
                   <<    "\n_______________________________________________________\n";
    }
    /// Зафиксировать изменения на сервере.
    if (_worker and is_compare and need_result_) {
        CommandHandlerBase *cmd_handler = _worker->getCommandHandler();
        if (cmd_handler) {
            cmd_handler->sendProducts(out_prods, in_prods);
        }
    }
}


void RfidController::onReadData(const RfidCmd::ReadCmdData &read_data_) {
    std::string EPC_str = RfidCmdHdl::toString(read_data_._EPC);
    /// Сохранить очередную метку в буфер, если метка была получена.
    uint16_t cur_read_data_size = 0;
    RfidCmd::ReadCmdData data;
    {
        std::unique_lock<std::mutex> lock(_mutex);
        /// Накопить метки.
        auto iter = _prob_read_data.find(EPC_str);
        if (iter not_eq _prob_read_data.end()) {
            ++iter->second._readed_num;
            data = iter->second;
        } else {
            data = read_data_;
            data._readed_num = 1;
            _prob_read_data.insert(std::make_pair(EPC_str, data));
        }
        /// Сохранить метки, аккумулируемые на иттерацию.
        _accumulate_data.insert(std::make_pair(EPC_str, data));
        /// Сохранить текущие данные, для фиксации изменений.
        _buffered_data.insert(std::make_pair(EPC_str, data));
        cur_read_data_size = _buffered_data.size();
    }
    LOG(TRACE) << "[" << cur_read_data_size << "]: read: " << EPC_str << ": " << data._readed_num;
    if (read_data_._TagCount not_eq 0 and read_data_._TagCount == cur_read_data_size) {
        notifyOneCmdResult(RfidCid::all_tags_receaved);
    }
}


void RfidController::startPeriodicInventory() {
    LOG(DEBUG);
    _periodic_inventory_timeout = std::make_shared<Timer>(PERIODIC_INVENTORY_TIMEOUT, [this] {
        LOG(DEBUG) << "Periodic timeout is active.";
        inventory(true);
    });
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


RfidController::RfidController(WorkerBase *worker_,
                               const std::string &device_,
                               size_t reread_timeout_,
                               size_t close_read_num_,
                               size_t attempt_read_num_)
    : _worker(worker_)
    , _is_inited(false)
    , _is_runing(false)
    , _is_inventory(false)
    , _is_inventory_run(false)
    , _read_count(READ_ANTENNS_COUNT)
    , _reread_timeout(reread_timeout_)
    , _close_read_num(close_read_num_)
    , _attempt_read_num(attempt_read_num_) {
    _tty_io = std::make_shared<utils::TtyIo>(device_, B115200);
    if (_tty_io and _tty_io->isInit()) {
        /// Инициализация обработчика.
        _rfid_handler = std::make_shared<RfidCmdHdl>(_tty_io.get());
        /// Запуск потока.
        _is_runing = true;
        _thread = std::shared_ptr<Thread>(
            new Thread(std::bind(&Ctrl::runSerial, this)),
            [this](Thread *p_) {
                _is_runing = false;
                p_->join();
                delete p_;
        });
        /// Получить информацию об устройстве RFID.
        if (_rfid_handler) {
            std::unique_lock<std::mutex> lock(_mutex);
            RfidCmd *rfid_cmd = _rfid_handler->getCommand();
            if (rfid_cmd) {
                rfid_cmd->setRfidAddres(RFID_ADDR);
                rfid_cmd->getFirmwareVersion();
                /// Подождать ответа.
                if (not extLockWaitCmdResult(RfidCid::cmd_get_firmware_version, lock, UNLOCK_TIMEOUT)) {
                    _is_inited = false;
                    _is_runing = false;
                } else {
                    _is_inited = true;
                }
            }
        }
        _notify_timer.reset();
        if (_is_inited) {
            /// Проинициализировать функтор приёма меток.
            _rfid_handler->initOnReadDataFunc(std::bind(&RfidController::onReadData, this, ph::_1));
            /// Запустить периодическую инвенторизацию.
            //startPeriodicInventory();
            /// Сбросить.
            //if (_rfid_handler) {
            //    std::unique_lock<std::mutex> lock(_mutex);
            //    RfidCmd *rfid_cmd = _rfid_handler->getCommand();
            //    if (rfid_cmd) {
            //        rfid_cmd->reset();
            //    }
            //}
        }
    } else {
        LOG(ERROR) << "Error: Could not open serial port " << device_ << ".";
    }
}


RfidController::~RfidController() {
    /// Остановить инвенторизацию.
    stopInventory();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool RfidController::isInited() {
    return _is_inited;
}


bool RfidController::execute(uint8_t cmd_id_, const std::vector<uint8_t> &data_buf_) {
    bool res = true;
    LOG(DEBUG) << RfidCmdHdl::toString(static_cast<uint8_t>(cmd_id_)) << " " << RfidCmdHdl::toString(data_buf_);
    std::unique_lock<std::mutex> lock(_mutex); ///< Точка доступа.
    if (_rfid_handler) {
        RfidCmd *rfid_cmd = _rfid_handler->getCommand();
        if (rfid_cmd) {
            switch (static_cast<RfidCid>(cmd_id_)) {
                case RfidCid::cmd_reset:
                    rfid_cmd->reset();
                    break;
                case RfidCid::cmd_set_uart_baudrate:
                    if (data_buf_.size() == 1) {
                        rfid_cmd->setUartBaudrate(static_cast<RfidBaudrate>(data_buf_[0]));
                    } else {
                        res = false;
                    }
                    break;
                case RfidCid::cmd_get_firmware_version:
                    rfid_cmd->getFirmwareVersion();
                    break;
                case RfidCid::cmd_set_work_antenna:
                    if (data_buf_.size() == 1) {
                        rfid_cmd->setWorkAntenna(static_cast<RfidWorkAntenna>(data_buf_[0]));
                    } else {
                        res = false;
                    }
                    break;
                case RfidCid::cmd_get_work_antenna:
                    rfid_cmd->getWorkAntenna();
                    break;
                case RfidCid::cmd_set_output_power:
                    if (data_buf_.size() == 1) {
                        rfid_cmd->setOutputPower(data_buf_[0], data_buf_[0], data_buf_[0], data_buf_[0]);
                    } else if (data_buf_.size() == 4) {
                        rfid_cmd->setOutputPower(data_buf_[0], data_buf_[1], data_buf_[2], data_buf_[3]);
                    } else {
                        res = false;
                    }
                    break;
                case RfidCid::cmd_get_output_power:
                    rfid_cmd->getOutputPower();
                    break;
                case RfidCid::cmd_set_frequency_region:
                    if (data_buf_.size() == 3) {
                        rfid_cmd->setFrequencyRegion(static_cast<RfidESpektrumRegion>(data_buf_[0]), data_buf_[1], data_buf_[2]);
                    } else if (data_buf_.size() == 2) {
                        rfid_cmd->setFrequencyRegion(static_cast<RfidESpektrumRegion>(data_buf_[0]), data_buf_[1], data_buf_[1]);
                    } else {
                        res = false;
                    }
                    break;
                case RfidCid::cmd_get_frequency_region:
                    rfid_cmd->getFrequencyRegion();
                    break;
                case RfidCid::cmd_inventory:
                    if (not data_buf_.empty()) {
                        rfid_cmd->inventory(data_buf_[0]);
                    } else {
                        rfid_cmd->inventory(0x0);
                    }
                    break;
                case RfidCid::cmd_read: {
                        RfidCmd::EReadMemBank mem_bank = data_buf_.size() ?
                                                     static_cast<RfidCmd::EReadMemBank>(data_buf_[0]) :
                                                     RfidCmd::EReadMemBank::EPC;
                        rfid_cmd->read(mem_bank, 0x0, 0x01);
                    }
                    break;
                case RfidCid::cmd_get_inventory_buffer:
                    rfid_cmd->getInventoryBuffer();
                    break;
                case RfidCid::cmd_get_and_reset_inventory_buffer:
                    rfid_cmd->getAndResetInventoryBuffer();
                    break;
                case RfidCid::cmd_get_inventory_buffer_tag_count:
                    rfid_cmd->getInventoryBufferTagCount();
                    break;
                case RfidCid::cmd_reset_inventory_buffer:
                    rfid_cmd->resetInventoryBuffer();
                    break;
                default:
                    LOG(WARNING) << "Undeclared command: " << RfidCmdHdl::toString(static_cast<uint8_t>(cmd_id_));
                    res = false;
                    break;
            }
        }
    }
    return res;
}


void RfidController::inventory(size_t count_, bool with_count_) {
    LOG(DEBUG);
    if (_is_inventory_run) {
        LOG(WARNING) << "Inventory is running.";
    } else {
        /// Запустить поток опроса.
        _inv_thread = std::shared_ptr<Thread>(new Thread([this, count_, with_count_] {
            _is_inventory_run = true;
            LOG(DEBUG) << "Inventory thread start.";
            /// Сбросить аккумулируемый буфер меток.
            {
                std::unique_lock<std::mutex> lock(_mutex);
                LOG(TRACE) << "Clear accumulated buf: " << _accumulate_data.size();
                _accumulate_data.clear();
            }
            /// Для тестирования необходимо сбросить вероятностный буфер.
            if (with_count_) {
                std::unique_lock<std::mutex> lock(_mutex);
                LOG(TRACE) << "Clear probability buf: " << _prob_read_data.size();
                _prob_read_data.clear();
            }
            for (uint8_t i = 0; i < count_; ++i) {
                /// Сбросить буфер меток.
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    LOG(TRACE) << "Clear cur buf: " << _cur_read_data.size();
                    _cur_read_data.clear();
                }
                /// Проинициализировать и прочитать буфер меток, по завершению - сбросить.
                LOG(DEBUG) << "Buf read 2 {";
                bufferReadProcess();
                LOG(DEBUG) << "Buf read 2 }";
                /// Проверять метки на изменение их количества каждую попытку.
                compareBuffers(false);
                /// Зафиксировать изменения.
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    _read_data = _cur_read_data;
                    LOG(TRACE) << "Save cur buf: " << _read_data.size();
                }
                /// Подождать после выполнения текущей операции.
                if (_is_inventory) {
                    std::this_thread::sleep_for(chr::milliseconds(_reread_timeout));
                }
            }
            LOG(DEBUG) << "Inventory thread complete.";
            if (with_count_) {
                verifyBuffer();
            } else {
                currentBuffer();
            }
            _is_inventory_run = false;
        }), [](Thread *p_) {
            LOG(DEBUG) << "Reset with join.";
            p_->join();
            delete p_;
        });
    }
    /// Запустить периодическую инвенторизацию.
    //startPeriodicInventory();
}


void RfidController::startInventory(bool need_result_) {
    LOG(DEBUG);
    if (_is_inventory_run) {
        stopInventory();
    }
    /// Запустить поток опроса.
    _inv_thread = std::shared_ptr<Thread>(new Thread([this, need_result_] {
        _is_inventory_run = true;
        _is_inventory = true;
        /// Сбросить аккумулируемый буфер меток.
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _accumulate_data.clear();
            LOG(TRACE) << "Clear accumulated buf: " << _accumulate_data.size();
        }
        while (_is_inventory) {
            /// Сбросить буфер меток.
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _cur_read_data.clear();
                LOG(TRACE) << "Clear cur buf: " << _cur_read_data.size();
            }
            /// Проинициализировать и прочитать буфер меток, по завершению - сбросить.
            LOG(DEBUG) << "Buf read 1 {";
            if (need_result_) {
                for (size_t i = 0; i < _attempt_read_num; ++i) {
                    LOG(DEBUG) << "STEP: " << i;
                    bufferReadProcess();
                }
            } else {
                bufferReadProcess();
            }
            LOG(DEBUG) << "Buf read 1 }";
            /// Проверять метки на изменение их количества каждую попытку.
            compareBuffers(false); ///need_result_);
            ///< Зафиксировать изменения.
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _read_data = _cur_read_data;
                //_read_data = _accumulate_data;
                LOG(TRACE) << "Save cur buf: " << _read_data.size();
            }
            /// Подождать после выполнения текущей операции.
            if (_is_inventory) {
                std::this_thread::sleep_for(chr::milliseconds(_reread_timeout));
            }
        }
    }), [this](Thread *p_) {
        LOG(DEBUG) << "Reset with join.";
        _is_inventory = false;
        p_->join();
        delete p_;
        _is_inventory_run = false;
    });
}


void RfidController::stopInventory() {
    LOG(DEBUG);
    _inv_thread.reset();
}


void RfidController::accumulateBuffer() {
    if (not _is_inventory_run and not _is_inventory) {
        /// Отправить текущее содержимое холодильника на сервер.
        currentBuffer();
    }
}


MapReadDatas RfidController::getProbBuffer() {
    std::unique_lock<std::mutex> lock(_mutex);
    return _prob_read_data;
}


std::string RfidController::getAntSettings() {
    std::stringstream ss_ant_sets;
    if (not _is_inventory) {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_rfid_handler) {
                RfidCmd *rfid_cmd = _rfid_handler->getCommand();
                if (rfid_cmd) {
                    /// Получить частотные настройки.
                    rfid_cmd->getFrequencyRegion();
                    extLockWaitCmdResult(RfidCid::cmd_get_frequency_region, lock, UNLOCK_TIMEOUT);
                    /// Получить мощность антенн.
                    rfid_cmd->getOutputPower();
                    extLockWaitCmdResult(RfidCid::cmd_get_output_power, lock, UNLOCK_TIMEOUT);
                }
            }
        }
        /// Зафиксировать полученные данные.
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_rfid_handler) {
                _ant_sets = _rfid_handler->getAntSettings();
                ss_ant_sets << "\"frequencyRegion\":{"
                            << "\"startFrequency\":" << static_cast<uint16_t>(_ant_sets._start_freq) << ","
                            << "\"endFrequency\":" << static_cast<uint16_t>(_ant_sets._end_freq) << ","
                            << "\"region\":" << static_cast<uint16_t>(_ant_sets._region)
                            << "},\"powers\":["
                            << static_cast<uint16_t>(_ant_sets._ant_pow_1) << ","
                            << static_cast<uint16_t>(_ant_sets._ant_pow_2) << ","
                            << static_cast<uint16_t>(_ant_sets._ant_pow_3) << ","
                            << static_cast<uint16_t>(_ant_sets._ant_pow_4) << "]";
            }
        }
    } else {
        LOG(WARNING) << "Inventory is running.";
    }
    return ss_ant_sets.str();
}


void RfidController::setReadAntennsCount(size_t read_antenns_count_) {
    LOG(DEBUG) << read_antenns_count_;
    std::unique_lock<std::mutex> lock(_mutex);
    _attempt_read_num = read_antenns_count_;
}


void RfidController::setBufReadNumAttempt(size_t buf_read_num_attempt_) {
    LOG(DEBUG) << buf_read_num_attempt_;
    std::unique_lock<std::mutex> lock(_mutex);
    _close_read_num = buf_read_num_attempt_;
}


size_t RfidController::getBufReadNumAttempt() {
    LOG(DEBUG) << _close_read_num;
    return _close_read_num;
}


void RfidController::setReadAntennsTimeout(size_t timeout_) {
    LOG(DEBUG) << timeout_;
    std::unique_lock<std::mutex> lock(_mutex);
    _reread_timeout = timeout_;
}


void RfidController::findBrokenLabels(size_t iterations_num_) {
    if (not iterations_num_) {
        iterations_num_ = 1;
    }
    inventory(iterations_num_, true);
}
