#ifndef BOOST_STATIC_LINK
#   define BOOST_TEST_DYN_LINK
#endif // BOOST_STATIC_LINK

#define BOOST_TEST_MODULE BoostRegex
#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
#include <boost/regex.hpp>

#include <vector>
#include <string>
#include <memory>

#include "Log.hpp"

namespace test {

} // test


BOOST_AUTO_TEST_CASE(TestBoostRegex) {
    LOG_TO_STDOUT;
    std::vector<std::pair<std::string, boost::regex>> ep;
    //ep.push_back(std::make_pair("/rest/peerjs?key=peerjs&id=robot&token=test", boost::regex("^/rest/peerjs/robot.*")));
    //ep.push_back(std::make_pair("/rest/peerjs?key=peerjs&id=robot&token=test", boost::regex("^/rest/peerjs.*")));
    //ep.push_back(std::make_pair("/rest/peerjs/robot/test/id?i=0", boost::regex("^/rest/peerjs/robot.*")));
    //ep.push_back(std::make_pair("/rest/peerjs/robot/test/id?i=0", boost::regex("^/rest/peerjs/robot.*")));
    //for (const auto &e : ep) {
    //    LOG(DEBUG) << e.first << "; " << e.second;
    //    boost::smatch sm;
    //    //if (boost::regex_match(e.first, sm, e.second)) {
    //    if (boost::regex_search(e.first, sm, e.second)) {
    //        LOG(DEBUG) << "{";
    //        for (const auto &s : sm) {
    //            LOG(DEBUG) << s << "[" << e.second << "]";
    //        }
    //        LOG(DEBUG) << "}\n";
    //    }
    //}

    std::set<boost::regex> regexes;
    regexes.insert(boost::regex("по"));
    regexes.insert(boost::regex("пока"));
    regexes.insert(boost::regex("покажи"));
    regexes.insert(boost::regex("покашляй"));
    std::set<std::string> reqs;
    reqs.insert("по");
    reqs.insert("пока");
    reqs.insert("покажи");
    reqs.insert("покашляй");

    for (const auto &r : reqs) {
        LOG(DEBUG) << ">> \"" << r << "\"";
        size_t count = 0;
        for (const auto &x : regexes) {
            LOG(DEBUG) << "[" << x << "]";
            boost::smatch sm;
            if (boost::regex_search(r, sm, x)) {
                LOG(DEBUG) << "{";
                for (const auto &s : sm) {
                    LOG(DEBUG) << s << "[" << x << "]";
                }
                LOG(DEBUG) << "}";
                ++count;
            }
        }
        LOG(DEBUG) << " = " << count;
    }
}
