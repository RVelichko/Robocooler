/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Набор обработчиков команд RFID.
 * \author Величко Ростислав
 * \date   07.17.2017
 */

#pragma once

#include <vector>
#include <map>
#include <atomic>
#include <functional>

#include "Message.hpp"
#include "Commands.hpp"
#include "TtyIo.hpp"

namespace robocooler {
namespace rfid {

typedef std::shared_ptr<Command> PCommand;
typedef Command::ECommandId Cid;
typedef Message::Buffer Buffer;

class CommandsHandler {
public:
    typedef std::function<void(const Command::ReadCmdData&)> OnReadDataFunc;

private:
    utils::TtyIo *_tty_io;
    std::vector<uint8_t> _recv_buf;
    PCommand _command;
    Command::ReadCmdData _cur_read_data;
    uint16_t _cur_tag_count;
    std::atomic_bool _is_error;
    Command::AntSettings _ant_sets;

    OnReadDataFunc _on_read_data_func;
    
    Cid onMessage(const Message &msg_);

public:
    static std::string toString(const Buffer &recv_buf_);
    static std::string toString(uint8_t b_);
    static std::string freqCodeToString(uint8_t code_);
    static std::string specRegionToString(Command::ESpektrumRegion region_);

    explicit CommandsHandler(utils::TtyIo *tty_io_);
    virtual ~CommandsHandler();

    void initOnReadDataFunc(const OnReadDataFunc &on_read_data_func_);

    bool isError();
    const Command::ReadCmdData& getCurReadData();
    uint16_t getCurTagCount();
    Command::AntSettings getAntSettings();

    Command* getCommand();
    bool sendMessage(const Message &msg_);
    Cid receivePacket(uint8_t b_);
    
    void onError(const std::string &err_);
    void onSetUartBaudrate();
    void onGetFirmwareVersion(uint8_t major, uint8_t minor);
    void onSetWorkAntenna();
    void onGetWorkAntenna(Command::EWorkAntenna work_ant_);
    void onSetOutputPower();
    void onGetOutputPower(uint8_t ant_pow_1_, uint8_t ant_pow_2_, uint8_t ant_pow_3_, uint8_t ant_pow_4_);
    void onGetFrequencyRegion(Command::ESpektrumRegion region_, uint8_t start_freq_, uint8_t end_freq_);
    void onSetFrequencyRegion();
    void onInventory(uint8_t ant_id_, uint16_t tag_count_, uint16_t read_rate_, uint32_t total_read_);
    void onRead(const Command::ReadCmdData &read_data_);
    void onGetInventoryBuffer(const Command::ReadCmdData &read_data_);
    void onGetAndResetInventoryBuffer(const Command::ReadCmdData &read_data_);
    void onGetInventoryBufferTagCount(uint16_t TagCount_);
    void onResetInventoryBuffer();
};
} /// robocooler
} /// rfid
