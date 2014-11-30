#include <string>
#include "edisense_comms.h"
#include "client.h"

namespace edisense_comms {
  class MonitorSubscriber;
}

class edisense_comms::Monitor : Client {

  MonitorSubscriber* subscriber;

public:
  /*!
   * Initializes the Monitor on the network
   */
  virtual void start(MonitorSubscriber* subscriber);

  /*!
   * Requests all nodes to send their current utilization
   */
  void syncUtilization(node_t node);

  /*!
   * Ask all Nodes to prepare for a configuration change
   */
  void sendPrepareConfiguration();

  /*!
   * send the configuration change to all Nodes
   */
  void sendConfiguration(std::string filePath);
};

class edisense_comms::MonitorSubscriber : public ClientSubscriber {
  /*!
   * Invoked when the monitor receives an utilization update from a Node.  This may be in response to a request or sent
   * by the node when utilization changes during normal operation
   */
  virtual void utilizationUpdate(node_t node, float utilization) = 0;

  /*!
   * Invoked when a Node has marked itself as ready to accept a configuration change
   */
  virtual void configurationReadyReceived(messageID message, node_t readyNode) = 0;

  /*!
   * Invoked when a Node has safely persisted the new configuration
   */
  virtual void configurationAcknowledged(messageID message, node_t configuredNode) = 0;
};

