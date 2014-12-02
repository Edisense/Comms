#include <list>
#include <set>
#include <string>
#include <zmqpp/socket.hpp>
#include <zmqpp/context.hpp>
#include <future>
#include <vector>
#include <ctime>
#include "edisense_comms.h"
#include "edisense_types.h"

#define SERVER_SOCKET_PORT 4567

namespace edisense_comms {
  class ClientServer;
}

class edisense_comms::Client {

  ClientServer *subscriber;

public:

  Client();

  /*!
    Initializes the Client on the network

    \param handler a Handler for performing the server operations associated with this role
   */
  virtual void start(ClientServer *handler);

  /*!
    Stops operations on the client
   */
  virtual void stop();

  /*!
    puts a blob of data onto the cluster
    \param tid an ID for the transaction
    \param recipients a list of hostnames who should have the data
    \param deviceId an id for the device that provided the data
    \param timestamp the time at which the data was recorded
    \param end the end of the time range for the query
    \param data the binary data to store
    \return a list of recipients that acknowledged the put
   */
  std::future<std::list<std::string>> put(transaction_t tid, std::list<std::string> &recipients,
      device_t deviceId, time_t timestamp, time_t expiration,
      blob data);

  /*!
    gets a blob of data from the cluster
    \param tid an ID for the transaction
    \param recipients a list of hostnames who should have the data
    \param deviceId an id for the device that provided the data
    \param start the beginning of the time range for the request
    \param end the end of the time range for the query
    \return a list of all binary data falling within the search parameters
   */
  std::future<std::list<blob>> get(transaction_t tid, std::list<std::string> &recipients,
      device_t deviceId, time_t begin, time_t end);

protected:
  zmqpp::context context;
  zmqpp::socket clientSocket;
  zmqpp::socket serverSocket;

  static zmqpp::endpoint_t buildEndpoint(std::string target, int port);

  virtual bool dispositionRequest(zmqpp::message);

private:
  std::atomic<bool> run = false;

  void startServer();

  std::list<std::string> remotePut(transaction_t tid, std::list<std::string> &recipients,
      device_t deviceId, time_t timestamp, time_t expiration,
      blob data);

  std::list<blob> remoteGet(transaction_t tid, std::list<std::string> &recipients,
      device_t deviceId, time_t begin, time_t end);
};


class edisense_comms::ClientServer { // TODO Work on nomenclature
public:
  /*!
    Handle a request for data
   */
  virtual std::list<blob> handleGetRequest(transaction_t tid, device_t deviceId, time_t begin, time_t end)
      = 0;

  virtual bool handlePutRequest(transaction_t tid, device_t deviceId, time_t timestamp, time_t expiry)
      = 0;
};