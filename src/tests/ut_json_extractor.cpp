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
#include "JsonExtractor.hpp"


namespace ph = std::placeholders;

namespace test {

class HandleJson {
public:    
    HandleJson() {
    }
    
    void onJson(const std::string &json_) {
        LOG(DEBUG) << json_;
    }
};
} // test

typedef test::HandleJson HandleJson;
typedef robocooler::driver::JsonExtractor JsonExtractor;


BOOST_AUTO_TEST_CASE(TestJsonExtractor) {
    LOG_TO_STDOUT;
    HandleJson handle;
    JsonExtractor je(std::bind(&HandleJson::onJson, &handle, ph::_1));
    je.onMessage(R"({}�z{"C":"d-5AC30A07-N,1|O,0|P,1","M":[{"H")");
    je.onMessage(R"(:"PlantHub","M":"sendPutOrTakenGoodsByUser","A":[{"O":[], "I":[5,2,6,8], "PlantId":1}]}]}�{)");
    je.onMessage(R"({}�{}�{}�{}�{}�{}�{}�{}�{}�)");
    je.onMessage(R"({"C":"d-5AC30A07-q,0|r,0|s,1","S":1,"M":[]}�{}�{}�{}�z{"C":"d-5AC30A07-q,1|r,0|s,1",")");
}
