#include "Log.hpp"
#include "Message.hpp"
#include "CommandsHandler.hpp"
#include "Commands.hpp"


using namespace robocooler;
using namespace rfid;

namespace ph = std::placeholders;

typedef Command::ECommandId Cid;
typedef Command::EErrorCode Ec;
typedef Message::Buffer Buffer;
typedef CommandsHandler CmdHdl;


std::string Command::cmdToString(Cid cmd_code_) {
    switch (cmd_code_) {
        /// Reader control commands.
        case Cid::cmd_reset: return "cmd_reset"; ///< Reset reader.
        case Cid::cmd_set_uart_baudrate: return "cmd_set_uart_baudrate"; ///< Set baud rate of serial port.
        case Cid::cmd_get_firmware_version: return "cmd_get_firmware_version"; ///< Get firmware version. 
        case Cid::cmd_set_reader_address: return "cmd_set_reader_address"; ///< Set reader’s address.
        case Cid::cmd_set_work_antenna: return "cmd_set_work_antenna"; ///< Set working antenna.
        case Cid::cmd_get_work_antenna: return "cmd_get_work_antenna"; ///< Query current working antenna.
        case Cid::cmd_set_output_power: return "cmd_set_output_power"; ///< Set RF output power.
        case Cid::cmd_get_output_power: return "cmd_get_output_power"; ///< Query current RF output power.
        case Cid::cmd_set_frequency_region: return "cmd_set_frequency_region"; ///< Set RF frequency spectrum.
        case Cid::cmd_get_frequency_region: return "cmd_get_frequency_region"; ///< Query RF frequency spectrum.
        case Cid::cmd_set_beeper_mode: return "cmd_set_beeper_mode"; ///< Set reader’s buzzer hehavior.
        case Cid::cmd_get_reader_temperature: return "cmd_get_reader_temperature"; ///< Check reader’s internal temperature.
        case Cid::cmd_read_gpio_value: return "cmd_read_gpio_value"; ///< Get GPIO1, GPIO2 status.
        case Cid::cmd_write_gpio_value: return "cmd_write_gpio_value"; ///< Set GPIO3, GPIO4 status.
        case Cid::cmd_set_ant_connection_detector: return "cmd_set_ant_connection_detector"; ///< Set antenna detector status.
        case Cid::cmd_get_ant_connection_detector: return "cmd_get_ant_connection_detector"; ///< Get antenna detector status.
        case Cid::cmd_set_temporary_output_power: return "cmd_set_temporary_output_power"; ///< Set RF power without saving to flash.
        case Cid::cmd_set_reader_identifier: return "cmd_set_reader_identifier"; ///< Set reader’s identification bytes.
        case Cid::cmd_get_reader_identifier: return "cmd_get_reader_identifier"; ///< Get reader’s identification bytes.
        case Cid::cmd_set_rf_link_profile: return "cmd_set_rf_link_profile"; ///< Set RF link profile.
        case Cid::cmd_get_rf_link_profile: return "cmd_get_rf_link_profile"; ///< Get RF link profile.
        case Cid::cmd_get_rf_port_return_loss: return "cmd_get_rf_port_return_loss"; ///< Get current antenna port’s return loss.

        /// 18000-6C Commands.
        case Cid::cmd_inventory: return "cmd_inventory"; ///< Inventory EPC C1G2 tags to buffer.
        case Cid::cmd_read: return "cmd_read"; ///< Read EPC C1G2 tag(s).
        case Cid::cmd_write: return "cmd_write"; ///< Write EPC C1G2 tag(s).
        case Cid::cmd_lock: return "cmd_lock"; ///< Lock EPC C1G2 tag(s).
        case Cid::cmd_kill: return "cmd_kill"; ///< Kill EPC C1G2 tag(s).
        case Cid::cmd_set_access_epc_match: return "cmd_set_access_epc_match"; ///< Set tag access filter by EPC.
        case Cid::cmd_get_access_epc_match: return "cmd_get_access_epc_match"; ///< Query access filter by EPC.
        case Cid::cmd_real_time_inventory: return "cmd_real_time_inventory"; ///< Inventory tags in real time mode.
        case Cid::cmd_fast_switch_ant_inventory: return "cmd_fast_switch_ant_inventory"; ///< Real time inventory with fast ant switch.
        case Cid::cmd_customized_session_target_inventory: return "cmd_customized_session_target_inventory"; ///< Inventory with desired session and inventoried flag.
        case Cid::cmd_set_impinj_fast_tid: return "cmd_set_impinj_fast_tid"; ///< Set impinj FastTID function. (Without saving to FLASH)
        case Cid::cmd_set_and_save_impinj_fast_tid: return "cmd_set_and_save_impinj_fast_tid"; ///< Set impinj FastTID function. (Save to FLASH)
        case Cid::cmd_get_impinj_fast_tid: return "cmd_get_impinj_fast_tid"; ///< Get current FastTID setting.

        /// ISO 18000-6B Commands.
        case Cid::cmd_iso18000_6b_inventory: return "cmd_iso18000_6b_inventory"; ///< Inventory 18000-6B tag(s).
        case Cid::cmd_iso18000_6b_read: return "cmd_iso18000_6b_read"; ///< Read 18000-6B tag.
        case Cid::cmd_iso18000_6b_write: return "cmd_iso18000_6b_write"; ///< Write 18000-6B tag.
        case Cid::cmd_iso18000_6b_lock: return "cmd_iso18000_6b_lock"; ///< Lock 18000-6B tag data byte.
        case Cid::cmd_iso18000_6b_query_lock: return "cmd_iso18000_6b_query_lock"; ///< Query lock 18000-6B tag data byte.

        /// Buffer control commands.
        case Cid::cmd_get_inventory_buffer: return "cmd_get_inventory_buffer"; ///< Get buffered data without clearing.
        case Cid::cmd_get_and_reset_inventory_buffer: return "cmd_get_and_reset_inventory_buffer"; ///< Get and clear buffered data.
        case Cid::cmd_get_inventory_buffer_tag_count: return "cmd_get_inventory_buffer_tag_count"; ///< Query how many tags are buffered.
        case Cid::cmd_reset_inventory_buffer: return "cmd_reset_inventory_buffer"; ///< Clear buffer.

        case Cid::all_tags_receaved: return "all_tags_receaved"; ///< Reading complete condition.
        
        default:
            return "cmd_none"; ///< Команда в пакете отсутствует.
    }
    return "cmd_none"; ///< Команда в пакете отсутствует.
}


void Command::initErrorCodes() {
    _err_codes.insert(std::make_pair(Ec::command_success, "Command succeeded."));
    _err_codes.insert(std::make_pair(Ec::command_fail, "Command failed."));
    _err_codes.insert(std::make_pair(Ec::mcu_reset_error, "CPU reset error."));
    _err_codes.insert(std::make_pair(Ec::cw_on_error, "Turn on CW error."));
    _err_codes.insert(std::make_pair(Ec::antenna_missing_error, "Antenna is missing."));
    _err_codes.insert(std::make_pair(Ec::write_flash_error, "Write flash error."));
    _err_codes.insert(std::make_pair(Ec::read_flash_error, "Read flash error."));
    _err_codes.insert(std::make_pair(Ec::set_output_power_error, "Set output power error."));
    _err_codes.insert(std::make_pair(Ec::tag_inventory_error, "Error occurred when inventory."));
    _err_codes.insert(std::make_pair(Ec::tag_read_error, "Error occurred when read."));
    _err_codes.insert(std::make_pair(Ec::tag_write_error, "Error occurred when write."));
    _err_codes.insert(std::make_pair(Ec::tag_lock_error, "Error occurred when lock."));
    _err_codes.insert(std::make_pair(Ec::tag_kill_error, "Error occurred when kill."));
    _err_codes.insert(std::make_pair(Ec::no_tag_error, "There is no tag to be operated."));
    _err_codes.insert(std::make_pair(Ec::inventory_ok_but_access_fail, "Tag Inventoried but access failed."));
    _err_codes.insert(std::make_pair(Ec::buffer_is_empty_error, "Buffer is empty."));
    _err_codes.insert(std::make_pair(Ec::access_or_password_error, "Access failed or wrong password."));
    _err_codes.insert(std::make_pair(Ec::parameter_invalid, "Invalid parameter."));
    _err_codes.insert(std::make_pair(Ec::parameter_invalid_wordCnt_too_long, "WordCnt is too long."));
    _err_codes.insert(std::make_pair(Ec::parameter_invalid_membank_out_of_range, "MemBank out of range."));
    _err_codes.insert(std::make_pair(Ec::parameter_invalid_lock_region_out_of_range, "Lock region out of range.")); 
    _err_codes.insert(std::make_pair(Ec::parameter_invalid_lock_action_out_of_range, "LockType out of range."));
    _err_codes.insert(std::make_pair(Ec::parameter_reader_address_invalid, "Invalid reader address."));
    _err_codes.insert(std::make_pair(Ec::parameter_invalid_AntennaID_out_of_range, "AntennaID out of range."));
    _err_codes.insert(std::make_pair(Ec::parameter_invalid_output_power_out_of_range, "Output power out of range."));
    _err_codes.insert(std::make_pair(Ec::parameter_invalid_frequency_region_out_of_range, "Frequency region out of range."));
    _err_codes.insert(std::make_pair(Ec::parameter_invalid_baudrate_out_of_range, "Baud rate out of range."));
    _err_codes.insert(std::make_pair(Ec::parameter_beeper_mode_out_of_range, "Buzzer behavior out of range."));
    _err_codes.insert(std::make_pair(Ec::parameter_epc_match_len_too_long, "EPC match is too long."));
    _err_codes.insert(std::make_pair(Ec::parameter_epc_match_len_error, "EPC match length wrong."));
    _err_codes.insert(std::make_pair(Ec::parameter_invalid_epc_match_mode, "Invalid EPC match mode."));
    _err_codes.insert(std::make_pair(Ec::parameter_invalid_frequency_range, "Invalid frequency range."));
    _err_codes.insert(std::make_pair(Ec::fail_to_get_RN16_from_tag, "Failed to receive RN16 from tag."));
    _err_codes.insert(std::make_pair(Ec::parameter_invalid_drm_mode, "Invalid DRM mode."));
    _err_codes.insert(std::make_pair(Ec::pll_lock_fail, "PLL can not lock."));
    _err_codes.insert(std::make_pair(Ec::rf_chip_fail_to_response, "No response from RF chip."));
    _err_codes.insert(std::make_pair(Ec::fail_to_achieve_desired_output_power, "Can’t achieve desired output power level."));
    _err_codes.insert(std::make_pair(Ec::copyright_authentication_fail, "Can’t authenticate firmware copyright."));
    _err_codes.insert(std::make_pair(Ec::spectrum_regulation_error, "Spectrum regulation wrong."));
    _err_codes.insert(std::make_pair(Ec::output_power_too_low, "Output power is too low."));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Command::Command(CommandsHandler *hdl_)
    : _hdl(hdl_) {
    initErrorCodes();
}


Command::~Command() 
{}


Cid Command::receive(const Message &msg_) {
    //LOG(DEBUG) << "msg: " << CmdHdl::toString(msg_.getAryTranData());
    Cid cid = static_cast<Cid>(msg_.getCmd());
    switch (cid) {
        case Cid::cmd_reset: onReset(msg_); break;
        case Cid::cmd_set_uart_baudrate: onSetUartBaudrate(msg_); break;
        case Cid::cmd_get_firmware_version: onGetFirmwareVersion(msg_); break;
        case Cid::cmd_set_work_antenna: onSetWorkAntenna(msg_); break;
        case Cid::cmd_get_work_antenna: onGetWorkAntenna(msg_); break;
        case Cid::cmd_set_output_power: onSetOutputPower(msg_); break;
        case Cid::cmd_get_output_power: onGetOutputPower(msg_); break;
        case Cid::cmd_set_frequency_region: onSetFrequencyRegion(msg_); break;
        case Cid::cmd_get_frequency_region: onGetFrequencyRegion(msg_); break;
        case Cid::cmd_inventory: onInventory(msg_); break;
        case Cid::cmd_read: onRead(msg_); break;
        case Cid::cmd_get_inventory_buffer: onGetInventoryBuffer(msg_); break;
        case Cid::cmd_get_and_reset_inventory_buffer: onGetAndResetInventoryBuffer(msg_); break;
        case Cid::cmd_get_inventory_buffer_tag_count: onGetInventoryBufferTagCount(msg_); break;
        case Cid::cmd_reset_inventory_buffer: onResetInventoryBuffer(msg_); break;
        default:
            LOG(WARNING) << "Undeclared recv command: " << CmdHdl::toString(msg_.getCmd());
            cid = Cid::cmd_none;
            break;
    }
    return cid;
}


std::string Command::getError(Ec err_code_) {
    ErrorCodeIter iter = _err_codes.find(err_code_);
    if (iter not_eq _err_codes.end()) {
        return iter->second;
    }
    return "Undeclared error code: " + CmdHdl::toString(static_cast<uint8_t>(err_code_));
}


void Command::setRfidAddres(uint8_t addr_) {
    _rfid_addr = addr_;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void Command::reset() {
    LOG(DEBUG) << "addr: " << CmdHdl::toString(_rfid_addr);
    Cid cid = Cid::cmd_reset;
    Message msg(_rfid_addr, static_cast<uint8_t>(cid));
    _hdl->sendMessage(msg);
}


void Command::onReset(const Message &msg_) {
    //LOG(DEBUG) << "msg: " << CmdHdl::toString(msg_.getAryTranData());
    Ec err_code = static_cast<Ec>(msg_.getErrorCode());
    LOG(ERROR) << getError(err_code);
    _hdl->onError(getError(err_code));
}


void Command::setUartBaudrate(EBaudrate bdr_) {
    LOG(DEBUG) << "addr: " << CmdHdl::toString(_rfid_addr) << "uart_baudrate: " << CmdHdl::toString(static_cast<uint8_t>(bdr_));
    Cid cid = Cid::cmd_set_uart_baudrate;
    Buffer data(1, static_cast<uint8_t>(bdr_));
    Message msg(_rfid_addr, static_cast<uint8_t>(cid), data);
    _hdl->sendMessage(msg);
}


void Command::onSetUartBaudrate(const Message &msg_) {
    Ec err_code = static_cast<Ec>(msg_.getErrorCode());
    if (err_code not_eq Ec::command_success) {
        LOG(ERROR) << getError(err_code);
        _hdl->onError(getError(err_code));
    } else {
        //LOG(TRACE) << getError(err_code);
        _hdl->onSetUartBaudrate();
    }
}


void Command::getFirmwareVersion() {
    LOG(DEBUG) << "addr: " << CmdHdl::toString(_rfid_addr);
    Cid cid = Cid::cmd_get_firmware_version;
    Message msg(_rfid_addr, static_cast<uint8_t>(cid));
    _hdl->sendMessage(msg);
}


void Command::onGetFirmwareVersion(const Message &msg_) {
    if (msg_.getDataLen() not_eq 5) { ///< Размер данных, передаваемый в поле пакета.
        LOG(ERROR) << getError(Ec::command_fail);
        _hdl->onError(getError(Ec::command_fail));
    } else {
        const Buffer &pack = msg_.getAryTranData();
        uint8_t major = pack[4];
        uint8_t minor = pack[5];
        //LOG(TRACE) << "Firmware vertion: " << static_cast<uint16_t>(major) << "." << static_cast<uint16_t>(minor);
        /// Передать полученные данные подписавшемуся объекту.
        _hdl->onGetFirmwareVersion(major, minor);
    }
}


void Command::setWorkAntenna(EWorkAntenna work_ant_) {
    LOG(DEBUG) << "addr: " << CmdHdl::toString(_rfid_addr) 
               << " Work ant: [" << CmdHdl::toString(static_cast<uint8_t>(work_ant_)) << "]";
    Cid cid = Cid::cmd_set_work_antenna;
    Buffer data(1, static_cast<uint8_t>(work_ant_));
    Message msg(_rfid_addr, static_cast<uint8_t>(cid), data);
    _hdl->sendMessage(msg);
}


void Command::onSetWorkAntenna(const Message &msg_) {
    LOG(DEBUG) << "msg: " << CmdHdl::toString(msg_.getAryTranData());
    Ec err_code = static_cast<Ec>(msg_.getErrorCode());
    if (err_code not_eq Ec::command_success) {
        LOG(ERROR) << getError(err_code);
        _hdl->onError(getError(err_code));
    } else {
        //LOG(TRACE) << getError(err_code);
        _hdl->onSetWorkAntenna();
    }
}


void Command::getWorkAntenna() {
    LOG(DEBUG) << std::hex << "addr: " << CmdHdl::toString(_rfid_addr);
    Cid cid = Cid::cmd_get_work_antenna;
    Message msg(_rfid_addr, static_cast<uint8_t>(cid));
    _hdl->sendMessage(msg);
}


void Command::onGetWorkAntenna(const Message &msg_) {
    if (msg_.getDataLen() not_eq 4) { ///< Размер данных, передаваемый в поле пакета.
        LOG(ERROR) << getError(Ec::command_fail);
        _hdl->onError(getError(Ec::command_fail));
    } else {
        const Buffer &pack = msg_.getAryTranData();
        uint8_t ant_id = pack[4];
        //LOG(TRACE) << "Antenna ID: " << CmdHdl::toString(ant_id + 1);
        /// Передать полученные данные подписавшемуся объекту.
        _hdl->onGetWorkAntenna(static_cast<EWorkAntenna>(ant_id));
    }
}


void Command::setOutputPower(uint8_t ant_power_1_, uint8_t ant_power_2_, uint8_t ant_power_3_, uint8_t ant_power_4_) {
    LOG(DEBUG) << std::hex << "addr: " << CmdHdl::toString(_rfid_addr);
    Cid cid = Cid::cmd_set_output_power;
    Buffer data({  
        ((ant_power_1_ <= static_cast<uint8_t>(0x21)) ? ant_power_1_ : static_cast<uint8_t>(0x00)), 
        ((ant_power_2_ <= static_cast<uint8_t>(0x21)) ? ant_power_2_ : static_cast<uint8_t>(0x00)), 
        ((ant_power_3_ <= static_cast<uint8_t>(0x21)) ? ant_power_3_ : static_cast<uint8_t>(0x00)), 
        ((ant_power_4_ <= static_cast<uint8_t>(0x21)) ? ant_power_4_ : static_cast<uint8_t>(0x00))
    });
    Message msg(_rfid_addr, static_cast<uint8_t>(cid), data);
    _hdl->sendMessage(msg);
}


void Command::onSetOutputPower(const Message &msg_) {
    LOG(DEBUG) << "msg: " << CmdHdl::toString(msg_.getAryTranData());
    Ec err_code = static_cast<Ec>(msg_.getErrorCode());
    if (err_code not_eq Ec::command_success) {
        LOG(ERROR) << getError(err_code);
        _hdl->onError(getError(err_code));
    } else {
        //LOG(TRACE) << getError(err_code);
        _hdl->onSetOutputPower();
    }

}


void Command::getOutputPower() {
    LOG(DEBUG) << std::hex << "addr: " << CmdHdl::toString(_rfid_addr);
    Cid cid = Cid::cmd_get_output_power;
    Message msg(_rfid_addr, static_cast<uint8_t>(cid));
    _hdl->sendMessage(msg);
}


void Command::onGetOutputPower(const Message &msg_) {
    if (msg_.getDataLen() not_eq 4) { ///< Размер данных, передаваемый в поле пакета.
        LOG(ERROR) << getError(Ec::command_fail);
        _hdl->onError(getError(Ec::command_fail));
    } else {
        uint8_t ant_power_1 = 0;
        uint8_t ant_power_2 = 0;
        uint8_t ant_power_3 = 0;
        uint8_t ant_power_4 = 0;
        const Buffer &data = msg_.getAryData();
        if (msg_.getDataLen() == 4) {
            ant_power_1 = data[0];
            ant_power_2 = data[0];
            ant_power_3 = data[0];
            ant_power_4 = data[0];
        } else if (msg_.getDataLen() == 7) {
            ant_power_1 = data[0];
            ant_power_2 = data[1];
            ant_power_3 = data[2];
            ant_power_4 = data[3];
        }
        //LOG(TRACE) << "Output power: " << CmdHdl::toString(ant_power_1) 
        //           << "; " << CmdHdl::toString(ant_power_2) 
        //           << "; " << CmdHdl::toString(ant_power_3) 
        //           << "; " << CmdHdl::toString(ant_power_4);
        /// Передать полученные данные подписавшемуся объекту.
        _hdl->onGetOutputPower(ant_power_1, ant_power_2, ant_power_3, ant_power_4);
    }
}


void Command::setFrequencyRegion(ESpektrumRegion region_, uint8_t start_freq_code_, uint8_t end_freq_code_) {
    LOG(DEBUG) << std::hex << "addr: " << CmdHdl::toString(_rfid_addr) 
               << " Region: " << CmdHdl::toString(static_cast<uint8_t>(region_)) 
               << " StartFreq: " << CmdHdl::toString(start_freq_code_) 
               << " EndFreq: " << CmdHdl::toString(end_freq_code_);
    Cid cid = Cid::cmd_set_frequency_region;
    if (end_freq_code_ < start_freq_code_ or 
        MAX_FREQUENCY_CODE < start_freq_code_ or 
        MAX_FREQUENCY_CODE < end_freq_code_) {
        start_freq_code_ = 0x00; 
        end_freq_code_  = 0x06;
    }
    if (ESpektrumRegion::QUANTITY < region_) {
        region_ = ESpektrumRegion::ETSI;
    }
    Buffer data({ 
        static_cast<uint8_t>(region_), 
        start_freq_code_, 
        end_freq_code_ 
    });
    Message msg(_rfid_addr, static_cast<uint8_t>(cid), data);
    _hdl->sendMessage(msg);
}


void Command::onSetFrequencyRegion(const Message &msg_) {
    LOG(DEBUG) << "msg: " << CmdHdl::toString(msg_.getAryTranData());
    Ec err_code = static_cast<Ec>(msg_.getErrorCode());
    if (err_code not_eq Ec::command_success) {
        LOG(ERROR) << getError(err_code);
        _hdl->onError(getError(err_code));
    } else {
        //LOG(TRACE) << getError(err_code);
        _hdl->onSetFrequencyRegion();
    }
}


void Command::getFrequencyRegion() {
    LOG(DEBUG) << std::hex << "addr: " << CmdHdl::toString(_rfid_addr);
    Cid cid = Cid::cmd_get_frequency_region;
    Message msg(_rfid_addr, static_cast<uint8_t>(cid));
    _hdl->sendMessage(msg);
}


void Command::onGetFrequencyRegion(const Message &msg_) {
    if (msg_.getDataLen() not_eq 6) { ///< Размер данных, передаваемый в поле пакета.
        LOG(ERROR) << getError(Ec::command_fail);
        _hdl->onError(getError(Ec::command_fail));
    } else {
        const Buffer &pack = msg_.getAryTranData();
        uint8_t Region = pack[4];
        uint8_t StartFreq = pack[5];
        uint8_t EndFreq = pack[6];
        //LOG(TRACE) << " Region: " << CmdHdl::toString(Region) 
        //           << " StartFreq: " << CmdHdl::toString(StartFreq) 
        //           << " EndFreq: " << CmdHdl::toString(EndFreq);
        /// Передать полученные данные подписавшемуся объекту.
        _hdl->onGetFrequencyRegion(static_cast<ESpektrumRegion>(Region), StartFreq, EndFreq);
    }
}


void Command::inventory(uint8_t repeat_) {
    LOG(DEBUG) << std::hex << "addr: " << CmdHdl::toString(_rfid_addr) << " repeat: " << CmdHdl::toString(repeat_);
    Cid cid = Cid::cmd_inventory;
    Buffer data(1, repeat_);
    Message msg(_rfid_addr, static_cast<uint8_t>(cid), data);
    _hdl->sendMessage(msg);
}


void Command::onInventory(const Message &msg_) {
    if (msg_.getDataLen() == 4) { ///< Размер данных, передаваемый в поле пакета.
        Ec err_code = static_cast<Ec>(msg_.getErrorCode());
        LOG(ERROR) << getError(err_code);
        _hdl->onError(getError(err_code));
    } else {
        const Buffer &data = msg_.getAryData();
        uint8_t ant_id = data[0];
        
        /// Байты приходят в перевёрнутом виде.
        uint16_t tag_count = 0;
        (reinterpret_cast<uint8_t*>(&(tag_count)))[1] = data[1];
        (reinterpret_cast<uint8_t*>(&(tag_count)))[0] = data[2];
        
        /// Байты приходят в перевёрнутом виде.
        uint16_t read_rate = 0;
        (reinterpret_cast<uint8_t*>(&(read_rate)))[1] = data[3];
        (reinterpret_cast<uint8_t*>(&(read_rate)))[0] = data[4];
        
        /// Байты приходят в перевёрнутом виде.
        uint32_t total_read = 0;
        (reinterpret_cast<uint8_t*>(&(total_read)))[3] = data[5];
        (reinterpret_cast<uint8_t*>(&(total_read)))[2] = data[6];
        (reinterpret_cast<uint8_t*>(&(total_read)))[1] = data[7];
        (reinterpret_cast<uint8_t*>(&(total_read)))[0] = data[8];
        
        //LOG(TRACE) 
        //    << " AntId: " << static_cast<uint16_t>(ant_id) 
        //    << " TagCount: " << tag_count 
        //    << " ReadRate: " << read_rate 
        //    << " TotalRead: " << total_read;
        /// Передать полученные данные подписавшемуся объекту.
        _hdl->onInventory(ant_id, tag_count, read_rate, total_read);
    }
}


void Command::read(EReadMemBank mem_bank_, uint8_t word_add_, uint8_t word_cnt_) {
    LOG(DEBUG) << "addr: " << CmdHdl::toString(_rfid_addr);
    Cid cid = Cid::cmd_read;
    Buffer buf(3, 0x0);
    buf[0] = static_cast<uint8_t>(mem_bank_);
    buf[1] = word_add_;
    buf[2] = word_cnt_;
    Message msg(_rfid_addr, static_cast<uint8_t>(cid), buf);
    _hdl->sendMessage(msg);
}


void Command::onRead(const Message &msg_) {
    if (msg_.getDataLen() == 4) { ///< Размер данных, передаваемый в поле пакета.
        Ec err_code = static_cast<Ec>(msg_.getErrorCode());
        LOG(ERROR) << getError(err_code);
        _hdl->onError(getError(err_code));
    } else {
        const Buffer &data = msg_.getAryData();
        ReadCmdData rcd = {0};
        size_t len = msg_.getDataLen() - 3;

        /// Байты приходят в перевёрнутом виде.
        (reinterpret_cast<uint8_t*>(&(rcd._TagCount)))[1] = data[0];
        (reinterpret_cast<uint8_t*>(&(rcd._TagCount)))[0] = data[1];

        rcd._DataLen    = data[2];
        rcd._ReadLen    = data[len - 3];
        rcd._freq_param = data[len - 2] & 0xfc;
        rcd._AntId      = data[len - 2] & 0x03;
        rcd._ReadCount  = data[len - 1];
        size_t epc_len = static_cast<size_t>(rcd._DataLen) - rcd._ReadLen - 4;

        /// Байты приходят в перевёрнутом виде.
        (reinterpret_cast<uint8_t*>(&(rcd._PC)))[1] = data[3];
        (reinterpret_cast<uint8_t*>(&(rcd._PC)))[0] = data[4];

        if (epc_len) {
            rcd._EPC.resize(epc_len, 0x0);
            for (size_t e = 0; e < epc_len; ++e) {
                rcd._EPC[e] = data[5 + e];
            };
        }

        /// Байты приходят в перевёрнутом виде.
        (reinterpret_cast<uint8_t*>(&(rcd._CRC)))[1]  = data[epc_len + 5];
        (reinterpret_cast<uint8_t*>(&(rcd._CRC)))[0]  = data[epc_len + 6];

        if (rcd._ReadLen) {
            rcd._Data.resize(rcd._ReadLen, 0x0);
            for(size_t d = 0; d < rcd._ReadLen; ++d) {
                rcd._Data[d] = data[epc_len + 7 + d];
            }
        }
        //LOG(TRACE) << " TagCount: " << rcd._TagCount
        //           << " DataLen: " << rcd._DataLen
        //           << " PC: " << rcd._PC
        //           << " EPC: " << CmdHdl::toString(rcd._EPC)
        //           << " CRC: " << rcd._CRC
        //           << " Data: " << CmdHdl::toString(rcd._Data)
        //           << " ReadLen: " << rcd._ReadLen
        //           << " AntId: [" << CmdHdl::toString(rcd._freq_param) << "|" << CmdHdl::toString(rcd._AntId) << "]"
        //           << " ReadCount: " << rcd._ReadCount;
        _hdl->onRead(rcd);
    }
}


void Command::getInventoryBuffer() {
    LOG(DEBUG) << "addr: " << CmdHdl::toString(_rfid_addr);
    Cid cid = Cid::cmd_get_inventory_buffer;
    Message msg(_rfid_addr, static_cast<uint8_t>(cid));
    _hdl->sendMessage(msg);
}


Command::ReadCmdData Command::parseInventoryBufferData(const Buffer &data_) {
    ReadCmdData rcd = {0};

    /// Байты приходят в перевёрнутом виде.
    (reinterpret_cast<uint8_t*>(&(rcd._TagCount)))[1] = data_[0];
    (reinterpret_cast<uint8_t*>(&(rcd._TagCount)))[0] = data_[1];

    rcd._DataLen = data_[2];
    size_t epc_len = static_cast<size_t>(rcd._DataLen) - 4;

    /// Байты приходят в перевёрнутом виде.
    (reinterpret_cast<uint8_t*>(&(rcd._PC)))[1] = data_[3];
    (reinterpret_cast<uint8_t*>(&(rcd._PC)))[0] = data_[4];

    if (epc_len) {
        rcd._EPC.resize(epc_len, 0x0);
        for (size_t e = 0; e < epc_len; ++e) {
            rcd._EPC[e] = data_[5 + e];
        };
    }

    /// Байты приходят в перевёрнутом виде.
    (reinterpret_cast<uint8_t*>(&(rcd._CRC)))[1]  = data_[epc_len + 5];
    (reinterpret_cast<uint8_t*>(&(rcd._CRC)))[0]  = data_[epc_len + 6];

    rcd._RSSI       = data_[epc_len + 7];
    rcd._freq_param = data_[epc_len + 8] & 0xfc;
    rcd._AntId      = data_[epc_len + 8] & 0x03;
    rcd._InvCount   = data_[epc_len + 9];

    //LOG(TRACE) << " TagCount: " << rcd._TagCount
    //           << " DataLen: " << rcd._DataLen
    //           << " PC: " << rcd._PC
    //           << " EPC: " << CmdHdl::toString(rcd._EPC)
    //           << " CRC: " << rcd._CRC
    //           << " RSSI: " << CmdHdl::toString(rcd._RSSI)
    //           << " FreqAnt: [" << CmdHdl::toString(rcd._freq_param) << "|" << CmdHdl::toString(rcd._AntId) << "]"
    //           << " InvCount: " << CmdHdl::toString(rcd._InvCount);
    return rcd;
}


void Command::onGetInventoryBuffer(const Message &msg_) {
    if (msg_.getDataLen() == 4) { ///< Размер данных, передаваемый в поле пакета.
        Ec err_code = static_cast<Ec>(msg_.getErrorCode());
        LOG(ERROR) << getError(err_code);
        _hdl->onError(getError(err_code));
    } else {
        _hdl->onGetInventoryBuffer(parseInventoryBufferData(msg_.getAryData()));
    }
}


void Command::getAndResetInventoryBuffer() {
    LOG(DEBUG) << "addr: " << CmdHdl::toString(_rfid_addr);
    Cid cid = Cid::cmd_get_and_reset_inventory_buffer;
    Message msg(_rfid_addr, static_cast<uint8_t>(cid));
    _hdl->sendMessage(msg);
}


void Command::onGetAndResetInventoryBuffer(const Message &msg_) {
    if (msg_.getDataLen() == 4) { ///< Размер данных, передаваемый в поле пакета.
        Ec err_code = static_cast<Ec>(msg_.getErrorCode());
        LOG(ERROR) << getError(err_code);
        _hdl->onError(getError(err_code));
    } else {
        _hdl->onGetAndResetInventoryBuffer(parseInventoryBufferData(msg_.getAryData()));
    }
}


void Command::getInventoryBufferTagCount() {
    LOG(DEBUG) << "addr: " << CmdHdl::toString(_rfid_addr);
    Cid cid = Cid::cmd_get_inventory_buffer_tag_count;
    Message msg(_rfid_addr, static_cast<uint8_t>(cid));
    _hdl->sendMessage(msg);
}


void Command::onGetInventoryBufferTagCount(const Message &msg_) {
    if (msg_.getDataLen() not_eq 5) { ///< Размер данных, передаваемый в поле пакета.
        Ec err_code = static_cast<Ec>(msg_.getErrorCode());
        LOG(ERROR) << getError(err_code);
        _hdl->onError(getError(err_code));
    } else {
        const Buffer &data = msg_.getAryData();
        uint16_t TagCount = 0;
        /// Байты приходят в перевёрнутом виде.
        (reinterpret_cast<uint8_t*>(&(TagCount)))[1] = data[0];
        (reinterpret_cast<uint8_t*>(&(TagCount)))[0] = data[1];
        //LOG(DEBUG) << "TagCount=" << TagCount;
        _hdl->onGetInventoryBufferTagCount(TagCount);
    }
}


void Command::resetInventoryBuffer() {
    LOG(DEBUG) << "addr: " << CmdHdl::toString(_rfid_addr);
    Cid cid = Cid::cmd_reset_inventory_buffer;
    Message msg(_rfid_addr, static_cast<uint8_t>(cid));
    _hdl->sendMessage(msg);
}


void Command::onResetInventoryBuffer(const Message &msg_) {
    LOG(DEBUG) << "msg: " << CmdHdl::toString(msg_.getAryTranData());
    Ec err_code = static_cast<Ec>(msg_.getErrorCode());
    if (err_code not_eq Ec::command_success) {
        LOG(ERROR) << getError(err_code);
        _hdl->onError(getError(err_code));
    } else {
        //LOG(TRACE) << getError(err_code);
        _hdl->onResetInventoryBuffer();
    }
}
