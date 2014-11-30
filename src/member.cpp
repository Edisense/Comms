#include "member.h"

using namespace edisense_comms;

Member::Member() {

}

void Member::start(MemberSubscriber * subscriber) {
  this->subscriber = subscriber;
  Client::start(subscriber);
}

messageID Member::sendPrepare(device_t sensor) {
  messageID id = Client::generateMessageID();
  // TODO Send message
  return id;
}

void Member::sendJoin() {

}

messageID Member::sendData(node_t nodeId, device_t sensorId, std::string filePath) {
  messageID id = Client::generateMessageID();

  return id;
}

