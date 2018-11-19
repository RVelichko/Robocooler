#include "Log.hpp"
#include "CommandsHandler.hpp"
#include "Message.hpp"


using namespace robocooler;
using namespace rfid;

typedef Message::Buffer Buffer; 
typedef CommandsHandler CmdHdl; 


uint8_t Message::checkSum(const Buffer &btAryBuffer_, int nStartPos_, int nLen_) {
    uint8_t btSum = RFID_NULL;
    for (int nloop = nStartPos_; nloop < nStartPos_ + nLen_; nloop++) {
        btSum += btAryBuffer_[nloop];
    }
    return (((~btSum) + 1) & 0xFF);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Message::Message() 
    : _btPacketType(RFID_NULL)
    , _btDataLen(RFID_NULL)  
    , _btReadId(RFID_NULL)        
    , _btCmd(RFID_NULL)           
    , _btCheck(RFID_NULL) {
}


Message::Message(uint8_t btReadId_, uint8_t btCmd_, const Buffer &btAryData_)            
    : _btPacketType(RFID_HEAD)
    , _btDataLen(RFID_NULL)  
    , _btReadId(btReadId_)        
    , _btCmd(btCmd_)           
    , _btCheck(RFID_NULL) {
    size_t len = btAryData_.size();
    _btDataLen = len + 3;
    _btAryData = btAryData_;
    _btAryTranData.resize(len + 5, RFID_NULL);
    _btAryTranData[0] = _btPacketType;
    _btAryTranData[1] = _btDataLen;
    _btAryTranData[2] = _btReadId;
    _btAryTranData[3] = _btCmd;
    _btAryTranData.insert(_btAryTranData.begin() + 4, _btAryData.begin(), _btAryData.end());
    _btCheck = checkSum(_btAryTranData, 0, len + 4);
    _btAryTranData[len + 4] = _btCheck;
//    LOG(DEBUG) << CmdHdl::toString(_btPacketType) << " "
//               << CmdHdl::toString(_btDataLen) << " "
//               << CmdHdl::toString(_btReadId) << " "
//               << CmdHdl::toString(_btCmd) << " "
//               << CmdHdl::toString(_btAryData) << " "
//               << CmdHdl::toString(_btCheck);
}


Message::Message(uint8_t btReadId_, uint8_t btCmd_)        
    : _btPacketType(RFID_HEAD)
    , _btDataLen(3)  
    , _btReadId(btReadId_)        
    , _btCmd(btCmd_)           
    , _btCheck(RFID_NULL) {
    _btAryTranData.resize(5, RFID_NULL);
    _btAryTranData[0] = _btPacketType;
    _btAryTranData[1] = _btDataLen;
    _btAryTranData[2] = _btReadId;
    _btAryTranData[3] = _btCmd;
    _btCheck = checkSum(_btAryTranData, 0, 4);
    _btAryTranData[4] = _btCheck;
//    LOG(DEBUG) << CmdHdl::toString(_btPacketType) << " "
//               << CmdHdl::toString(_btDataLen) << " "
//               << CmdHdl::toString(_btReadId) << " "
//               << CmdHdl::toString(_btCmd) << " "
//               << CmdHdl::toString(_btAryData) << " "
//               << CmdHdl::toString(_btCheck);
}


Message::Message(const Buffer &btAryTranData_)
    : _btPacketType(RFID_NULL)
    , _btDataLen(RFID_NULL)  
    , _btReadId(RFID_NULL)        
    , _btCmd(RFID_NULL)           
    , _btCheck(RFID_NULL) {
    size_t nLen = btAryTranData_.size();
    _btAryTranData = btAryTranData_;
    uint8_t btCK = checkSum(_btAryTranData, 0, nLen - 1);
    if (btCK not_eq _btAryTranData[nLen - 1]) {
        return;
    }
    _btPacketType = _btAryTranData[0];
    _btDataLen = _btAryTranData[1];
    _btReadId = _btAryTranData[2];
    _btCmd = _btAryTranData[3];
    _btCheck = _btAryTranData[nLen - 1];
    if (nLen > 5) {
        _btAryData.resize(nLen - 5, RFID_NULL);
        for (size_t nloop = 0; nloop < nLen - 5; ++nloop) {
            _btAryData[nloop] = _btAryTranData[nloop + 4];
        }
    }
    //LOG(DEBUG) << CmdHdl::toString(_btPacketType) << " "
    //           << CmdHdl::toString(_btDataLen) << " "
    //           << CmdHdl::toString(_btReadId) << " "
    //           << CmdHdl::toString(_btCmd) << " "
    //           << CmdHdl::toString(_btAryData) << " "
    //           << CmdHdl::toString(_btCheck);
}


void Message::setAryData(const Buffer &ary_data_) {
    _btDataLen = ary_data_.size();
    _btAryData = ary_data_;
    _btAryTranData.resize(_btDataLen + 5, RFID_NULL);
    _btAryTranData[0] = _btPacketType;
    _btAryTranData[1] = _btDataLen;
    _btAryTranData[2] = _btReadId;
    _btAryTranData[3] = _btCmd;
    for (size_t nloop = 0; nloop < _btDataLen; ++nloop) {
        _btAryTranData[nloop + 4] = _btAryData[nloop];
    }
    _btCheck = checkSum(_btAryTranData, 0, _btDataLen + 4);
    _btAryTranData[_btDataLen + 4] = _btCheck;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


const Buffer& Message::getAryTranData() const {
    return _btAryTranData;
}


const Buffer& Message::getAryData() const {
    return _btAryData;
}


uint8_t Message::getDataLen() const {
    return _btDataLen;
}


uint8_t Message::getReadId() const {
    return _btReadId;
}


uint8_t Message::getCmd() const {
    return _btCmd;
}


uint8_t Message::getPacketType() const {
    return _btPacketType;
}


uint8_t Message::getErrorCode() const {
    return _btAryTranData[RFID_ERR_CODE_POS];
}
