#ifndef EDISENSE_TYPES_H
#define EDISENSE_TYPES_H

#include <cstdint>
#include <ctime>
#include <list>

//TODO Choose appropriate length
#define HOST_NAME_MAX 40

// 16-bit node id
typedef uint16_t node_t;

// 16-bit sensor id
typedef uint16_t device_t;

// 16-bit range id
typedef uint16_t partition_t;

// 64-bit transaction id
typedef uint64_t transaction_t;

typedef struct {
  node_t node;
  std::list<int> range; // TODO I don't know how we want to represent ranges?
} Ownership;

#endif /* EDISENSE_TYPES_H */