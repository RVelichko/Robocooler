/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Реализация клиентского подключения к серверу.
 * \author Величко Ростислав
 * \date   06.21.2017
 */

#pragma once

#include <mutex>
#include <memory>
#include <string>
#include <stack>

#include "websocketpp/config/asio_client.hpp"
#include "websocketpp/client.hpp"


#define HANDSHAKE_TIMEOUT 30000

namespace robocooler {
namespace driver {

namespace ws = websocketpp;
namespace wsl = websocketpp::lib;

typedef ws::client<ws::config::asio_tls_client> Client;
typedef Client::connection_ptr PConnection;
typedef Client::message_ptr PMessage;
typedef ws::connection_hdl ConnectionHdl;
typedef std::thread Thread;
typedef std::shared_ptr<Thread> PThread;

class WsClientWorker;


class WsClient {
    WsClientWorker *_client_worker;
    Client _endpoint;
    PConnection _connection;
    PThread _thread;
    
public:
    WsClient(WsClientWorker *client_worker_);
    virtual ~WsClient();
    
    bool connect(const std::string &uri_);
    void close(const ws::close::status::value &code_, const std::string &reason_);
    bool send(const std::string &message_);
};
} /// driver
} /// robocooler
