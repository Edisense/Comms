#include <edisense_types.h>
#include <future>
#include <zmqpp/zmqpp.hpp>
#include <sstream>
#include "member.h"

#define MSG_UPDATE_PARTITION_OWNER "updatePartitionOwner"
#define MSG_CAN_RECEIVE_REQUEST "canReceiveRequest"
#define MSG_COMMIT_RECEIVE_REQUEST "commitReceiveRequest"
#define MSG_COMMIT_AS_STABLE_REQUEST "commitAsStableRequest"
#define MSG_JOIN_REQUEST "joinRequest"

using namespace edisense_comms;
using namespace std;

Member::Member() {
}

void Member::start(MemberServer*  handler) {
  memberHandler = handler;
  Client::start(handler);
}

future<list<string>> Member::updatePartitionOwner(node_t sender, transaction_t tid, list<string> recipients, node_t newOwner, partition_t partition) {
  return async(launch::async, &Member::remoteUpdatePartitionOwner, this, sender, tid, recipients, newOwner, partition);
}

std::future<CanReceiveResult> Member::canReceiveRequest(node_t sender, transaction_t tid, string &recipient, partition_t partition_id) {
  return async(launch::async, &Member::remoteCanReceiveRequest, this, sender, tid, ref(recipient), partition_id);
}

std::future<CallStatusBool> Member::commitReceiveRequest(node_t sender, transaction_t tid, string &recipient, partition_t partition_id) {
  return async(launch::async, &Member::remoteCommitReceiveRequest, this, sender,  tid, ref(recipient), partition_id);
}

std::future<bool> Member::commitAsStableRequest(node_t sender, transaction_t tid, string &recipient, partition_t partition_id) {
  return async(launch::async, &Member::remoteCommitAsStableRequest, this, sender,  tid, ref(recipient), partition_id);
}

std::future<JoinResult> Member::sendJoinRequest(node_t sender, transaction_t tid, string &recipient, string &new_owner) {
  return async(launch::async, &Member::remoteJoinRequest, this, sender,  tid, ref(recipient), ref(new_owner));
}

list<string> Member::remoteUpdatePartitionOwner(node_t sender, transaction_t tid, list<string> recipients, node_t newOwner, partition_t partition) {
  list<string> non_respondents;

  vector<zmqpp::socket *> open_sockets;
  vector<zmqpp::endpoint_t> open_endpoints;

  for(string &node : recipients) {
    zmqpp::message message;
    message << MSG_UPDATE_PARTITION_OWNER << sender << tid << newOwner << partition;

    zmqpp::endpoint_t endpoint = buildEndpoint(node, SERVER_SOCKET_PORT);

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
  for(string &node : recipients) {
    zmqpp::message response;  
    zmqpp::socket *socket = open_sockets[i];
    bool allGood = false;
    try
    {
      socket->receive(response);
      response >> allGood;
      socket->disconnect(open_endpoints[i]);
    }
    catch(zmqpp::exception e)
    {
      cerr << e.what() << endl;
    }
    if (!allGood) {
        non_respondents.push_back(node);
    }

    try 
    {
      socket->close();
      delete socket;
    } 
    catch (zmqpp::exception e)
    {
      cerr << e.what() << endl;
    }
    i++;
  }

  return non_respondents;
}

CanReceiveResult Member::remoteCanReceiveRequest(node_t sender, transaction_t tid, string &recipient, partition_t partition) {
  zmqpp::endpoint_t endpoint = buildEndpoint(recipient, SERVER_SOCKET_PORT);
  zmqpp::message message;
  message << MSG_CAN_RECEIVE_REQUEST << sender << tid << partition;
  zmqpp::message response;
  CanReceiveResult result;
  result.can_recv = false;

  zmqpp::socket *socket = buildClientSocket();

  try
  {
    socket->connect(endpoint);
    socket->send(message);

    socket->receive(response);
    response >> result.can_recv >> result.free >> result.util;
    socket->disconnect(endpoint);
  }
  catch (zmqpp::exception e)
  {
    cerr << e.what() << endl;
  }

  try 
  {
    socket->close();
    delete socket;
  }
  catch (zmqpp::exception e)
  {
    cerr << e.what() << endl;
  }

  return result;
}

CallStatusBool Member::remoteCommitReceiveRequest(node_t sender, transaction_t tid, string &recipient, partition_t partition) {
  zmqpp::endpoint_t endpoint = buildEndpoint(recipient, SERVER_SOCKET_PORT);
  zmqpp::message message;
  message << MSG_COMMIT_RECEIVE_REQUEST << sender << tid << partition;
  zmqpp::message response;

  zmqpp::socket *socket = buildClientSocket();

  bool allGood = false;
  bool commsFailure = false;
  try
  {
    socket->connect(endpoint);
    socket->send(message);

    socket->receive(response);
    response >> allGood;
    socket->disconnect(endpoint);
  }
  catch (zmqpp::exception e)
  {
    commsFailure = true;
    cerr << e.what() << endl;
  }
  
  try 
  {
    socket->close();
    delete socket;
  }
  catch (zmqpp::exception e)
  {
    cerr << e.what() << endl;
  }

  if (commsFailure)
    return RET_COMMS_FAILURE;

  return allGood ? RET_TRUE : RET_FALSE;
}

bool Member::remoteCommitAsStableRequest(node_t sender, transaction_t tid, string &recipient, partition_t partition) {
  zmqpp::endpoint_t endpoint = buildEndpoint(recipient, SERVER_SOCKET_PORT);
  zmqpp::message message;
  message << MSG_COMMIT_AS_STABLE_REQUEST << sender << tid << partition;
  zmqpp::message response;
  
  zmqpp::socket *socket = buildClientSocket();

  bool allGood = false;

  try
  {
    socket->connect(endpoint);
    socket->send(message);

    socket->receive(response);
    response >> allGood;
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

  return allGood;
}

JoinResult Member::remoteJoinRequest(node_t sender, transaction_t tid, string &recipient, string &new_node)
{
  zmqpp::endpoint_t endpoint = buildEndpoint(recipient, SERVER_SOCKET_PORT);
  zmqpp::message message;
  message << MSG_JOIN_REQUEST << sender << tid << new_node;
  zmqpp::message response;
  
  zmqpp::socket *socket = buildClientSocket();
  JoinResult result;
  try
  {
    socket->connect(endpoint);
    socket->send(message);

    socket->receive(response);
    int count;
    response >> result.success;
    response >> result.num_partitions;
    response >> result.num_replicas;
    response >> count;
    for (int i = 0; i < count; i++) {
      partition_t pid;
      response >> pid;
      result.partitions.push_back(pid);
    }
    socket->disconnect(endpoint);
  }
  catch (zmqpp::exception e)
  {
    result.success = false;
    cerr << e.what() << endl;
  }
  
  try 
  {
    socket->close();
    delete socket;
  }
  catch (zmqpp::exception e)
  {
    cerr << e.what() << endl;
  }
  return result;
}

bool Member::dispositionRequest(string topic, zmqpp::message &message) {
  bool wasRequestProcessed = Client::dispositionRequest(topic, message);
  if (!wasRequestProcessed) {
    if      ((wasRequestProcessed = (topic == MSG_UPDATE_PARTITION_OWNER  ))) handleUpdatePartitionOwner(message);
    else if ((wasRequestProcessed = (topic == MSG_CAN_RECEIVE_REQUEST     ))) handleCanReceiveRequest(message);
    else if ((wasRequestProcessed = (topic == MSG_COMMIT_RECEIVE_REQUEST  ))) handleCommitReceiveRequest(message);
    else if ((wasRequestProcessed = (topic == MSG_COMMIT_AS_STABLE_REQUEST))) handleCommitAsStableRequest(message);
    else if ((wasRequestProcessed = (topic == MSG_JOIN_REQUEST            ))) handleJoinRequest(message);
  }
  return wasRequestProcessed;
}

void Member::handleUpdatePartitionOwner(zmqpp::message &message) {
  zmqpp::message response;
  node_t sender;
  transaction_t tid;
  node_t owner;
  partition_t partition;
  message >> sender >> tid >> owner >> partition;

  bool allGood = memberHandler->handleUpdatePartitionOwner(sender, tid, owner, partition);
  response << allGood;
  serverSocket->send(response);
}

void Member::handleCanReceiveRequest(zmqpp::message &message) {
  zmqpp::message response;
  node_t sender;
  transaction_t tid;
  partition_t partition;
  message >> sender >> tid >> partition;

  CanReceiveResult result = memberHandler->handleCanReceiveRequest(sender, tid, partition);
  response << result.can_recv << result.free << result.util;
  serverSocket->send(response);
}

void Member::handleCommitReceiveRequest(zmqpp::message &message) {
  zmqpp::message response;
  node_t sender;
  transaction_t tid;
  partition_t partition;
  message >> sender >> tid >> partition;

  bool allGood = memberHandler->handleCommitReceiveRequest(sender, tid, partition);
  response << allGood;
  serverSocket->send(response);
}

void Member::handleCommitAsStableRequest(zmqpp::message &message) {
  zmqpp::message response;
  node_t sender;
  transaction_t tid;
  partition_t partition;
  message >> sender >> tid >> partition;

  bool allGood = memberHandler->handleCommitAsStableRequest(sender, tid, partition);
  response << allGood;
  serverSocket->send(response);
}

void Member::handleJoinRequest(zmqpp::message &message)
{
  zmqpp::message response;
  node_t sender;
  transaction_t tid;
  string sender_hostname;
  message >> sender >> tid >> sender_hostname;

  JoinResult result = memberHandler->handleJoinRequest(sender, tid, sender_hostname);
  response << result.success << result.num_partitions << result.num_replicas;
  int count = result.partitions.size();
  response << count;
  for (partition_t pid : result.partitions)
    response << pid;
  serverSocket->send(response);
}
