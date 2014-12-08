#include <iostream>
#include <member.h>

#include "gtest/gtest.h"


class MemberTest : public ::testing::Test, public edisense_comms::MemberServer {
public:

protected:
  virtual void SetUp() {

  }

public:
  virtual GetResult handleGetRequest(transaction_t tid, device_t deviceId, time_t begin, time_t end);

  virtual PutResult handlePutRequest(transaction_t tid, device_t deviceId, time_t timestamp, time_t expiry);

  virtual bool handleUpdatePartitionOwner(transaction_t tid, node_t newOwner, partition_t partition);

  virtual CanReceiveResult handleCanReceiveRequest(transaction_t tid, partition_t partition_id);

  virtual bool handleCommitReceiveRequest(transaction_t tid, partition_t partition_id);

  virtual bool handleCommitAsStableRequest(transaction_t tid, partition_t partition_id);
};

TEST_F(MemberTest, DefaultTest)
{

  EXPECT_EQ(1, 1);
}

GetResult MemberTest::handleGetRequest(transaction_t tid, device_t deviceId, time_t begin, time_t end) {
  return GetResult(); // Tested in test_client.h
}

PutResult MemberTest::handlePutRequest(transaction_t tid, device_t deviceId, time_t timestamp, time_t expiry) {
  return PutResult(); // Tested in test_client.h
}

bool MemberTest::handleUpdatePartitionOwner(transaction_t tid, node_t newOwner, partition_t partition) {
  return false;
}

CanReceiveResult MemberTest::handleCanReceiveRequest(transaction_t tid, partition_t partition_id) {
  return CanReceiveResult();
}

bool MemberTest::handleCommitReceiveRequest(transaction_t tid, partition_t partition_id) {
  return false;
}

bool MemberTest::handleCommitAsStableRequest(transaction_t tid, partition_t partition_id) {
  return false;
}
