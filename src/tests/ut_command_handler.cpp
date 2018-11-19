#ifndef BOOST_STATIC_LINK
#   define BOOST_TEST_DYN_LINK
#endif // BOOST_STATIC_LINK

#define BOOST_TEST_MODULE BoostRegex
#define BOOST_AUTO_TEST_MAIN

#include <functional>
#include <string>

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include "Log.hpp"
#include "Bases.hpp"
#include "CommandHandler.hpp"
#include "CommandsHandler.hpp"

namespace test {

typedef robocooler::driver::WorkerBase WorkerBase;
typedef robocooler::driver::GpioControllerBase GpioControllerBase;
typedef robocooler::driver::RfidControllerBase RfidControllerBase;
typedef robocooler::driver::CommandHandlerBase CommandHandlerBase;
typedef robocooler::rfid::CommandsHandler RfidCommandsHandler;


class TestGpioController
    : public GpioControllerBase {
public:
    TestGpioController() {
    }
    
    virtual ~TestGpioController() {
    }
        
    virtual void openLeftDoor(size_t mlscs_ = 0) {
        LOG(DEBUG);
    }

    virtual void closeLeftDoor(size_t mlscs_ = 0) {
        LOG(DEBUG);
    }

    virtual void openRightDoor(size_t mlscs_ = 0) {
        LOG(DEBUG);
    }

    virtual void closeRightDoor(size_t mlscs_ = 0) {
        LOG(DEBUG);
    }

    virtual bool isOpened() {
        LOG(DEBUG);
        return false;
    }
};


class TestRfidController
    : public RfidControllerBase {
public:
    TestRfidController(WorkerBase *worker_) {
    }

    virtual ~TestRfidController() {
    }

    virtual void startInventory(bool need_result_ = false) {
        LOG(DEBUG);
    }

    virtual void stopInventory() {
        LOG(DEBUG);
    }

    virtual void inventory(size_t, bool) {
        LOG(DEBUG);
    }

    virtual bool execute(uint8_t cmd_id_, const std::vector<uint8_t> &data_buf_ = std::vector<uint8_t>()) {
        LOG(DEBUG) << RfidCommandsHandler::toString(cmd_id_) << ": " << RfidCommandsHandler::toString(data_buf_);
        return true;
    }

    virtual std::string getAntSettings() {
        LOG(DEBUG);
        return "";
    }

    virtual void accumulateBuffer() {
        LOG(DEBUG);
    }

    virtual void setBufReadNumAttempt(size_t buf_read_num_attempt_) {
        LOG(DEBUG);
    }

    virtual size_t getBufReadNumAttempt() {
        LOG(DEBUG);
        return 0;
    }

    virtual void setReadAntennsCount(size_t count_) {
        LOG(DEBUG);
    }

    virtual void setReadAntennsTimeout(size_t timeout_) {
        LOG(DEBUG);
    }

    virtual void findBrokenLabels(size_t iterations_num_) {
        LOG(DEBUG);
    }
};


class TestWorker 
    : public WorkerBase {
    std::shared_ptr<TestGpioController> _gpio_controller;
    std::shared_ptr<TestRfidController> _rfid_controller;
    
public:    
    TestWorker() 
        : _gpio_controller(std::make_shared<TestGpioController>()) {
        _rfid_controller = std::make_shared<TestRfidController>(this);
    }
    
    virtual ~TestWorker() {
    }
    
    virtual std::string getCoolerId() {
        return "0";
    }

    virtual void send(const std::string &json_str_) {
    }

    virtual GpioControllerBase* getGpioController() {
        return _gpio_controller.get();
    }

    virtual RfidControllerBase* getRfidController() {
        return _rfid_controller.get();
    }

    virtual CommandHandlerBase* getCommandHandler() {
        return nullptr;
    }
};
} // test


typedef robocooler::driver::CommandHandler CommandHandler;
typedef test::TestWorker TestWorker;


BOOST_AUTO_TEST_CASE(TestCommandHandler) {
    LOG_TO_STDOUT;
    TestWorker worker;
    CommandHandler ch(&worker);
    ch.handle(R"({"H":"PlantHub","M":"sendPutOrTakenGoodsByUser","A":{"O":[1,2], "I":[3,4,5], "PlantId": 0}})");
    ch.handle(R"({"M":"openRightDoor","H":"PlantHub","A":""})");
    ch.handle(R"({"M":"openLeftDoor","H":"PlantHub","A":""})");
    ch.handle(R"({"M":"closeLeftDoor","H":"PlantHub","A":""})");
    ch.handle(R"({"M":"closeRightDoor","H":"PlantHub","A":""})");
    ch.handle(R"({"H":"PlantHub", "M":"configureRFIDDevice", "A":{"frequencyRegion":{"startFrequency":0,"endFrequency":59,"region":1},"powers":[0,1,2,33]}})");
}
