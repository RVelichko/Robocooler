/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Класс пакета RFID.
 * \author Величко Ростислав
 * \date   07.17.2017
 */

#pragma once

#include <cstdint>
#include <vector>


static const uint8_t RFID_HEAD = 0xa0;
static const uint8_t RFID_NULL = 0x00;
static const uint8_t RFID_ADDR = 0xff;
static const size_t RFID_DATA_POS = 4;
static const size_t RFID_PACK_MINLEN = 5;
static const size_t RFID_ERR_CODE_POS = RFID_DATA_POS;


namespace robocooler {
namespace rfid {


class Message {
public:
    typedef std::vector<uint8_t> Buffer;

private:
    uint8_t _btPacketType; ///< заголовок пакета, по умолчанию 0xA0.
    uint8_t _btDataLen;    ///< Количество байтов пакета не содержит сами байты длинны.
    uint8_t _btReadId;     ///< Адрес чтения.
    uint8_t _btCmd;        ///< Код команды пакета.
    Buffer _btAryData;     ///< Параметры команды.
    uint8_t _btCheck;      ///< Контрольная сумма всех байтов.
    Buffer _btAryTranData; ///< Весь пакет.

    uint8_t checkSum(const Buffer &btAryBuffer_, int nStartPos_, int nLen_);

public:    
    Message();
    explicit Message(uint8_t read_id_, uint8_t cmd_, const Buffer &ary_data_);
    explicit Message(uint8_t read_id_, uint8_t cmd_);
    explicit Message(const Buffer &ary_tran_data_);

    const Buffer& getAryTranData() const;
    const Buffer& getAryData() const;
    uint8_t getDataLen() const;
    uint8_t getReadId() const;
    uint8_t getCmd() const;
    uint8_t getPacketType() const;
    uint8_t getErrorCode() const;

    void setAryData(const Buffer &ary_data_);
};
} /// robocooler
} /// rfid
