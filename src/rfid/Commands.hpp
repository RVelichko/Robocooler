/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Набор обработчиков команд RFID.
 * \author Величко Ростислав
 * \date   07.17.2017
 */

#include <cstdint>
#include <memory>
#include <map>
#include <functional>

#include "Message.hpp"

#pragma once

#define MAX_FREQUENCY_CODE 0x3b ///< Максимальный код для определения частотного региона.

namespace robocooler {
namespace rfid {
    
class CommandsHandler;    

typedef Message::Buffer Buffer;


class Command {
public:
    enum class ECommandId : uint8_t {
        /// Reader control commands.
        cmd_none = 0x0, ///< Команда в пакете отсутствует.
        cmd_reset = 0x70, ///< Reset reader.
        cmd_set_uart_baudrate = 0x71, ///< Set baud rate of serial port.
        cmd_get_firmware_version = 0x72, ///< Get firmware version. 
        cmd_set_reader_address = 0x73, ///< Set reader’s address.
        cmd_set_work_antenna = 0x74, ///< Set working antenna.
        cmd_get_work_antenna = 0x75, ///< Query current working antenna.
        cmd_set_output_power = 0x76, ///< Set RF output power.
        cmd_get_output_power = 0x77, ///< Query current RF output power.
        cmd_set_frequency_region = 0x78, ///< Set RF frequency spectrum.
        cmd_get_frequency_region = 0x79, ///< Query RF frequency spectrum.
        cmd_set_beeper_mode = 0x7A, ///< Set reader’s buzzer hehavior.
        cmd_get_reader_temperature = 0x7B, ///< Check reader’s internal temperature.
        cmd_read_gpio_value = 0x60, ///< Get GPIO1, GPIO2 status.
        cmd_write_gpio_value = 0x61, ///< Set GPIO3, GPIO4 status.
        cmd_set_ant_connection_detector = 0x62, ///< Set antenna detector status.
        cmd_get_ant_connection_detector = 0x63, ///< Get antenna detector status.
        cmd_set_temporary_output_power = 0x66, ///< Set RF power without saving to flash.
        cmd_set_reader_identifier = 0x67, ///< Set reader’s identification bytes.
        cmd_get_reader_identifier = 0x68, ///< Get reader’s identification bytes.
        cmd_set_rf_link_profile = 0x69, ///< Set RF link profile.
        cmd_get_rf_link_profile = 0x6A, ///< Get RF link profile.
        cmd_get_rf_port_return_loss = 0x7E, ///< Get current antenna port’s return loss.

        /// 18000-6C Commands.
        cmd_inventory = 0x80, ///< Inventory EPC C1G2 tags to buffer.
        cmd_read = 0x81, ///< Read EPC C1G2 tag(s).
        cmd_write = 0x82, ///< Write EPC C1G2 tag(s).
        cmd_lock = 0x83, ///< Lock EPC C1G2 tag(s).
        cmd_kill = 0x84, ///< Kill EPC C1G2 tag(s).
        cmd_set_access_epc_match = 0x85, ///< Set tag access filter by EPC.
        cmd_get_access_epc_match = 0x86, ///< Query access filter by EPC.
        cmd_real_time_inventory = 0x89, ///< Inventory tags in real time mode.
        cmd_fast_switch_ant_inventory = 0x8A, ///< Real time inventory with fast ant switch.
        cmd_customized_session_target_inventory = 0x8B, ///< Inventory with desired session and inventoried flag.
        cmd_set_impinj_fast_tid = 0x8C, ///< Set impinj FastTID function. (Without saving to FLASH)
        cmd_set_and_save_impinj_fast_tid = 0x8D, ///< Set impinj FastTID function. (Save to FLASH)
        cmd_get_impinj_fast_tid = 0x8E, ///< Get current FastTID setting.
        
        /// ISO 18000-6B Commands.
        cmd_iso18000_6b_inventory = 0xB0, ///< Inventory 18000-6B tag(s).
        cmd_iso18000_6b_read = 0xB1, ///< Read 18000-6B tag.
        cmd_iso18000_6b_write = 0xB2, ///< Write 18000-6B tag.
        cmd_iso18000_6b_lock = 0xB3, ///< Lock 18000-6B tag data byte.
        cmd_iso18000_6b_query_lock = 0xB4, ///< Query lock 18000-6B tag data byte.

        /// Buffer control commands.
        cmd_get_inventory_buffer = 0x90, ///< Get buffered data without clearing.
        cmd_get_and_reset_inventory_buffer = 0x91, ///< Get and clear buffered data.
        cmd_get_inventory_buffer_tag_count = 0x92, ///< Query how many tags are buffered.
        cmd_reset_inventory_buffer = 0x93, ///< Clear buffer.

        all_tags_receaved = 0x94 ///< Reading complete condition.
    };

    enum class EErrorCode : uint8_t {
        command_success = 0x10, ///< Command succeeded.
        command_fail = 0x11, ///< Command failed.
        mcu_reset_error = 0x20, ///< CPU reset error.
        cw_on_error = 0x21, ///< Turn on CW error.
        antenna_missing_error = 0x22, ///< Antenna is missing.
        write_flash_error = 0x23, ///< Write flash error.
        read_flash_error = 0x24, ///< Read flash error.
        set_output_power_error = 0x25, ///< Set output power error.
        tag_inventory_error = 0x31, ///< Error occurred when inventory.
        tag_read_error = 0x32, ///< Error occurred when read.
        tag_write_error = 0x33, ///< Error occurred when write.
        tag_lock_error = 0x34, ///< Error occurred when lock.
        tag_kill_error = 0x35, ///< Error occurred when kill.
        no_tag_error = 0x36, ///< There is no tag to be operated.
        inventory_ok_but_access_fail = 0x37, ///< Tag Inventoried but access failed.
        buffer_is_empty_error = 0x38, ///< Buffer is empty.
        access_or_password_error = 0x40, ///< Access failed or wrong password. 
        parameter_invalid = 0x41, ///< Invalid parameter.
        parameter_invalid_wordCnt_too_long = 0x42, ///< WordCnt is too long.
        parameter_invalid_membank_out_of_range = 0x43, ///< MemBank out of range.
        parameter_invalid_lock_region_out_of_range = 0x44, ///< Lock region out of range. 
        parameter_invalid_lock_action_out_of_range = 0x45, ///< LockType out of range.
        parameter_reader_address_invalid = 0x46, ///< Invalid reader address.
        parameter_invalid_AntennaID_out_of_range = 0x47, ///< AntennaID out of range.
        parameter_invalid_output_power_out_of_range = 0x48, ///< Output power out of range.
        parameter_invalid_frequency_region_out_of_range = 0x49, ///< Frequency region out of range.
        parameter_invalid_baudrate_out_of_range = 0x4A, ///< Baud rate out of range.
        parameter_beeper_mode_out_of_range = 0x4B, ///< Buzzer behavior out of range.
        parameter_epc_match_len_too_long = 0x4C, ///< EPC match is too long.
        parameter_epc_match_len_error = 0x4D, ///< EPC match length wrong.
        parameter_invalid_epc_match_mode = 0x4E, ///< Invalid EPC match mode.
        parameter_invalid_frequency_range = 0x4F, ///< Invalid frequency range.
        fail_to_get_RN16_from_tag = 0x50, ///< Failed to receive RN16 from tag.
        parameter_invalid_drm_mode = 0x51, ///< Invalid DRM mode.
        pll_lock_fail = 0x52, ///< PLL can not lock.
        rf_chip_fail_to_response = 0x53, ///< No response from RF chip.
        fail_to_achieve_desired_output_power = 0x54, ///< Can’t achieve desired output power level.
        copyright_authentication_fail = 0x55, ///< Can’t authenticate firmware copyright.
        spectrum_regulation_error = 0x56, ///< Spectrum regulation wrong.
        output_power_too_low = 0x57 ///< Output power is too low.
    };
    
    enum class EBaudrate : uint8_t {
        bps_38400 = 0x03, ///< 38400 bps.
        bps_115200 = 0x04 ///< 115200 bps.
    };

    enum class EReadMemBank : uint8_t {
        RESERVED = 0x00,
        EPC = 0x01,
        TID = 0x02,
        USER = 0x03
    };

    enum class EWorkAntenna : uint8_t {
        Ant1 = 0x00,
        Ant2 = 0x01,
        Ant3 = 0x02,
        Ant4 = 0x03,
        QUANTITY = 4
    };

    enum class ESpektrumRegion : uint8_t {
        FCC = 0x01,
        ETSI = 0x02,
        CHN = 0x03,
        QUANTITY = 4
    };

    struct ReadCmdData {
        uint16_t _TagCount;
        size_t _DataLen;
        uint16_t _PC;
        Buffer _EPC;
        uint16_t _CRC;
        Buffer _Data;
        size_t _ReadLen;
        uint8_t _freq_param;
        uint8_t _AntId;
        size_t _ReadCount;
        uint8_t _RSSI;
        uint8_t _InvCount;
        uint32_t _readed_num;
    };

    struct AntSettings {
        Command::ESpektrumRegion _region;
        uint8_t _start_freq;
        uint8_t _end_freq;
        uint8_t _ant_pow_1;
        uint8_t _ant_pow_2;
        uint8_t _ant_pow_3;
        uint8_t _ant_pow_4;
    };

    typedef Command::EErrorCode Ec;
    typedef Command::ECommandId Cid;
    typedef std::map<Ec, std::string> MapErrorCodes;
    typedef MapErrorCodes::iterator ErrorCodeIter;

private:
    uint8_t _rfid_addr;
    MapErrorCodes _err_codes;
    CommandsHandler *_hdl;

    void initErrorCodes();

    Command::ReadCmdData parseInventoryBufferData(const Buffer &data);
    
public:
    static std::string cmdToString(Cid cmd_code_);

    explicit Command(CommandsHandler *hdl_);
    virtual ~Command();
    
    Cid receive(const Message &msg_);
    std::string getError(Ec err_code_);
    
    void setRfidAddres(uint8_t addr_);
    
    /// Методы команд.
    void reset();
    void onReset(const Message &msg_);
    
    void setUartBaudrate(EBaudrate bdr_);
    void onSetUartBaudrate(const Message &msg_);
    
    void getFirmwareVersion();
    void onGetFirmwareVersion(const Message &msg_);
    
    void setWorkAntenna(EWorkAntenna work_ant_);
    void onSetWorkAntenna(const Message &msg_);
    
    void getWorkAntenna();
    void onGetWorkAntenna(const Message &msg_);

    void setOutputPower(uint8_t ant_power_1_, uint8_t ant_power_2_, uint8_t ant_power_3_, uint8_t ant_power_4_);
    void onSetOutputPower(const Message &msg_);
    
    void getOutputPower();
    void onGetOutputPower(const Message &msg_);

    void setFrequencyRegion(ESpektrumRegion region_, uint8_t start_freq_code_, uint8_t end_freq_code_);
    void onSetFrequencyRegion(const Message &msg_);
    
    void getFrequencyRegion();
    void onGetFrequencyRegion(const Message &msg_);

    void inventory(uint8_t repeat_);
    void onInventory(const Message &msg_);
                   
    void read(EReadMemBank mem_banck_, uint8_t word_add_, uint8_t word_cnt_ = 1);
    void onRead(const Message &msg_);

    void getInventoryBuffer();
    void onGetInventoryBuffer(const Message &msg_);

    void getAndResetInventoryBuffer();
    void onGetAndResetInventoryBuffer(const Message &msg_);

    void getInventoryBufferTagCount();
    void onGetInventoryBufferTagCount(const Message &msg_);

    void resetInventoryBuffer();
    void onResetInventoryBuffer(const Message &msg_);
};
} /// robocooler
} /// rfid
