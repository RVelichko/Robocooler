/** Copyright &copy; 2015, Alfarobotics.
 * \brief  Базовые обработчики событий THRIFT сервера.
 * \author Величко Ростислав
 * \date   10.12.2015
 */

#include <thrift/transport/TSocket.h>

#include "Log.hpp"
#include "ServiceHandler.hpp"


using namespace utils;


ServerEventHandler::ServerEventHandler()
{}


void ServerEventHandler::preServe() {
}


void* ServerEventHandler::createContext(boost::shared_ptr<apache::thrift::protocol::TProtocol>,
                                        boost::shared_ptr<apache::thrift::protocol::TProtocol>) {
    LOG(DEBUG) << "New client conected.";
    return nullptr;
}


void ServerEventHandler::deleteContext(void*,
                                       boost::shared_ptr<apache::thrift::protocol::TProtocol>,
                                       boost::shared_ptr<apache::thrift::protocol::TProtocol>) {
    LOG(DEBUG) << "Client disconected.";
}


void ServerEventHandler::processContext(void* serverContext, boost::shared_ptr<apache::thrift::transport::TTransport> transport) {
    if (apache::thrift::transport::TSocket* sock = static_cast<apache::thrift::transport::TSocket*>(transport.get())) {
        LOG(DEBUG) << "Вызов от: " << sock->getPeerAddress();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ProcessorEventHandler::ProcessorEventHandler()
{}


void* ProcessorEventHandler::getContext(const char*, void*) {
    return nullptr;
}


void ProcessorEventHandler::freeContext(void*, const char*) {
}


void ProcessorEventHandler::preRead(void*, const char* fn_name) {
    LOG(DEBUG) << "preRead \"" << fn_name << "\" метода";
}


void ProcessorEventHandler::postRead(void*, const char* fn_name, uint32_t bytes) {
    LOG(DEBUG) << "postRead \"" << fn_name << "\" метода,  байт=" << bytes;
}


void ProcessorEventHandler::preWrite(void*, const char* fn_name) {
    LOG(DEBUG) << "preWrite \"" << fn_name << "\" метода";
}


void ProcessorEventHandler::postWrite(void*, const char* fn_name, uint32_t bytes) {
    LOG(DEBUG) << "postWrite \"" << fn_name << "\" метода,  байт=" << bytes;
}


void ProcessorEventHandler::asyncComplete(void*, const char* fn_name) {
    LOG(DEBUG) <<  "Завершена асинхронная операция: \"" << fn_name << "\"";
}


void ProcessorEventHandler::handlerError(void*, const char* fn_name) {
    LOG(DEBUG) <<  "Ошибка в методе: \"" << fn_name << "\"";
}
