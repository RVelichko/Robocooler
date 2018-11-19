/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Класс реализует выделение JSON посылок из потока входных сообщений.
 * \author Величко Ростислав
 * \date   07.03.2017
 */

#pragma once

#include <functional>
#include <string>
#include <stack>

namespace robocooler {
namespace driver {
    
typedef std::function<void(const std::string&)> OnJsonFunc;
typedef std::stack<size_t> Stack;

    
class JsonExtractor {
    OnJsonFunc _on_json_fn; ///< Функтор принимающий готовый json.
    std::string _buf; ///< Буфер принятых данных.
    Stack _stack; ///< Глобальный контроль JSON посылок.
    
public:
    /**
     * \brief Конструктор инициализирует функтор, в который передаётся готовый json. 
     */
    JsonExtractor(const OnJsonFunc &on_json_fn_);

    /**
     * \brief Метод вызыватеся при получении очередного блока данных с кусками JSON. 
     * \param msg_ Входная строка необработанных данных.
     */
    void onMessage(const std::string &msg_);
    
    /**
     * \brief Метод выполняет очистку состояний и буферов при ошибках снаружи. 
     */
    void clear();
};
} /// driver
} /// robocooler
