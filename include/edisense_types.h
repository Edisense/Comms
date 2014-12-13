#ifndef EDISENSE_TYPES_H
#define EDISENSE_TYPES_H

#include <cstdint>
#include <ctime>
#include <list>
#include <vector>
#include <string>

// 16-bit node id
typedef uint16_t node_t;

// 16-bit sensor id
typedef uint16_t device_t;

// 16-bit range id
typedef uint16_t partition_t;

// 64-bit transaction id
typedef uint64_t transaction_t;

// Binary blob
typedef std::string blob;

const int kMaxDataLen = 20;

typedef struct Data
{
	time_t timestamp;
	time_t expiration;
	blob data;
} Data;

// for message passing

enum CallStatus
{
	SUCCESS = 0,
	DATA_MOVED = 1,
	DATA_NOT_OWNED = 2,
	DATA_MOVING = 3,
	DB_ERROR = 4
};

enum CallStatusBool
{
	RET_TRUE,
	RET_FALSE,
	RET_COMMS_FAILURE
};

typedef struct MessageId
{
	node_t node_id;
	transaction_t tid;
} MessageId;

typedef struct PutResult
{
	CallStatus status;
	node_t moved_to;
} PutResult;

typedef struct GetResult
{
	CallStatus status;
	node_t moved_to;
	std::list<Data> *values;
} GetResult;

typedef struct CanReceiveResult 
{
	bool can_recv;
	float util;
	uint64_t free;
} CanReceiveResult;

typedef struct JoinResult
{
	bool success;
	int num_replicas;
	int num_partitions;
	std::list<partition_t> partitions;
} JoinResult;

#endif /* EDISENSE_TYPES_H */
