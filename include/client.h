#include <list>
#include <set>
#include <string>
#include "edisense_comms.h"
#include "edisense_types.h"


namespace edisense_comms {
  class ClientSubscriber;
}

class edisense_comms::Client {

  ClientSubscriber *subscriber;

public:

  Client();

  /*!
   * Initializes the Client on the network
   */
  void start(const ClientSubscriber * subscriber);

  /*!
   * Stops operations on the client
   */
  void stop();

  /*!
   * Request the current set of responsive nodes
   */
  void syncNodes();

  /*!
   * Request the set of currently known sensors
   */
  void syncSensors();

  /*!
   * Returns all data for a given sensor over the provided timestamp range (in seconds since epoch)
   */
  messageID requestData(device_t sensor, time_t start, time_t end);

private:
  /*!
   * Generates a unique identifier for a given message in the current session
   */
  messageID generateMessageID();

};

class edisense_comms::ClientSubscriber {
public:
  /*!
   * Invoked when a new set of Nodes is received
   */
  virtual void nodeSetReceived(std::set<Ownership> nodes) = 0;

  /*!
   * Invoked when a new set of Sensors is received
   */
  virtual void sensorSetReceived(std::set<device_t> sensors) = 0;

  /*!
   * Invoked when a response to our data request has been received
   x*/
  virtual void dataReceived(messageID request, std::string filePath) = 0;
};