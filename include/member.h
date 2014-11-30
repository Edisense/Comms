#include "edisense_comms.h"
#include "edisense_types.h"
#include "client.h"

namespace edisense_comms {
  class MemberSubscriber;
}

class edisense_comms::Member : Client {

  MemberSubscriber* subscriber;

public:

  Member();

  /*!
   * Initializes the member on the network
   */
  virtual void start(MemberSubscriber *subscriber);

  /*!
   * Asks for a node willing to accept this Node's data for a given sensor.  It is expected that only Range owners will
   * respond to this request
   */
  messageID sendPrepare(device_t sensor);

  /*!
   * Notifies all current nodes that this Node should join the cluster
   */
  void sendJoin();

  /*!
   * Sends this node's data for a given sensor to another node
   */
  messageID sendData(node_t nodeId, device_t sensorId, std::string filePath);
};

class edisense_comms::MemberSubscriber : public ClientSubscriber {
public:
  /*!
   * Invoked when a set of members have indicated their willingness to receive the data discussed in messageID
   */
  virtual void readyReceived(messageID message, std::set<node_t> node) = 0;

  /*!
   * Invoked when the member has committed the donation sent in messageID
   */
  virtual void donationAccepted(messageID id, node_t node) = 0;

  /*!
   * Invoked when a member has asked this node to receive a data donation.  Returns true if this member is willing to
   * receive the donation
   *
   * results in a readyReceived message if true
   */
  virtual bool prepare(messageID message, node_t node) = 0;

  /*!
   * Invoked when the member has sent data to this node.  Returns true if the transaction concluded successfully.
   *
   * Results in a donationAccepted message if true
   */
  virtual bool accept() = 0;
};