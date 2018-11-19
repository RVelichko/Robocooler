/*!
 * \brief  Объект объмена по последовательностному порту ввода/вывода.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   14.09.2017
 */

#pragma once

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>

#include <string>
#include <vector>


#define NO_ERROR 0
#define ERROR_LEN -1

namespace utils {

class TtyIo {
    int _fd;           /// Дескриптор файла порта.
    std::string _what; /// Строка с сообщением об ошибке.

    /**
     * Метод выполняет установку параметров интерфейса.
     * \param  speed_  Скорость объмена килобит в секунду.
     */
    bool setInterfaceAttribs(int speed_);

    /**
     * Метод выполняет установку минимального количества попыток чтения.
     * \param  mcount_  Количество попыток.
     * \param  mtime_  Таймаут ожидания.
     */
    bool setMincount(bool mcount_, int mtime_);

public:
    /**
     * Конструктор открывает файл порта ввода/вывода и устанавливает параметры объмена.
     * \param  tty_name_  Имя порта.
     * \param  tty_speed_  Скорость объмена килобит в секунду.
     */
    TtyIo(const std::string &tty_name_ = "/dev/ttyUSB0", int tty_speed_ = B115200);
    virtual ~TtyIo();

    /**
     * Метод возвращает строку с сообщением при ошибке.
     */
    std::string what();

    /**
     * Метод выполняет запись байт в порт объмена.
     * \param  ibuf_  Буфер с данными на отправку.
     * \return  Возвращает количество отправленных байт либо -1 в случае ошибки.
     */
    int write(const std::vector<uint8_t> &ibuf_);

    /**
     * Метод выполняет чтение байт из порта объмена.
     * \param  obuf_  Указатель на массив для записи полученных данных.
     * \param  len_   Доступный размер переданного массива.
     * \return  Возвращает количество отправленных байт либо -1 в случае ошибки.
     */
    int read(uint8_t *obuf_, size_t len_);

    /**
     * Метод возвращает true если порт подключён и готов к объмену, false - в противном случае.
     */
    bool isInit();
};
} /// utils
