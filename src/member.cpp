#include <edisense_types.h>
#include <future>
#include <zmqpp/zmqpp.hpp>
#include <sstream>
#include "member.h"

using namespace edisense_comms;
using namespace std;

Member::Member()
{

}

future<list<string>> Member::updatePartitionOwner(transaction_t tid, list<string> recipients, node_t newOwner, partition_t partition) {
  return async(launch::async, &Member::remoteUpdatePartitionOwner, this, tid, recipients, newOwner, partition);
}

std::future<bool> Member::canReceiveRequest(transaction_t tid, string &recipient, partition_t partition_id) {
  return async(launch::async, &Member::remoteCanReceiveRequest, this, tid, recipient, partition_id);
}

std::future<bool> Member::commitReceiveRequest(transaction_t tid, string &recipient, partition_t partition_id) {
  return async(launch::async, &Member::remoteCommitReceiveRequest, this, tid, recipient, partition_id);
}

std::future<bool> Member::commitAsStableRequest(transaction_t tid, string &recipient, partition_t partition_id) {
  return async(launch::async, &Member::remoteCommitAsStableRequest, this, tid, recipient, partition_id);
}

list<string> Member::remoteUpdatePartitionOwner(transaction_t tid, list<string> recipients, node_t newOwner, partition_t partition) {
  list<string> respondents;
  zmqpp::message message;
  message << "updatePartitionOwner" << tid << newOwner << partition;

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

bool Member::remoteCanReceiveRequest(transaction_t tid, string &recipient, partition_t partition) {
  zmqpp::endpoint_t endpoint = buildEndpoint(recipient, SERVER_SOCKET_PORT);
  zmqpp::message message;
  message << "canReceiveRequest" << tid << partition;
  zmqpp::message response;
  clientSocket.bind(endpoint);
  clientSocket.send(message);

  clientSocket.receive(response);
  clientSocket.unbind(endpoint);

  bool allGood;
  response >> allGood;
  return allGood;
}

bool Member::remoteCommitReceiveRequest(transaction_t tid, string &recipient, partition_t partition) {
  zmqpp::endpoint_t endpoint = buildEndpoint(recipient, SERVER_SOCKET_PORT);
  zmqpp::message message;
  message << "commitReceiveRequest" << tid << partition;
  zmqpp::message response;
  clientSocket.bind(endpoint);
  clientSocket.send(message);

  clientSocket.receive(response);
  clientSocket.unbind(endpoint);

  bool allGood;
  response >> allGood;
  return allGood;
}

bool Member::remoteCommitAsStableRequest(transaction_t tid, string &recipient, partition_t partition) {
  zmqpp::endpoint_t endpoint = buildEndpoint(recipient, SERVER_SOCKET_PORT);
  zmqpp::message message;
  message << "commitAsStableRequest" << tid << partition;
  zmqpp::message response;
  clientSocket.bind(endpoint);
  clientSocket.send(message);

  clientSocket.receive(response);
  clientSocket.unbind(endpoint);

  bool allGood;
  response >> allGood;
  return allGood;
}

