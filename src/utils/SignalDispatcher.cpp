#include "SignalDispatcher.hpp"

using namespace utils;

SignalDispatcherResources::Handlers SignalDispatcherResources::_resources = SignalDispatcherResources::Handlers();
SignalDispatcherResources::Condition SignalDispatcherResources::_wait_condition;
SignalDispatcherResources::Mutex SignalDispatcherResources::_wait_mutex;

