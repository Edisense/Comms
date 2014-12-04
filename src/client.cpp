#include <vector>
#include <future>
#include <edisense_types.h>
#include <list>
#include <zmqpp/message.hpp>
#include <sstream>
#include <Foundation/Foundation.h>
#include <CoreLocation/CoreLocation.h>
#include <Python/Python.h>
#include <AppKit/AppKit.h>
#include "client.h"

using namespace edisense_comms;
using namespace std;

Client::Client() :
  serverSocket(context, zmqpp::socket_type::reply),
  clientSocket(context, zmqpp::socket_type::request)
{
  clientSocket.set(zmqpp::socket_option::receive_timeout, 1000);
  clientSocket.set(zmqpp::socket_option::send_timeout, 1000);
  serverSocket.set(zmqpp::socket_option::send_timeout, 1000);
}

void Client::start(ClientServer *subscriber) {
  this->subscriber = subscriber;
  run = true;
  std::thread(&Client::startServer, this);
}

void Client::stop() {
  run = false;
}

void Client::startServer() {
  while (run) {
    zmqpp::message message;
    serverSocket.receive(message);
    string requestName;
    message >> requestName;
    dispositionRequest(requestName, message);
  }
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
    response.add(getResult.values->size());
    for (Data data : *getResult.values) {
      response.add(data.data.size());
      response.add(&data.data[0], data.data.size());
      response.add(data.expiration);
      response.add(data.timestamp);
    }
  } else if (topic == "put") {
    wasRequestProcessed = true;
    transaction_t tid;
    device_t deviceId;
    uint32_t timestamp;
    uint32_t expiry;
    request >> tid >> deviceId >> timestamp >> expiry;

    PutResult result = subscriber->handlePutRequest(tid, deviceId, timestamp, expiry);
    response.add(result.status);
    response.add(result.moved_to);
  }
  serverSocket.send(response);
  return wasRequestProcessed;
}

list<string> Client::remotePut(transaction_t tid, list<string> &recipients, device_t deviceId, time_t timestamp, time_t expiration, blob data) {
  list<string> respondents;
  zmqpp::message message;
  message << "put" << tid << deviceId << (uint32_t) timestamp << (uint32_t) expiration;
  message.push_front(&data[0], data.size());
  for(string node : recipients) {
    zmqpp::endpoint_t endpoint = buildEndpoint(node, SERVER_SOCKET_PORT);
    zmqpp::message response;
    clientSocket.bind(endpoint);
    clientSocket.send(message);

    // TODO I think this will actually block until we get a response from everyone - introduce a timeout
    clientSocket.receive(response);
    bool allGood;
    response >> allGood;
    if (allGood) {
      respondents.push_back(node);
    }
    clientSocket.unbind(endpoint);
  }
  return respondents;
}

std::list<GetResult> Client::remoteGet(transaction_t tid, std::list<std::string> &recipients, device_t deviceId, time_t begin, time_t end) {
  zmqpp::message message;
  message << "put" << tid << deviceId << (uint32_t) begin << (uint32_t) end;
  list<GetResult> combinedResults;

  for(string node : recipients) {
    zmqpp::endpoint_t endpoint = buildEndpoint(node, SERVER_SOCKET_PORT);
    zmqpp::message response;
    clientSocket.bind(endpoint);
    clientSocket.send(message);

    // TODO I think this will actually block until we get a response from everyone - introduce a timeout
    clientSocket.receive(response);
    if (response.remaining() > 0) {
      CallStatus status;
      node_t movedNode;
      int dataCount;
      list<Data> *results;

      message >> status;
      message >> movedNode;
      message >> dataCount;

      uint32_t pointSize;
      uint32_t timestamp;
      uint32_t expiry;

      for (int i = 0; i < dataCount; i++) {
        message >> pointSize;
        blob point(pointSize);
        message.get(&point[0], pointSize);
        message >> timestamp;
        message >> expiry;

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

    clientSocket.unbind(endpoint);
    if (!combinedResults.empty()) break; // Exit once we've gotten data from any node
  }
  return combinedResults;
}

zmqpp::endpoint_t Client::buildEndpoint(std::string target, int port) {
  stringstream buf;
  buf << "tcp://" << target << ':' << port;
  return buf.str();
}