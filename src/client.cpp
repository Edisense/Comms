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

future<list<pair<string, PutResult>>> Client::put(node_t sender, transaction_t tid, list<string> &recipients, device_t deviceId, time_t timestamp, time_t expiration, blob data) {
  return async(launch::async, &Client::remotePut, this, sender, tid, std::ref(recipients), deviceId, timestamp, expiration, data);
}

future<list<GetResult>> Client::get(transaction_t tid, list<string> &recipients, device_t deviceId, time_t begin, time_t end) {
  return async(launch::async, &Client::remoteGet, this, tid, std::ref(recipients), deviceId, begin, end);
}

future<list<string>> Client::locate(device_t deviceId, string &recipient)
{
  return async(launch::async, &Client::remoteLocate, this, deviceId, recipient);
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
      response.add(data.data);
      response.add((uint32_t) data.expiration);
      response.add((uint32_t) data.timestamp);
    }
    serverSocket->send(response);
    delete getResult.values;
  } else if (topic == "put") {
    wasRequestProcessed = true;
    node_t sender;
    transaction_t tid;
    device_t deviceId;
    uint32_t timestamp;
    uint32_t expiry;
    blob data;    

    request >> sender >> tid >> deviceId >> timestamp >> expiry;
    request >> data;

    cout << sender << " [sender] " << tid << " [tid] " << deviceId << " [did] "  << timestamp << " [time] "
        << expiry << " [expire] " << data << endl;

    PutResult result = subscriber->handlePutRequest(sender, tid, deviceId, timestamp, expiry, data);
    response.add((uint8_t) result.status);
    response.add(result.moved_to);
    serverSocket->send(response);
  } else if (topic == "locate") {
    wasRequestProcessed = true;
    device_t deviceId;
    request >> deviceId;
    list<string> *result = subscriber->handleLocateRequest(deviceId);
    response.add((uint32_t) result->size());
    for (string &s : *result)
      response.add(s);
    delete result;
  }
  return wasRequestProcessed;
}

list<pair<string, PutResult>> Client::remotePut(node_t sender, transaction_t tid, list<string> &recipients, device_t deviceId, time_t timestamp, time_t expiration, blob data) {
  list<pair<string, PutResult>> respondents;

  vector<zmqpp::socket *> open_sockets;
  vector<zmqpp::endpoint_t> open_endpoints;

  for(string &node : recipients) 
  {
    zmqpp::message message;
    message << "put" << sender << tid << deviceId << (uint32_t) timestamp << (uint32_t) expiration;
    message << data;

    zmqpp::endpoint_t endpoint = buildEndpoint(node, SERVER_SOCKET_PORT);
    printf("%s\n", endpoint.c_str());

    zmqpp::socket *socket = buildClientSocket();
    open_sockets.push_back(socket);
    open_endpoints.push_back(endpoint);
    
    try
    {
      socket->connect(endpoint);
      socket->send(message);
    }
    catch(zmqpp::exception e)
    {
      cerr << e.what() << endl;
    }
  }

  int i = 0;
  for(string &node : recipients) 
  {
    zmqpp::socket *socket = open_sockets[i];
    zmqpp::message response;

    try 
    {
      socket->receive(response);
      uint8_t tmpStatus;
      node_t movedTo;
      response >> tmpStatus >> movedTo;
      PutResult result = {};
      result.status = (CallStatus) tmpStatus;
      result.moved_to = movedTo;

      pair<string, PutResult> respondent(node, result);
      respondents.push_back(respondent);
      socket->disconnect(open_endpoints[i]);
    }
    catch (zmqpp::exception e)
    {
      cerr << e.what() << endl;
    }

    try
    {
      socket->close();
    }
    catch (zmqpp::exception e)
    {
      cerr << e.what() << endl;
    }

    i++;
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
      list<Data> *results = new list<Data>;

      response >> tmpStatus;
      status = (CallStatus) tmpStatus;
      response >> movedNode;
      response >> dataCount;

      string point;
      uint32_t timestamp;
      uint32_t expiry;

      for (int i = 0; i < dataCount; i++) {
        
        response >> point;
        response >> timestamp;
        response >> expiry;

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

list<string> Client::remoteLocate(device_t deviceId, string hostname)
{
  zmqpp::message message;
  message << "locate" << deviceId;

  zmqpp::socket *socket = buildClientSocket();
  zmqpp::endpoint_t endpoint = buildEndpoint(hostname, SERVER_SOCKET_PORT);

  list<string> result;
  try
  {
    socket->connect(endpoint);
    socket->send(message);

    zmqpp::message response;
    socket->receive(response);
    int count;
    response >> count;

    for (int i = 0; i < count; i++) {
      string host;
      response >> host;
      result.push_back(host);
    }
    socket->disconnect(endpoint);
  }
  catch (zmqpp::exception e)
  {
    cerr << e.what() << endl;
  }

  try 
  {
    socket->close();
  }
  catch (zmqpp::exception e)
  {
    cerr << e.what() << endl;
  }

  return result;
}

std::string Client::buildEndpoint(std::string target, int port) {
  stringstream buf;
  buf << "tcp://" << target << ':' << port;
  return buf.str();
}

zmqpp::socket *Client::buildClientSocket()
{
  zmqpp::socket *socket = new zmqpp::socket(*context, zmqpp::socket_type::request);
  socket->set(zmqpp::socket_option::receive_timeout, 1000);
  socket->set(zmqpp::socket_option::send_timeout, 1000);
  return socket;
}
