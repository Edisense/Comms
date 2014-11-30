#include "edisense_comms.h"
#include "edisense_types.h"
#include "client.h"

namespace edisense_comms {
  class MemberSubscriber;
}

using namespace edisense_comms;

class Member : Client {

public:

  Member();

  void start(MemberSubscriber subscriber);
  void sendPrepare(device_t sensor);
  void sendJoin();
  messageID sendData(node_t nodeId, device_t sensorId, std::string filePath);
  void sendAccepted();

};

class MemberSubscriber : ClientSubscriber {
public:
  virtual void readyReceived(std::set<node_t> node) = 0;
  virtual void donationAccepted(messageID id, node_t node) = 0;
  virtual bool prepare() = 0;
  virtual bool accept() = 0;
};