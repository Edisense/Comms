#include <iostream>
#include <client.h>
#include "gtest/gtest.h"

class ClientTest : public ::testing::Test, public edisense_comms::ClientServer {

public:
  edisense_comms::Client *client;

protected:


  virtual void SetUp() override;

  virtual void TearDown() override;

public:
  virtual GetResult handleGetRequest(transaction_t tid, device_t deviceId, time_t begin, time_t end);

  virtual PutResult handlePutRequest(node_t sender, transaction_t tid, device_t deviceId, time_t timestamp, time_t expiry, blob data);
};

TEST_F(ClientTest, RunGetRequest) {
  std::list<std::string> recipients;
  recipients.push_back("localhost");
  std::future<std::list<GetResult>> resultsSoon = client->get(12345, recipients, 12, 1000, 2000);
  std::future_status status = resultsSoon.wait_for(std::chrono::seconds(2));
  ASSERT_EQ(std::future_status::ready, status);
  std::list<GetResult> results = resultsSoon.get();
  EXPECT_EQ(1, results.size());
}

TEST_F(ClientTest, RunPutRequest) {
  std::list<std::string> recipients;
  recipients.push_back("localhost");
  blob data;
  data.push_back('a');
  std::future<std::list<std::pair<std::string,PutResult>>> resultsSoon = client->put(1, 12345, recipients, 12, 1001, 5000, data);
  std::future_status status = resultsSoon.wait_for(std::chrono::seconds(2));
  ASSERT_EQ(std::future_status::ready, status);
  std::list<std::pair<std::string,PutResult>> results = resultsSoon.get();
  EXPECT_EQ(1, results.size());
}

GetResult ClientTest::handleGetRequest(transaction_t tid, device_t deviceId, time_t begin, time_t end) {
  std::list<Data> * values = new std::list<Data>;
  GetResult result;
  result.moved_to = 0;
  result.status = CallStatus::SUCCESS;
  result.values = values;
  return result;
}

PutResult ClientTest::handlePutRequest(node_t sender, transaction_t tid, device_t deviceId, time_t timestamp, time_t expiry, blob data) {
  PutResult result;
  result.moved_to = 0;
  result.status = CallStatus::SUCCESS;
  return result;
}

void ClientTest::SetUp() {
  Test::SetUp();
  client = new edisense_comms::Client();
  client->start(this);
}

void ClientTest::TearDown() {
  Test::TearDown();
  client->stop();
  delete client;
}

