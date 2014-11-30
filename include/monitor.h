#include <string>
#include "edisense_comms.h"
#include "client.h"

namespace edisesnse_comms {
  class MonitorSubscriber;
}

using namespace edisense_comms;

class Monitor : Client {
public:
  void syncUtilization(node_t node);
  void sendPrepareConfiguration();
  void sendConfiguration(std::string filePath);
};

class MonitorSubscriber : ClientSubscriber {
  virtual void utilizationUpdate(node_t node, float utilization) = 0;
  virtual void configurationReadyReceived(node_t readyNode) = 0;
  virtual void configurationAcknowledged(node_t configuredNode) = 0;
};

