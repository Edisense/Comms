#include "client.h"

using namespace edisense_comms;

Client::Client() {
  // TODO Generate ClientID
}

void Client::start(ClientSubscriber * subscriber) {
  this->subscriber = subscriber;
  // TODO Setup ZMQ
}

void Client::stop() {
  //TODO Stop ZMQ
}

void Client::syncNodes() {
  // TODO Send ZMQ sync message
}

void Client::syncSensors() {
  // TODO Send ZMQ sync message
}

messageID Client::requestData(device_t sensor, time_t start, time_t end) {
  messageID id = generateMessageID();
  // TODO Send ZMQ data message
  return id;
}

messageID Client::generateMessageID() {
  // TODO generate unique ID - clientID + timestamp?
  return 0;
}

