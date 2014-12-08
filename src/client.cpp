#include <vector>
#include <future>
#include <edisense_types.h>
#include <list>
#include <zmqpp/message.hpp>
#include <zmqpp/socket.hpp>
#include <sstream>
#include <zmqpp/context.hpp>
#include "client.h"

using namespace edisense_comms;
using namespace std;

Client::Client() {
  context = new zmqpp::context;

  serverSocket = new zmqpp::socket(*context, zmqpp::socket_type::reply);
  clientSocket = new zmqpp::socket(*context, zmqpp::socket_type::request);

  clientSocket->set(zmqpp::socket_option::receive_timeout, 1000);
  clientSocket->set(zmqpp::socket_option::send_timeout, 1000);
  serverSocket->set(zmqpp::socket_option::send_timeout, 1000);
}

void Client::start(ClientServer *subscriber) {
  this->subscriber = subscriber;
  run = true;
  server = std::thread(&Client::startServer, this);
  server.detach();
}

void Client::stop() {
  run = false;
  serverSocket->disconnect(buildEndpoint("*", SERVER_SOCKET_PORT));
}

void Client::startServer() {

  zmqpp::endpoint_t acceptAll = buildEndpoint("*", SERVER_SOCKET_PORT);
  serverSocket->bind(acceptAll);

  while (run) {
    zmqpp::message message;
    serverSocket->receive(message);
    string requestName;
    message >> requestName;
    dispositionRequest(requestName, message);
  }

  serverSocket->unbind(acceptAll);
}

future<list<pair<string, PutResult>>> Client::put(transaction_t tid, list<string> &recipients, device_t deviceId, time_t timestamp, time_t expiration, blob data) {
  return async(launch::async, &Client::remotePut, this, tid, std::ref(recipients), deviceId, timestamp, expiration, data);
}

future<list<GetResult>> Client::get(transaction_t tid, list<string> &recipients, device_t deviceId, time_t begin, time_t end) {
  return async(launch::async, &Client::remoteGet, this, tid, std::ref(recipients), deviceId, begin, end);
}

bool Client::dispositionRequest(string topic, zmqpp::message &request) { // TODO extract to functions
  bool wasRequestProcessed = false;
  zmqpp::message response;
  if (topic == "get") {
    wasRequestProcessed = true;
    transaction_t tid;
    device_t deviceId;
    uint32_t begin;
    uint32_t end;
    request >> tid >> deviceId >> begin >> end;

    GetResult getResult = subscriber->handleGetRequest(tid, deviceId, begin, end);

    response.add((uint8_t) getResult.status);
    response.add(getResult.moved_to);
    response.add((uint32_t) getResult.values->size());
    for (Data data : *getResult.values) {
      response.add((uint32_t)data.data.size());
      response.add_raw(&data.data[0], data.data.size());
      response.add((uint32_t) data.expiration);
      response.add((uint32_t) data.timestamp);
    }
  } else if (topic == "put") {
    wasRequestProcessed = true;
    transaction_t tid;
    device_t deviceId;
    uint32_t timestamp;
    uint32_t expiry;
    request >> tid >> deviceId >> timestamp >> expiry;

    PutResult result = subscriber->handlePutRequest(tid, deviceId, timestamp, expiry);
    response.add((uint8_t) result.status);
    response.add(result.moved_to);
  }
  serverSocket->send(response);
  return wasRequestProcessed;
}

list<pair<string, PutResult>> Client::remotePut(transaction_t tid, list<string> &recipients, device_t deviceId, time_t timestamp, time_t expiration, blob data) {
  list<pair<string, PutResult>> respondents;
  zmqpp::message message;
  message << "put" << tid << deviceId << (uint32_t) timestamp << (uint32_t) expiration;
  message.push_back(&data[0], data.size());
  for(string node : recipients) {
    zmqpp::endpoint_t endpoint = buildEndpoint(node, SERVER_SOCKET_PORT);
    zmqpp::message response;
    clientSocket->connect(endpoint);
    clientSocket->send(message);

    // TODO I think this will actually block until we get a response from everyone - introduce a timeout
    clientSocket->receive(response);
    uint8_t tmpStatus;
    node_t movedTo;
    response >> tmpStatus >> movedTo;

    PutResult result = {};
    result.status = (CallStatus) tmpStatus;
    result.moved_to = movedTo;

    pair<string, PutResult> respondent(node, result);

    respondents.push_back(respondent);
    clientSocket->disconnect(endpoint);
  }
  return respondents;
}

std::list<GetResult> Client::remoteGet(transaction_t tid, std::list<std::string> &recipients, device_t deviceId, time_t begin, time_t end) {
  zmqpp::message message;
  message << "get" << tid << deviceId << (uint32_t) begin << (uint32_t) end;
  list<GetResult> combinedResults;

  for(string node : recipients) {
    zmqpp::endpoint_t endpoint = buildEndpoint(node, SERVER_SOCKET_PORT);
    zmqpp::message response;
    clientSocket->connect(endpoint);
    clientSocket->send(message);

    clientSocket->receive(response);
    if (response.remaining() > 0) {
      uint8_t tmpStatus;
      CallStatus status;
      node_t movedNode;
      int dataCount;
      list<Data> *results;

      response >> tmpStatus;
      status = (CallStatus) tmpStatus;
      response >> movedNode;
      response >> dataCount;

      uint32_t pointSize;
      uint32_t timestamp;
      uint32_t expiry;

      for (int i = 0; i < dataCount; i++) {
        response >> pointSize;
        unsigned char* rawPoint;
        rawPoint = (unsigned char *) message.raw_data(pointSize);
        response >> timestamp;
        response >> expiry;

        blob point(rawPoint, rawPoint + pointSize);

        Data data = {};
        data.data = point;
        data.timestamp = timestamp;
        data.expiration = expiry;
        results->push_back(data);
      }

      GetResult result = {};
      result.status = status;
      result.moved_to = movedNode;
      result.values = results;
      combinedResults.push_back(result);
    }

    clientSocket->disconnect(endpoint);
    if (!combinedResults.empty()) break; // Exit once we've gotten data from any node
  }
  return combinedResults;
}

std::string Client::buildEndpoint(std::string target, int port) {
  stringstream buf;
  buf << "tcp://" << target << ':' << port;
  return buf.str();
}