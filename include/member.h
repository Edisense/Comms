#include <future>
#include <zmqpp/context.hpp>
#include <zmqpp/socket.hpp>
#include "edisense_comms.h"
#include "edisense_types.h"
#include "client.h"

namespace edisense_comms {
  class MemberServer;
};

class edisense_comms::Member : Client {

public:

  Member();

  /*!
    Updates all nodes with the new owner of a partition asynchronously

    \param tid        a transaction ID for the change
    \param recipients a list of hostnames to broadcast the change to
    \param newOwner   the node ID for the new owner of a partition
    \param partition  the partition being reassigned
    \return a future to be fulfilled with the list of nodes that acknowledged the request
   */
  std::future<std::list<std::string>> updatePartitionOwner(
      transaction_t tid,
      std::list<std::string> recipients,
      node_t newOwner,
      partition_t partition);


  /*!
    send to recipent, which is a hostname
   */
  std::future<bool> canReceiveRequest(transaction_t tid, std::string &recipient, partition_t partition_id);

  /*!
    send to recipent, which is a hostname
   */
  std::future<bool> commitReceiveRequest(transaction_t tid, std::string &recipient, partition_t partition_id);

  /*!
    send to recipent, which is a hostname
   */
  std::future<bool> commitAsStableRequest(transaction_t tid, std::string &recipient, partition_t partition_id);

private:

  std::list<std::string> remoteUpdatePartitionOwner(transaction_t tid, std::list<std::string> recipients, node_t newOwner, partition_t partition);

  bool remoteCanReceiveRequest(transaction_t tid, std::string &recipient, partition_t partition);

  bool remoteCommitReceiveRequest(transaction_t tid, std::string &recipient, partition_t partition);

  bool remoteCommitAsStableRequest(transaction_t tid, std::string &recipient, partition_t partition);

};

class edisense_comms::MemberServer {

  virtual bool handleUpdatePartitionOwner(transaction_t tid, std::list<std::string> recipients, node_t newOwner, partition_t partition)
      = 0;

  virtual bool handleCanReceiveRequest(transaction_t tid, std::string &recipient, partition_t partition_id)
      = 0;

  virtual bool handleCommitReceiveRequest(transaction_t tid, std::string &recipient, partition_t partition_id)
      = 0;

  virtual bool handleCommitAsStableRequest(transaction_t tid, std::string &recipient, partition_t partition_id)
      = 0;
};