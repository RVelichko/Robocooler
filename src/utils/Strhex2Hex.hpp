/*!
 * \brief  Класс перобразования строки с hex данными в hex массив байт.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   14.09.2017
 */

#pragma once

#include <vector>
#include <string>

namespace utils {

class Strhex2Hex {
    std::vector<uint8_t> _buf;

public:
    /**
     * \brief  Конструктор разбивает входную строку на слова, описывающих hex байты и преобразует их непосредственно в hex байты.
     * \param  str_data_ Строка, описывающая hex байты, разделённые пробелами или запятыми.
     */
    explicit Strhex2Hex(const std::string &str_data_);

    /**
     * \brief  Оператор, возвращает массив успешно выделенных hex байт.
     */
    operator std::vector<uint8_t> ();

    /**
     * \brief  Оператор, возвращает массив успешно выделенных hex байт, записанных в строку (обратное преобразование результата).
     */
    operator std::string ();
};
} // utils
