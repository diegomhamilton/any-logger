#ifndef CHANNEL_H
#define CHANNEL_H

#include <stdint.h>

typedef uint8_t channel_id_t;

#define INDEX_OF(CHANNEL_ID)    ( (CHANNEL_ID) - 1 )

#define base_channel_fields                                                                                   \
    /* ID of Channel (set by the Logger), represented by (bit position + 1) in logger.registered_channels. */ \
    channel_id_t id;                                                                                               \
    /* Name of channel. Used by the Logger to write to channel's specific destination. */                     \
    char *name

typedef struct base_channel
{
    base_channel_fields;
} base_channel_t;

#endif
