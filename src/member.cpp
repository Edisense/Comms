#include <edisense_types.h>
#include <future>
#include <zmqpp/zmqpp.hpp>
#include <sstream>
#include "member.h"

#define MSG_UPDATE_PARTITION_OWNER "updatePartitionOwner"
#define MSG_CAN_RECEIVE_REQUEST "canReceiveRequest"
#define MSG_COMMIT_RECEIVE_REQUEST "commitReceiveRequest"
#define MSG_COMMIT_AS_STABLE_REQUEST "commitAsStableRequest"


using namespace edisense_comms;
using namespace std;

Member::Member() {
}

void Member::start(MemberServer*  handler) {
  memberHandler = handler;
  Client::start(handler);
}

future<list<string>> Member::updatePartitionOwner(transaction_t tid, list<string> recipients, node_t newOwner, partition_t partition) {
  return async(launch::async, &Member::remoteUpdatePartitionOwner, this, tid, recipients, newOwner, partition);
}

std::future<CanReceiveResult> Member::canReceiveRequest(transaction_t tid, string &recipient, partition_t partition_id) {
  return async(launch::async, &Member::remoteCanReceiveRequest, this, tid, ref(recipient), partition_id);
}

std::future<bool> Member::commitReceiveRequest(transaction_t tid, string &recipient, partition_t partition_id) {
  return async(launch::async, &Member::remoteCommitReceiveRequest, this, tid, ref(recipient), partition_id);
}

std::future<bool> Member::commitAsStableRequest(transaction_t tid, string &recipient, partition_t partition_id) {
  return async(launch::async, &Member::remoteCommitAsStableRequest, this, tid, ref(recipient), partition_id);
}

list<string> Member::remoteUpdatePartitionOwner(transaction_t tid, list<string> recipients, node_t newOwner, partition_t partition) {
  list<string> respondents;
  zmqpp::message message;
  message << MSG_UPDATE_PARTITION_OWNER << tid << newOwner << partition;

  for(string node : recipients) {
    zmqpp::endpoint_t endpoint = buildEndpoint(node, SERVER_SOCKET_PORT);
    zmqpp::message response;
    clientSocket->connect(endpoint);
    clientSocket->send(message);

    // TODO I think this will actually block until we get a response from everyone - introduce a timeout
    clientSocket->receive(response);
    bool allGood;
    response >> allGood;
    if (allGood) {
      respondents.push_back(node);
    }
    clientSocket->disconnect(endpoint);
  }
  return respondents;
}

CanReceiveResult Member::remoteCanReceiveRequest(transaction_t tid, string &recipient, partition_t partition) {
  zmqpp::endpoint_t endpoint = buildEndpoint(recipient, SERVER_SOCKET_PORT);
  zmqpp::message message;
  message << MSG_CAN_RECEIVE_REQUEST << tid << partition;
  zmqpp::message response;
  clientSocket->connect(endpoint);
  clientSocket->send(message);

  clientSocket->receive(response);
  clientSocket->disconnect(endpoint);

  bool canReceive;
  uint64_t free;
  float util;

  CanReceiveResult result= {};

  response >> result.can_recv >> result.free >> result.util;

  return result;
}

bool Member::remoteCommitReceiveRequest(transaction_t tid, string &recipient, partition_t partition) {
  zmqpp::endpoint_t endpoint = buildEndpoint(recipient, SERVER_SOCKET_PORT);
  zmqpp::message message;
  message << MSG_COMMIT_RECEIVE_REQUEST << tid << partition;
  zmqpp::message response;
  clientSocket->connect(endpoint);
  clientSocket->send(message);

  clientSocket->receive(response);
  clientSocket->disconnect(endpoint);

  bool allGood;
  response >> allGood;
  return allGood;
}

bool Member::remoteCommitAsStableRequest(transaction_t tid, string &recipient, partition_t partition) {
  zmqpp::endpoint_t endpoint = buildEndpoint(recipient, SERVER_SOCKET_PORT);
  zmqpp::message message;
  message << MSG_COMMIT_AS_STABLE_REQUEST << tid << partition;
  zmqpp::message response;
  clientSocket->connect(endpoint);
  clientSocket->send(message);

  clientSocket->receive(response);
  clientSocket->disconnect(endpoint);

  bool allGood;
  response >> allGood;
  return allGood;
}

bool Member::dispositionRequest(string topic, zmqpp::message &message) {
  bool wasRequestProcessed = Client::dispositionRequest(topic, message);
  if (!wasRequestProcessed) {
    if      ((wasRequestProcessed = (topic == MSG_UPDATE_PARTITION_OWNER  ))) handleUpdatePartitionOwner(&message);
    else if ((wasRequestProcessed = (topic == MSG_CAN_RECEIVE_REQUEST     ))) handleCanReceiveRequest(&message);
    else if ((wasRequestProcessed = (topic == MSG_COMMIT_RECEIVE_REQUEST  ))) handleCommitReceiveRequest(&message);
    else if ((wasRequestProcessed = (topic == MSG_COMMIT_AS_STABLE_REQUEST))) handleCommitAsStableRequest(&message);
  }
  return wasRequestProcessed;
}

void Member::handleUpdatePartitionOwner(zmqpp::message message) {
  zmqpp::message response;
  transaction_t tid;
  node_t owner;
  partition_t partition;
  message >> tid >> owner >> partition;

  bool allGood = memberHandler->handleUpdatePartitionOwner(tid, owner, partition);
  response << allGood;
  serverSocket->send(response);
}

void Member::handleCanReceiveRequest(zmqpp::message message) {
  zmqpp::message response;
  transaction_t tid;
  partition_t partition;
  message >> tid >> partition;

  CanReceiveResult result = memberHandler->handleCanReceiveRequest(tid, partition);
  response << result.can_recv << result.free << result.util;
  serverSocket->send(response);
}

void Member::handleCommitReceiveRequest(zmqpp::message message) {
  zmqpp::message response;
  transaction_t tid;
  partition_t partition;
  message >> tid >> partition;

  bool allGood = memberHandler->handleCommitReceiveRequest(tid, partition);
  response << allGood;
  serverSocket->send(response);
}

void Member::handleCommitAsStableRequest(zmqpp::message message) {
  zmqpp::message response;
  transaction_t tid;
  partition_t partition;
  message >> tid >> partition;

  bool allGood = memberHandler->handleCommitAsStableRequest(tid, partition);
  response << allGood;
  serverSocket->send(response);
}
