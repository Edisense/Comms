#include <iostream>
#include <member.h>

#include "gtest/gtest.h"


class MemberTest : public ::testing::Test, public edisense_comms::MemberServer {
public:
  bool updateCalled;
  bool canReceiveCalled;
  bool commitReceiveCalled;
  bool commitAsStableCalled;
  edisense_comms::Member * member;
protected:

  virtual void SetUp() override;

  virtual void TearDown() override;

public:
  virtual GetResult handleGetRequest(transaction_t tid, device_t deviceId, time_t begin, time_t end);

  virtual PutResult handlePutRequest(transaction_t tid, device_t deviceId, time_t timestamp, time_t expiry);

  virtual bool handleUpdatePartitionOwner(transaction_t tid, node_t newOwner, partition_t partition);

  virtual CanReceiveResult handleCanReceiveRequest(transaction_t tid, partition_t partition_id);

  virtual bool handleCommitReceiveRequest(transaction_t tid, partition_t partition_id);

  virtual bool handleCommitAsStableRequest(transaction_t tid, partition_t partition_id);
};

TEST_F(MemberTest, TestUpdatePartitionRequest)
{
  std::list<std::string> recipients;
  recipients.push_back("localhost");
  std::future<std::list<std::string>> resultsSoon = member->updatePartitionOwner(12345, recipients, 12, 123);
  std::future_status status = resultsSoon.wait_for(std::chrono::seconds(2));
  ASSERT_EQ(std::future_status::ready, status);
  std::list<std::string> results = resultsSoon.get();

  EXPECT_EQ(1, results.size());
  EXPECT_TRUE(updateCalled);
}

TEST_F(MemberTest, TestCanReceiveRequest)
{
  std::string recipient = "localhost";
  std::future<CanReceiveResult> resultsSoon = member->canReceiveRequest(12345, recipient, 123);
  std::future_status status = resultsSoon.wait_for(std::chrono::seconds(2));
  ASSERT_EQ(std::future_status::ready, status);
  CanReceiveResult result = resultsSoon.get();

  EXPECT_TRUE(result.can_recv);
  EXPECT_TRUE(canReceiveCalled);
}

TEST_F(MemberTest, TestCommitReceiveRequest) {
  std::string recipient = "localhost";
  std::future<bool> resultsSoon = member->commitReceiveRequest(12345, recipient, 123);
  std::future_status status = resultsSoon.wait_for(std::chrono::seconds(2));
  ASSERT_EQ(std::future_status::ready, status);
  bool result = resultsSoon.get();

  EXPECT_TRUE(result);
  EXPECT_TRUE(commitReceiveCalled);
}

TEST_F(MemberTest, TestCommitAsStableRequest)
{
  std::string recipient = "localhost";
  std::future<bool> resultsSoon = member->commitAsStableRequest(12345, recipient, 123);
  std::future_status status = resultsSoon.wait_for(std::chrono::seconds(2));
  ASSERT_EQ(std::future_status::ready, status);
  bool result = resultsSoon.get();

  EXPECT_TRUE(result);
  EXPECT_TRUE(commitAsStableCalled);
}

GetResult MemberTest::handleGetRequest(transaction_t tid, device_t deviceId, time_t begin, time_t end) {
  return GetResult(); // Tested in test_client.h
}

PutResult MemberTest::handlePutRequest(transaction_t tid, device_t deviceId, time_t timestamp, time_t expiry) {
  return PutResult(); // Tested in test_client.h
}

bool MemberTest::handleUpdatePartitionOwner(transaction_t tid, node_t newOwner, partition_t partition) {
  updateCalled = true;
  return true;
}

CanReceiveResult MemberTest::handleCanReceiveRequest(transaction_t tid, partition_t partition_id) {
  canReceiveCalled = true;
  CanReceiveResult result= {};
  result.can_recv = true;
  result.free = 2121212121;
  result.util = 0.5;
  return result;
}

bool MemberTest::handleCommitReceiveRequest(transaction_t tid, partition_t partition_id) {
  commitReceiveCalled = true;
  return true;
}

bool MemberTest::handleCommitAsStableRequest(transaction_t tid, partition_t partition_id) {
  commitAsStableCalled = true;
  return true;
}

void MemberTest::SetUp() {
  Test::SetUp();
  member = new edisense_comms::Member;
  member->start(this);
  updateCalled = false;
  canReceiveCalled = false;
  commitReceiveCalled = false;
  commitAsStableCalled = false;
}

void MemberTest::TearDown() {
  Test::TearDown();
  member->stop();
  delete member;
}
