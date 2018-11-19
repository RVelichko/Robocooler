/** Copyright &copy; 2015, Alfarobotics.
 * \brief  Базовые обработчики событий THRIFT сервера.
 * \author Величко Ростислав
 * \date   10.12.2015
 */

#pragma once

#include <cstdint>
#include <string>

#include <thrift/server/TServer.h>
#include <thrift/TProcessor.h>


namespace utils {

/// Обработчик событий подключений.
class ServerEventHandler
    : public apache::thrift::server::TServerEventHandler {
public:
    ServerEventHandler();

    /**
     * Вызывается при запуске сервера.
     */
    void preServe() override;

    /**
     * Вызывается при подуключении нового клиента и ожидающего обработки.
     */
    void* createContext(boost::shared_ptr<apache::thrift::protocol::TProtocol>,
                        boost::shared_ptr<apache::thrift::protocol::TProtocol>) override;

    /**
     * Вызывается при окончании запроса от клиента.
     */
    void deleteContext(void*,
                       boost::shared_ptr<apache::thrift::protocol::TProtocol>,
                       boost::shared_ptr<apache::thrift::protocol::TProtocol>) override;

    /**
     * Вызывается при ожидании клиентом выполнения процесса.
     */
    void processContext(void* serverContext, boost::shared_ptr<apache::thrift::transport::TTransport> transport) override;
};


/// Обработчик событий от Thrift процессора.
class ProcessorEventHandler
    : public apache::thrift::TProcessorEventHandler {
public:
    ProcessorEventHandler();

    /**
    * Called before calling other callback methods.
    * Expected to return some sort of context object.
    * The return value is passed to all other callbacks
    * for that function invocation.
    */
    void* getContext(const char*, void*) override;

    /**
     * Вызывается при освобождении ресурса, ассоциированного с контекстом
     */
    void freeContext(void*, const char*) override;

    /**
     * Вызывается при чтении аргументов.
     */
    void preRead(void*, const char* fn_name) override;

    /**
     * Вызывается между чтением аргументов и вызовом хендлера
     */
    void postRead(void*, const char* fn_name, uint32_t bytes) override;

    /**
     * Вызывается между вызовом хендлера и записью ответа
     */
    void preWrite(void*, const char* fn_name) override;

    /**
     * Вызывается после записи ответа.
     */
    void postWrite(void*, const char* fn_name, uint32_t bytes) override;

    /**
     * Вызывается когда асинхронная функция полностью выполнена
     */
    void asyncComplete(void*, const char* fn_name) override;

    /**
     * Вызывается если хендлер генерирует неопределённый эксепшон
     */
    void handlerError(void*, const char* fn_name) override;
};
} // utils
