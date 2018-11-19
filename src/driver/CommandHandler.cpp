#include <ostream>

#include <boost/property_tree/json_parser.hpp>
#include <boost/optional/optional.hpp>

#include "Log.hpp"
#include "Commands.hpp"
#include "CommandHandler.hpp"
#include "GpioController.hpp"
#include "RfidController.hpp"


namespace bpt = boost::property_tree;

using namespace robocooler;
using namespace driver;


std::string CommandHandler::getLabelsString(const std::vector<std::string> &labels_) {
    std::ostringstream oss;
    std::vector<std::string> labels;
    for (auto &l : labels_) {
        labels.push_back("\"" + l + "\"");
    }
    std::copy(labels.begin(), labels.end() - 1, std::ostream_iterator<std::string>(oss, ","));
    oss << labels.back();
    return oss.str();
}


void CommandHandler::handlePlantHub(const bpt::ptree &pt_) {
    /// Выделить сообщени из "M" поля.
    std::string M_str = pt_.get<std::string>("M");
    if (M_str == "sendPutOrTakenGoodsByUser") { ///< Админ команда изъятия и внесения продуктов.  
        boost::optional<const bpt::ptree&> opt_A = pt_.get_child_optional("A");
        if (opt_A) { ///< Проверить наличие поля "A".
            handleA(pt_.get_child("A"));
        }
    } else if (M_str == "openRightDoor" or M_str == "openLeftDoor") {
        LOG(INFO) << "Open door.";
        if (_worker) {
            GpioControllerBase *door = _worker->getGpioController();
            if (door) {
                if (M_str == "openRightDoor") {
                    door->openRightDoor();
                } else if (M_str == "openLeftDoor") {
                    door->openLeftDoor();
                }
            }
            /// Остановить таймер завершения инвенторизации по закрытию двери.
            _stop_inventory_timer.reset();
            /// Запустить инвенторизацию.
            RfidControllerBase *rfidc = _worker->getRfidController();
            if (rfidc) {
                rfidc->startInventory(true);
            }
        }
    } else if (M_str == "closeLeftDoor" or M_str == "closeRightDoor") {
        LOG(INFO) << "Close door.";
        if (_worker) {
            GpioControllerBase *door = _worker->getGpioController();
            if (door) {
                if (M_str == "closeLeftDoor") {
                    door->closeLeftDoor();
                } else if (M_str == "closeRightDoor") {
                    door->closeRightDoor();
                }
            }
            /// Остановить перезапустить процесс инвенторизации .
            LOG(DEBUG) << "Stop by close door.";
            RfidControllerBase *rfidc = _worker->getRfidController();
            if (rfidc) {
                rfidc->stopInventory();
            }
            /// Запустить таймер закрытия двери.
            _close_inventory_timer = std::make_shared<Timer>(CLOSED_TIMEOUT, [this] {
                /// Запустить итоговую инвенторизацию.
                LOG(DEBUG) << "Start result inventory.";
                RfidControllerBase *rfidc = _worker->getRfidController();
                if (rfidc) {
                    rfidc->startInventory(false);
                }
                /// Запустить таймер завершения итоговой инвенторизации.
                _stop_inventory_timer = std::make_shared<Timer>(INVENTORY_TIMEOUT, [this] {
                    LOG(DEBUG) << "Stop result inventory by timer.";
                    RfidControllerBase *rfidc = _worker->getRfidController();
                    if (rfidc) {
                        rfidc->stopInventory();
                        rfidc->accumulateBuffer();
                    }
                });
            });
        }
    } else if (M_str == "getContent") { ///< { "H": "PlantHub", "M":"getContent”, "A":null }
        LOG(INFO) << "Get content.";
        if (_worker) {
            RfidControllerBase *rfidc = _worker->getRfidController();
            if (rfidc) {
                size_t count = rfidc->getBufReadNumAttempt();
                boost::optional<const bpt::ptree&> opt_A = pt_.get_child_optional("A");
                if (opt_A) { ///< Проверить наличие поля "A".
                    bpt::ptree A = pt_.get_child("A");
                    boost::optional<size_t> opt_brna = A.get_optional<size_t>("bufReadNumAttempt");
                    if (opt_brna) { ///< Проверить наличие поля "bufReadNumAttempt".
                        count = opt_brna.get();
                    }
                }
                rfidc->inventory(count, false);
            }
        }
    } else if (M_str == "configureRFIDDevice") { ///< {"H":"PlantHub", "M":"configureRFIDDevice", "A":{...}}
        LOG(INFO) << "Set RFID configuration.";
        boost::optional<const bpt::ptree&> opt_A = pt_.get_child_optional("A");
        if (opt_A) { ///< Проверить наличие поля "A".
            setRfidConfig(pt_.get_child("A"));
        }
    } else if (M_str == "getAntennasConfiguration") { ///< {"H":"PlantHub", "M":"getAntennasConfiguration"}
        LOG(INFO) << "Get RFID configuration.";
        if (_worker) {
            RfidControllerBase *rfidc = _worker->getRfidController();
            if (rfidc) {
                std::stringstream ss;
                ss << "{\"H\":\"antennasConfiguration\",\"M\":\"setCurrentConfiguration\","
                   << "\"A\":{" << rfidc->getAntSettings() << ",\"plantId\":" << _worker->getCoolerId() << "},}";
                _worker->send(ss.str());
            }
        }
    } else if (M_str == "updateAntennasRequestsSettings") { ///< {"M":"updateAntennasRequestsSettings","H":"PlantHub","A":{"readAntennsCount":3,"bufReadNumAttempt":2,"updateRecvDataTimeout":10}}
        LOG(INFO) << "Update RFID request settings.";
        if (_worker) {
            boost::optional<const bpt::ptree&> opt_A = pt_.get_child_optional("A");
            if (opt_A) { ///< Проверить наличие поля "A".
                setRequestsSettings(pt_.get_child("A"));
            }
        }
    } else if (M_str == "findBrokenLabels") { ///< {"M":"findBrokenLabels","H":"PlantHub","A":{"iterationsNumber":0}}
        LOG(INFO) << "Update RFID request settings.";
        if (_worker) {
            boost::optional<const bpt::ptree&> opt_A = pt_.get_child_optional("A");
            if (opt_A) { ///< Проверить наличие поля "A".
                findBrokenLabels(pt_.get_child("A"));
            }
        }
    } else {
        LOG(WARNING) << "\"M\": " << M_str;
    }
}


void CommandHandler::setRfidConfig(const bpt::ptree &pt_) {
    /// "A":{"frequencyRegion":{"startFrequency":0,"endFrequency":59,"region":1},"powers":[0,1,2,33]}}
    bpt::ptree A_pt = pt_;
    /// Передать настройки частотных диапазонов.
    boost::optional<bpt::ptree&> opt_frequencyRegion = A_pt.get_child_optional("frequencyRegion");
    if (opt_frequencyRegion) {
        bpt::ptree frequencyRegion_pt = opt_frequencyRegion.get();
        /// Получить настройки региона.
        RfidCmd::ESpektrumRegion region = RfidCmd::ESpektrumRegion::ETSI;
        boost::optional<uint8_t> opt_region = frequencyRegion_pt.get_optional<uint8_t>("region");
        if (opt_region) {
            region = static_cast<RfidCmd::ESpektrumRegion>(opt_region.get());
        }
        /// Получить настройки начала чистотного региона.
        uint8_t startFrequency = 0;
        boost::optional<uint8_t> opt_startFrequency = frequencyRegion_pt.get_optional<uint8_t>("startFrequency");
        if (opt_startFrequency) {
            startFrequency = opt_startFrequency.get();
        }
        /// Получить настройки конца чистотного региона.
        uint8_t endFrequency = 0;
        boost::optional<uint8_t> opt_endFrequency = frequencyRegion_pt.get_optional<uint8_t>("endFrequency");
        if (opt_endFrequency) {
            endFrequency = opt_endFrequency.get();
        }
        /// Передать настройки.
        if (opt_region and opt_startFrequency and opt_endFrequency) {
            RfidControllerBase *rfidc = _worker->getRfidController();
            if (rfidc) {
                LOG(DEBUG) << "Set Frequency region";
                RfidBuffer buf({static_cast<uint8_t>(region), startFrequency, endFrequency});
                rfidc->execute(static_cast<uint8_t>(RfidCid::cmd_set_frequency_region), buf);
            }
        }
    }
    /// Передать настройки мощности антенн.
    boost::optional<bpt::ptree&> opt_powers = A_pt.get_child_optional("powers");
    if (opt_powers) {
        RfidControllerBase *rfidc = _worker->getRfidController();
        if (rfidc and not opt_powers.get().empty()) {
            RfidBuffer buf;
            for (auto &v : opt_powers.get()) {
                buf.push_back(v.second.get_value<uint8_t>());
            }
            LOG(DEBUG) << "Set Power";
            rfidc->execute(static_cast<uint8_t>(RfidCid::cmd_set_output_power), buf);
        }
    }
    if (not opt_frequencyRegion and not opt_powers) {
        LOG(ERROR) << "Can`t find RFID settings in JSON.";
    }
}


void CommandHandler::sendProducts(const std::vector<std::string> &out_, const std::vector<std::string> &in_) {
    std::ostringstream oss;
    oss << "{\"H\":\"labeledGoods\",\"M\":\"setPutOrTakenGoods\",\"A\":{";
    oss << "\"userActionOUT\":[";
    if (not out_.empty()) {
        oss << getLabelsString(out_);
    }
    oss << "],";
    oss << "\"userActionIN\":[";
    if (not in_.empty()) {
        oss << getLabelsString(in_);
    }
    oss << "],";
    oss << "\"param\":{\"id\":";
    std::string cooler_id = "0";
    if (_worker) {
        cooler_id = _worker->getCoolerId();
    }
    oss << cooler_id << "}}}";
    LOG(INFO) << oss.str();
    /// Отрпавить JSON на сервер с данными о продуктах.
    if (_worker) {
        _worker->send(oss.str());
    }
}


void CommandHandler::sendCurProducts(const std::vector<std::string> &cur_) {
    std::ostringstream oss;
    oss << "{\"H\":\"labeledGoods\",\"M\":\"verifyLabelsSynchronization\",\"A\":{";
    std::string cooler_id = "0";
    if (_worker) {
        cooler_id = _worker->getCoolerId();
    }
    oss << "\"plantId\":\"" << cooler_id << "\",";
    oss << "\"labels\":[";
    if (not cur_.empty()) {
        oss << getLabelsString(cur_);
    }
    oss << "]}}";
    LOG(INFO) << oss.str();
    /// Отрпавить JSON на сервер с данными о продуктах.
    if (_worker) {
        _worker->send(oss.str());
    }
}


void CommandHandler::handleA(const bpt::ptree &pt_) {
    /// "A":{"O":[], "I":[], "PlantId": 0}
    bpt::ptree A_pt = pt_;
    boost::optional<std::string> opt_PlantId = A_pt.get_optional<std::string>("PlantId");
    if (opt_PlantId) {
        std::string cooler_id = "0";
        if (_worker) {
            cooler_id = _worker->getCoolerId();
        }
        if (opt_PlantId.get() == cooler_id) {
            /// Проверить наличие поля "O" c массивом изъятых продуктов.
            boost::optional<bpt::ptree&> opt_O = A_pt.get_child_optional("O");
            std::vector<std::string> out;
            if (opt_O) {
                for (bpt::ptree::value_type &v : A_pt.get_child("O")) {
                    out.push_back(v.second.data());
                }
            }
            /// Проверить наличие поля "I" c массивом добавленных продуктов.
            boost::optional<bpt::ptree&> opt_I = A_pt.get_child_optional("I");
            std::vector<std::string> in;
            if (opt_I) {
                for (bpt::ptree::value_type &v : A_pt.get_child("I")) {
                    in.push_back(v.second.data());
                }
            }
            sendProducts(out, in);
        } else {
            LOG(WARNING) << "PlantId is foreign " << opt_PlantId.get();
        }
    } else {
        LOG(ERROR) << "Can`t find PlantId";
    }
}


void CommandHandler::setRequestsSettings(const bpt::ptree &pt_) {
    /// "A":{"readAntennsCount":3,"bufReadNumAttempt":2,"updateRecvDataTimeout":10}
    bpt::ptree A_pt = pt_;
    boost::optional<size_t> opt_readAntennsCount = A_pt.get_optional<size_t>("readAntennsCount");
    if (opt_readAntennsCount) {
        if (_worker) {
            RfidControllerBase *rfidc = _worker->getRfidController();
            if (rfidc) {
                rfidc->setReadAntennsCount(opt_readAntennsCount.get());
            }
        }
    } else {
        LOG(ERROR) << "Can`t find readAntennsCount";
    }
    boost::optional<size_t> opt_bufReadNumAttempt = A_pt.get_optional<size_t>("bufReadNumAttempt");
    if (opt_bufReadNumAttempt) {
        if (_worker) {
            RfidControllerBase *rfidc = _worker->getRfidController();
            if (rfidc) {
                rfidc->setBufReadNumAttempt(opt_bufReadNumAttempt.get());
            }
        }
    } else {
        LOG(ERROR) << "Can`t find bufReadNumAttempt";
    }
    boost::optional<size_t> opt_updateRecvDataTimeout = A_pt.get_optional<size_t>("updateRecvDataTimeout");
    if (opt_updateRecvDataTimeout) {
        if (_worker) {
            RfidControllerBase *rfidc = _worker->getRfidController();
            if (rfidc) {
                rfidc->setReadAntennsTimeout(opt_updateRecvDataTimeout.get());
            }
        }
    } else {
        LOG(ERROR) << "Can`t find updateRecvDataTimeout";
    }
}


void CommandHandler::findBrokenLabels(const bpt::ptree &pt_) {
    /// "A":{"iterationsNumber":0}
    bpt::ptree A_pt = pt_;
    boost::optional<size_t> opt_iterationsNumber = A_pt.get_optional<size_t>("iterationsNumber");
    if (opt_iterationsNumber) {
        if (_worker) {
            RfidControllerBase *rfidc = _worker->getRfidController();
            if (rfidc) {
                rfidc->findBrokenLabels(opt_iterationsNumber.get());
            }
        }
    } else {
        LOG(ERROR) << "Can`t find requestsNumber";
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


CommandHandler::CommandHandler(WorkerBase *worker_) 
    : _worker(worker_) {
    LOG(DEBUG);
}


CommandHandler::~CommandHandler() {
    LOG(DEBUG);
    /// Остановить таймер завершения инвенторизации по закрытию двери.
    _stop_inventory_timer.reset();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    
void CommandHandler::handle(const std::string &json_) {    
    LOG(DEBUG) << json_;
    /// Отправка старта подключения.
    try {
        bpt::ptree pt;
        std::stringstream ss(json_);
        bpt::read_json(ss, pt);
        if (not pt.empty()) {
            /// Проверить посылки с "H" и "M"
            boost::optional<std::string> opt_H = pt.get_optional<std::string>("H");
            boost::optional<std::string> opt_M = pt.get_optional<std::string>("M");
            if (opt_H and opt_M) {
                if (opt_H.get() == "PlantHub") {
                    handlePlantHub(pt);
                } else {
                    LOG(WARNING) << "\"H\" is not PlantHub: " << opt_H.get();
                }
            } else {
                LOG(ERROR) << "Can`t find \"H\" or \"M\" tags: " << json_;
            }
        } 
    } catch (const std::exception &e) {
        LOG(ERROR) << e.what();
    }
}


void CommandHandler::restartStopInventoryTimeout() {
    if (_close_inventory_timer) {
        _close_inventory_timer->restart();
    }
}

