#include <functional>

#include "WsClientWorker.hpp"
#include "WsClient.hpp"


using namespace robocooler;
using namespace driver;

namespace ph = std::placeholders;


WsClient::WsClient(WsClientWorker *client_worker_) 
    : _client_worker(client_worker_) {
    LOG(DEBUG);

    _endpoint.set_tls_init_handler([this](websocketpp::connection_hdl) {
        auto ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv1);

        try {
            ctx->set_options(boost::asio::ssl::context::default_workarounds |
                             boost::asio::ssl::context::no_sslv2 |
                             boost::asio::ssl::context::single_dh_use);
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
        return ctx;
        //return websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv1);
    });

    _endpoint.set_open_handshake_timeout(HANDSHAKE_TIMEOUT);
    _endpoint.set_close_handshake_timeout(HANDSHAKE_TIMEOUT);

    _endpoint.clear_access_channels(ws::log::alevel::all);
    _endpoint.clear_error_channels(ws::log::elevel::all);
    _endpoint.init_asio();
    _endpoint.start_perpetual();
    /// Запуск потока клиента.
    _thread = std::shared_ptr<Thread>(new Thread(&Client::run, &_endpoint), [this](Thread *t_) {
        _endpoint.stop_perpetual();
        _endpoint.stop();
        t_->join();
        LOG(DEBUG) << "Closing connection ";
    });
}


WsClient::~WsClient() {
    LOG(DEBUG);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool WsClient::connect(const std::string &uri_) {
    bool result = true;
    wsl::error_code ec;
    _connection = _endpoint.get_connection(uri_, ec);
    if (ec) {
        LOG(ERROR) << "Connect initialization error: " << ec.message();
        result = false;
    }
    _connection->set_open_handler(std::bind(&WsClientWorker::onOpen, _client_worker, &_endpoint, ph::_1));
    _connection->set_fail_handler(std::bind(&WsClientWorker::onError, _client_worker, &_endpoint, ph::_1));
    _connection->set_close_handler(std::bind(&WsClientWorker::onClose, _client_worker, &_endpoint, ph::_1));
    _connection->set_message_handler(std::bind(&WsClientWorker::onMessage, _client_worker, ph::_1, ph::_2));
    _endpoint.connect(_connection);
    LOG(DEBUG) << uri_ << " is " << (result ? "TRUE" : "FALSE");
    return result;
}


void WsClient::close(const ws::close::status::value &code_, const std::string &reason_) {
    LOG(DEBUG);
    if (_connection) {
        wsl::error_code ec;
        ConnectionHdl handle = _connection->get_handle();
        _endpoint.close(handle, code_, reason_, ec);
        if (ec) {
            LOG(DEBUG) << "Error initiating close: " << ec.message();
        }
    } else {
        LOG(DEBUG) << "Connection is NULL.";
    }
}


bool WsClient::send(const std::string &message_) {
    bool result = true;
    if (_connection) {
        LOG(DEBUG) << message_;
        wsl::error_code ec;
        ConnectionHdl handle = _connection->get_handle();
        _endpoint.send(handle, message_, ws::frame::opcode::text, ec);
        if (ec) {
            LOG(ERROR) << "Error sending message: " << ec.message();
            result = false;
        }
    } else {
        LOG(DEBUG) << "Connection is NULL.";
        result = false;
    }
    return result;
}
