#include <monitor.h>

using namespace edisense_comms;

void Monitor::start(MonitorSubscriber *subscriber) {
  this->subscriber = subscriber;
  Client::start(subscriber);
}

void Monitor::syncUtilization(node_t node) {

}

void Monitor::sendPrepareConfiguration() {

}

void Monitor::sendConfiguration(std::string filePath) {

}


