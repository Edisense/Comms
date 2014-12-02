#ifndef EDISENSE_TYPES_H
#define EDISENSE_TYPES_H

#include <cstdint>
#include <ctime>
#include <list>

// 16-bit node id
typedef uint16_t node_t;

// 16-bit sensor id
typedef uint16_t device_t;

// 16-bit range id
typedef uint16_t partition_t;

// 64-bit transaction id
typedef uint64_t transaction_t;

const int kMaxDataLen = 20;

typedef struct Data
{
	time_t timestamp;
	time_t expiration;
	char data[kMaxDataLen];
	size_t datalen;
} Data;

// for message passing

enum ErrorType
{
	DATA_MOVED = 0,
	DATA_NOT_OWNED = 1,
	DATA_MOVING = 2,
	DB_ERROR = 3
};

typedef struct MessageId
{
	node_t node_id;
	transaction_t tid;
} MessageId;

typedef struct PutResult
{
	bool success;
	ErrorType error;
	node_t moved_to;
} PutResult;

typedef struct CanReceiveResult 
{
	bool can_recv;
	float util;
	size_t free;
} CanReceiveResult;

typedef struct GetPartitionTableResult
{
	bool success;
	int num_partitions;
	int num_replicas;
	node_t *partition_table; // DO NOT WRITE TO OR FREE THIS!!!
} GetPartitionTableResult;

#endif /* EDISENSE_TYPES_H */