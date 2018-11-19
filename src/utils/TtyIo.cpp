#include <sstream>

#include "TtyIo.hpp"

using namespace utils;


bool TtyIo::setInterfaceAttribs(int speed_) {
    bool ret = true;
    struct termios tty;
    if (tcgetattr(_fd, &tty) < 0) {
        std::stringstream ss;
        ss << std::string("Error from tcgetattr: ") << strerror(errno);
        _what = ss.str();
    } else {
        cfsetospeed(&tty, (speed_t)speed_);
        cfsetispeed(&tty, (speed_t)speed_);
        tty.c_cflag |= (CLOCAL | CREAD);  /// ignore modem controls
        tty.c_cflag &= (~CSIZE | CS8);    /// 8-bit characters
        tty.c_cflag &= ~PARENB;           /// no parity bit
        tty.c_cflag &= ~CSTOPB;           /// only need 1 stop bit
        tty.c_cflag &= ~CRTSCTS;          /// no hardware flowcontrol
        /// setup for non-canonical mode
        tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
        tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
        tty.c_oflag &= ~OPOST;
        /// fetch bytes as they become available
        tty.c_cc[VMIN] = 1;
        tty.c_cc[VTIME] = 5;
        if (tcsetattr(_fd, TCSANOW, &tty) not_eq NO_ERROR) {
            std::stringstream ss;
            ss << std::string("Error from tcsetattr: ") << strerror(errno);
            _what = ss.str();
            ret = false;
        }
    }
    return ret;
}


bool TtyIo::setMincount(bool mcount_, int mtime_) {
    bool ret = true;
    struct termios tty;
    if (tcgetattr(_fd, &tty) < 0) {
        std::stringstream ss;
        ss << std::string("Error tcgetattr: ") << strerror(errno);
        _what = ss.str();
        ret = false;
    } else {
        tty.c_cc[VMIN] = mcount_ ? 1 : 0;
        tty.c_cc[VTIME] = mtime_;  /// half second timer
        if (tcsetattr(_fd, TCSANOW, &tty) < 0) {
            std::stringstream ss;
            ss << std::string("Error tcsetattr: ") << strerror(errno);
            _what = ss.str();
            ret = false;
        }
    }
    return ret;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


TtyIo::TtyIo(const std::string &tty_name_, int tty_speed_) {
    _fd = open(tty_name_.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (_fd < 0) {
        std::stringstream ss;
        ss << std::string("Error opening ") <<  tty_name_ << ": " << strerror(errno);
        _what = ss.str();
    } else {
        setInterfaceAttribs(tty_speed_);
        setMincount(false, 50);  /// set to pure timed read
    }
}


TtyIo::~TtyIo() {
    if (_fd >= 0) {
        close(_fd);
    }
}


std::string TtyIo::what() {
    return _what;
}


int TtyIo::write(const std::vector<uint8_t> &ibuf_) {
    int wlen = ERROR_LEN;
    if (_fd >= 0) {
        wlen = ::write(_fd, &ibuf_[0], ibuf_.size());
        if (wlen not_eq static_cast<int>(ibuf_.size())) {
            std::stringstream ss;
            ss << "Error from write: " << std::to_string(wlen) << ", " << strerror(errno);
            _what = ss.str();
        } else if (wlen == ERROR_LEN) {
            tcdrain(_fd);    /// delay for output
        }
    }
    return wlen;
}


int TtyIo::read(uint8_t *obuf_, size_t len_) {
    int rlen = ERROR_LEN;
    if (_fd >= 0) {
        rlen = ::read(_fd, obuf_, len_);
        if (rlen == ERROR_LEN) {
            std::stringstream ss;
            ss << std::string("Error from read: ") << std::to_string(rlen) << ": " << strerror(errno);
            _what = ss.str();
        }
    }
    return rlen;
}


bool TtyIo::isInit() {
    return (_fd >= 0);
}
