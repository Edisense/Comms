Client
======

The Comms library is a high level abstraction of the ZeroMQ message passing protocol underlying the Edisense system.  It consists of two API classes and their associated handlers for the other end:
* `Client` - interactions with the cluster that might be done by a client
  * `put(node_t sender, transaction_t tid, std::list<std::string> &recipients,device_t deviceId, time_t timestamp, time_t expiration, blob data)`
    * Adds a blob of binary data to the cluster 
  * `get(transaction_t tid, std::list<std::string> &recipients, device_t deviceId, time_t begin, time_t end)`
    * fetches the data from the provided range 
  * `locate(device_t deviceId, std::string &recipient)`
    * returns the list of cluster members replicating data from the provided device
* `Member` - interactions with the cluster as performed by cluster members
  * `updatePartitionOwner(node_t sender,transaction_t tid,std::list<std::string> recipients,node_t newOwner,partition_t partition)`
    * Broadcasts an update to the cluster when partition ownership changes
  * `canReceiveRequest(node_t sender, transaction_t tid, std::string &recipient, partition_t partition_id)`
    * Asks a set of nodes on the cluster if they are capable of taking over ownership of a partition 
  * `commitReceiveRequest(node_t sender, transaction_t tid, std::string &recipient, partition_t partition_id)`
    * Sets the node state to receiving if it is able to accept the partition
  * `commitAsStableRequest(node_t sender, transaction_t tid, std::string &recipient, partition_t partition_id)`
    * Marks the transaction state for partition ownership as stable

-----
 
Returned values are realized as futures to allow for non-blocking IO.  Members are also clients, and so can perform the same operations as a client would be able to.  Each class also contains a corresponding handler interface.  It is expected that the endpoint listening on the server side of the connection will implement the interface to fulfill requests.  Listening is begun when the server invokes `start(<HandlerType>)` on the object:

```cpp
int main(int argc, const char *argv[]) {
  edisense_comms::Member member;
  MyServer server;
  member.start(&server);  // Begins listening for member related events; dispatches them to MyServer object
  ...
}

class MyServer : public edisense_comms::MemberServer {
...
}
```

[![Build Status](https://travis-ci.org/Edisense/Comms.svg?branch=master)](https://travis-ci.org/Edisense/Comms)
