#include <sstream>
#include <iomanip>

#include "Log.hpp"
#include "Message.hpp"
#include "Commands.hpp"
#include "CommandsHandler.hpp"


using namespace robocooler;
using namespace rfid;

typedef Message::Buffer Buffer;
typedef Command::ECommandId Cid;


Cid CommandsHandler::onMessage(const Message &msg_) {
    Cid res = Cid::cmd_none;
    Command* cmd = getCommand();
    if (cmd) {
        res = cmd->receive(msg_);
    } else {
        LOG(ERROR) << "Can`t find command for responce: " << toString(msg_.getAryTranData());
    }
    return res;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


std::string CommandsHandler::toString(const Buffer &recv_buf_) {
    std::stringstream ss;
    for (uint8_t b : recv_buf_) {
        ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<uint16_t>(b) << " ";
    }
    return ss.str().substr(0, ss.str().size() - 1);
}


std::string CommandsHandler::toString(uint8_t b_) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<uint16_t>(b_);
    return ss.str();
}


std::string CommandsHandler::freqCodeToString(uint8_t code_) {
    std::stringstream ss;
        float f = 865.0;
        if (code_ <= 0x3b) {
            f += static_cast<float>(code_) * 0.5;
        }
    ss << std::to_string(f);
    return ss.str();
}


std::string CommandsHandler::specRegionToString(Command::ESpektrumRegion region_) {
    std::stringstream ss;
    switch(region_) {
        case Command::ESpektrumRegion::FCC: ss << "FCC"; break;
        case Command::ESpektrumRegion::ETSI: ss << "ETSI"; break;
        case Command::ESpektrumRegion::CHN: ss << "CHN"; break;
        default: ss << "Undeclared"; break;
    }
    return ss.str();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


CommandsHandler::CommandsHandler(utils::TtyIo *tty_io_)
    : _tty_io(tty_io_)
    , _cur_tag_count(0)
    , _is_error(false) {
    LOG(DEBUG);
    _command = std::make_shared<Command>(this);
} 


CommandsHandler::~CommandsHandler() {
    _command.reset();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void CommandsHandler::initOnReadDataFunc(const OnReadDataFunc &on_read_data_func_) {
    _on_read_data_func = on_read_data_func_;
}


bool CommandsHandler::isError() {
    return _is_error;
}


const Command::ReadCmdData& CommandsHandler::getCurReadData() {
    return _cur_read_data;
}


Command* CommandsHandler::getCommand() {
    if (_command) {
        return _command.get();
    }
    return nullptr;
}


uint16_t CommandsHandler::getCurTagCount() {
    return _cur_tag_count;
}


Command::AntSettings CommandsHandler::getAntSettings() {
    return _ant_sets;
}


bool CommandsHandler::sendMessage(const Message &msg_) {
    //LOG(DEBUG);
    bool res = false;
    if (_tty_io->isInit()) {
        const Buffer &pack = msg_.getAryTranData();
        _tty_io->write(pack);
        res = true;
    }
    return res;
}


Cid CommandsHandler::receivePacket(uint8_t b_) {
    Cid res = Cid::cmd_none;
    _recv_buf.push_back(b_);
    if (RFID_PACK_MINLEN <= _recv_buf.size()) {
        if (_recv_buf[0] == RFID_HEAD) {
            size_t s = _recv_buf[1] + 2; ///< Байт заголовка и байт размера не входят в размер блока данны.
            if (s <= _recv_buf.size()) {
                Buffer block(_recv_buf.begin(), _recv_buf.begin() + s);
                /// Обработать пакет.
                res = onMessage(Message(block));
                /// Очистить обработанный блок данных.
                _recv_buf.erase(_recv_buf.begin(), _recv_buf.begin() + s);
            }
        } else {
            LOG(ERROR) << "Incorrect data block: " << toString(_recv_buf);
            while (not _recv_buf.empty() and  _recv_buf[0] not_eq RFID_HEAD) {
                _recv_buf.erase(_recv_buf.begin());
            }
        }
    }
    return res;
}


void CommandsHandler::onError(const std::string &err_) {
    _is_error = true;
    _cur_read_data = Command::ReadCmdData();
}


void CommandsHandler::onSetUartBaudrate() {
    _is_error = false;
}


void CommandsHandler::onGetFirmwareVersion(uint8_t major, uint8_t minor) {
    _is_error = false;
}


void CommandsHandler::onSetWorkAntenna() {
    _is_error = false;
}


void CommandsHandler::onGetWorkAntenna(Command::EWorkAntenna work_ant_) {
    _is_error = false;
}


void CommandsHandler::onSetOutputPower() {
    _is_error = false;
}


void CommandsHandler::onGetOutputPower(uint8_t ant_pow_1_, uint8_t ant_pow_2_, uint8_t ant_pow_3_, uint8_t ant_pow_4_) {
    LOG(DEBUG) << "Output power "
               << ": A1: " << std::to_string(ant_pow_1_)
               << ", A2: " << std::to_string(ant_pow_2_)
               << ", A3: " << std::to_string(ant_pow_3_)
               << ", A4: " << std::to_string(ant_pow_4_) << " dBm";
    _ant_sets._ant_pow_1 = ant_pow_1_;
    _ant_sets._ant_pow_2 = ant_pow_2_;
    _ant_sets._ant_pow_3 = ant_pow_3_;
    _ant_sets._ant_pow_4 = ant_pow_4_;
    _is_error = false;
}


void CommandsHandler::onGetFrequencyRegion(Command::ESpektrumRegion region_, uint8_t start_freq_, uint8_t end_freq_) {
    LOG(DEBUG) << "Frequency region: " << specRegionToString(region_) << ": "
               << freqCodeToString(start_freq_) << " -:- " << freqCodeToString(end_freq_) << " MHz";
    _ant_sets._region = region_;
    _ant_sets._start_freq = start_freq_;
    _ant_sets._end_freq = end_freq_;
    _is_error = false;
}


void CommandsHandler::onSetFrequencyRegion() {
    _is_error = false;
}


void CommandsHandler::onInventory(uint8_t ant_id_, uint16_t tag_count_, uint16_t read_rate_, uint32_t total_read_) {
    _is_error = false;
}


void CommandsHandler::onRead(const Command::ReadCmdData &read_data_) {
    _is_error = false;
    if (_on_read_data_func) {
        _on_read_data_func(read_data_);
    } else {
        _cur_read_data = read_data_;
    }
}


void CommandsHandler::onGetInventoryBuffer(const Command::ReadCmdData &read_data_) {
    _is_error = false;
    if (_on_read_data_func) {
        _on_read_data_func(read_data_);
    } else {
        _cur_read_data = read_data_;
    }
}


void CommandsHandler::onGetAndResetInventoryBuffer(const Command::ReadCmdData &read_data_) {
    _is_error = false;
    if (_on_read_data_func) {
        _on_read_data_func(read_data_);
    } else {
        _cur_read_data = read_data_;
    }
}


void CommandsHandler::onGetInventoryBufferTagCount(uint16_t TagCount_) {
    _is_error = false;
    _cur_tag_count = TagCount_;
}


void CommandsHandler::onResetInventoryBuffer() {
    _is_error = false;
}
