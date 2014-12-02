#include <vector>
#include <future>
#include <edisense_types.h>
#include <list>
#include <zmqpp/message.hpp>
#include <sstream>
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
    dispositionRequest(message);
  }
}

future<list<string>> Client::put(transaction_t tid, list<string> &recipients, device_t deviceId, time_t timestamp, time_t expiration, vector<unsigned char> data) {
  return async(launch::async, &Client::remoteGet, this, tid, recipients, deviceId, timestamp, expiration, data);
}

future<list<vector<unsigned char>>> Client::get(transaction_t tid, list<string> &recipients, device_t deviceId, time_t begin, time_t end) {
  return async(launch::async, &Client::remoteGet, this, tid, recipients, deviceId, begin, end);
}

bool Client::dispositionRequest(zmqpp::message request) { // TODO extract to functions
  bool ret = false;
  zmqpp::message response;
  string requestName;
  request >> requestName;
  if (requestName == "get") {
    ret = true;
    transaction_t tid;
    device_t deviceId;
    time_t begin;
    time_t end;
    request >> tid >> deviceId >> begin >> end;

    list<blob> value = subscriber->handleGetRequest(tid, deviceId, begin, end);
    response.push_front(value.size());
    for(blob b : value) {
      response.push_front(&b[0], b.size());
    }
  } else if (requestName == "put") {
    ret = true;
    transaction_t tid;
    device_t deviceId;
    time_t timestamp;
    time_t expiry;
    request >> tid >> deviceId >> timestamp >> expiry;

    response << subscriber->handlePutRequest(tid, deviceId, timestamp, expiry);
  }
  serverSocket.send(response);
  return ret;
}

std::list<std::string> Client::remotePut(transaction_t tid, std::list<std::string> &recipients, device_t deviceId, time_t timestamp, time_t expiration, blob data) {
  list<string> respondents;
  zmqpp::message message;
  message << "put" << tid << deviceId << timestamp << expiration;

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

std::list<blob> Client::remoteGet(transaction_t tid, std::list<std::string> &recipients, device_t deviceId, time_t begin, time_t end) {


  return __1::list<blob, allocator<blob>>();
}

zmqpp::endpoint_t Client::buildEndpoint(std::string target, int port) {
  stringstream buf;
  buf << "tcp://" << target << ':' << port;
  return buf.str();
}