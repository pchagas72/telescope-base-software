#ifndef PROTOCOL_H
#define PROTOCOL_H

#define PACKET_TYPE_PING 0x01
#define PACKET_TYPE_CMD  0x02
#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t type;
    uint32_t packet_id;
    uint64_t timestamp;
    char payload[32];
} Message_Struct;

#endif
