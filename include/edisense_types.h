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

#endif /* EDISENSE_TYPES_H */