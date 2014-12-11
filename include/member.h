#ifndef MEMBER_H
#define MEMBER_H

#include <future>
#include "edisense_comms.h"
#include "edisense_types.h"
#include "client.h"


namespace edisense_comms {
  class MemberServer;
};

class edisense_comms::Member : public Client {
  MemberServer *memberHandler;
public:

  Member();

  virtual void start(MemberServer *handler);

  /*!
    Updates all nodes with the new owner of a partition asynchronously

    \param tid        a transaction ID for the change
    \param recipients a list of hostnames to broadcast the change to
    \param newOwner   the node ID for the new owner of a partition
    \param partition  the partition being reassigned
    \return a future to be fulfilled with the list of nodes that acknowledged the request
   */
  std::future<std::list<std::string>> updatePartitionOwner(
      node_t sender,
      transaction_t tid,
      std::list<std::string> recipients,
      node_t newOwner,
      partition_t partition);


  /*!
    send to recipent, which is a hostname
   */
  std::future<CanReceiveResult> canReceiveRequest(node_t sender, transaction_t tid, std::string &recipient, partition_t partition_id);

  /*!
    send to recipent, which is a hostname
   */
  std::future<bool> commitReceiveRequest(node_t sender, transaction_t tid, std::string &recipient, partition_t partition_id);

  /*!
    send to recipent, which is a hostname
   */
  std::future<bool> commitAsStableRequest(node_t sender, transaction_t tid, std::string &recipient, partition_t partition_id);

  /*!
    send to recipent, which is a hostname
  */
  std::future<JoinResult> sendJoinRequest(node_t sender, transaction_t tid, std::string &recipient, std::string &new_member);

protected:

  virtual bool dispositionRequest(std::string topic, zmqpp::message &message);

private:

  std::list<std::string> remoteUpdatePartitionOwner(node_t sender, transaction_t tid, std::list<std::string> recipients, node_t newOwner, partition_t partition);

  CanReceiveResult remoteCanReceiveRequest(node_t sender, transaction_t tid, std::string &recipient, partition_t partition);

  bool remoteCommitReceiveRequest(node_t sender, transaction_t tid, std::string &recipient, partition_t partition);

  bool remoteCommitAsStableRequest(node_t sender, transaction_t tid, std::string &recipient, partition_t partition);

  JoinResult remoteJoinRequest(node_t sender, transaction_t tid, std::string &recipient, std::string &new_node);

  void handleUpdatePartitionOwner(zmqpp::message &message);

  void handleCanReceiveRequest(zmqpp::message &message);

  void handleCommitReceiveRequest(zmqpp::message &message);

  void handleCommitAsStableRequest(zmqpp::message &message);

  void handleJoinRequest(zmqpp::message &message);
};

class edisense_comms::MemberServer : public ClientServer {
public:
  virtual bool handleUpdatePartitionOwner(node_t sender, transaction_t tid, node_t newOwner, partition_t partition)
      = 0;

  virtual CanReceiveResult handleCanReceiveRequest(node_t sender, transaction_t tid, partition_t partition_id)
      = 0;

  virtual bool handleCommitReceiveRequest(node_t sender, transaction_t tid, partition_t partition_id)
      = 0;

  virtual bool handleCommitAsStableRequest(node_t sender, transaction_t tid, partition_t partition_id)
      = 0;

  virtual JoinResult handleJoinRequest(node_t sender, transaction_t tid, std::string &new_node)
      = 0;
};

#endif /* MEMBER_H */