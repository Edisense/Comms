#ifndef EDISENSE_TYPES_H
#define EDISENSE_TYPES_H

#include <cstdint>
#include <ctime>
#include <list>
#include <vector>

// 16-bit node id
typedef uint16_t node_t;

// 16-bit sensor id
typedef uint16_t device_t;

// 16-bit range id
typedef uint16_t partition_t;

// 64-bit transaction id
typedef uint64_t transaction_t;

// Binary blob
typedef typename std::vector<unsigned char> blob;

const int kMaxDataLen = 20;

typedef struct Data
{
	time_t timestamp;
	time_t expiration;
	blob data;
	size_t datalen;
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

typedef struct GetPartitionTableResult
{
	bool success;
	int num_partitions;
	int num_replicas;
	node_t *partition_table; // DO NOT WRITE TO OR FREE THIS!!!
} GetPartitionTableResult;

#endif /* EDISENSE_TYPES_H */