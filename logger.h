#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include "buffer.h"
#include "channel.h"

typedef enum logger_status
{
    UNINITIALIZED = 0,
    IDLE,
    RUNNING
} logger_status_t;

typedef enum operation_result
{
    SUCCESS = 0,
    FAIL
} op_res_t;

typedef void (*signal_cb_t)(uint8_t *data);

/* Defines the size of write buffer used by the Logger */
#define WRITE_BUFFER_SIZE 16

typedef struct data
{
    /* Data buffer to by written by the logger */
    uint8_t *data;
    /* Bytes of data buffer to be written */
    uint16_t size;
    /* ID of channel that is requesting the write operation */
    channel_id_t id;
    /* Callback to be called on write operation */
    signal_cb_t write_cb;
} data_t;

typedef BUFFER_STRUCT_DEF(data_t, uint8_t, WRITE_BUFFER_SIZE) data_buffer_t;

#define base_logger_functions                                                \
    /* Initialize logger. Must set the logger to IDLE if succesful.          \
     * Example: If implementing an SD logger, this function must return      \
     * SUCCESS if init succeeds correctly, FAILED otherwise.                 \
     *                                                                       \
     * Note: Must not be NULL                                                \
     * */                                                                    \
    op_res_t (*_init)(void);                                                 \
    /* Start the logger defining its destination. This string identifier     \
     * can be used to identify the logger, e.g., name of the folder or       \
     * overhead of a message, depending on the logger stream used.           \
     * */                                                                    \
    void (*_start)(char *destination);                                       \
    /* This function can be used to perform operations desired when the      \
     * logger stops, e.g. disabling some piece of hardware. Set to NULL      \
     * if no additional operation is required. */                            \
    void (*_stop)(void);                                                     \
    /* Provide this function with the hardware implementation to write a     \
     * data object to the desired destination according to the channel id    \
     * and the logger specified. Must return SUCCESS if write is succesful   \
     *                                                                       \
     * Note: Must not be NULL */                                             \
    op_res_t (*_write)(data_t * data);                                       \
    /* Provide this function with the wait implementation on the platform    \
     * used by the application                                               \
     *                                                                       \
     * Note: Must not be NULL */                                             \
    void (*_wait)(void);                                                     \
    /* Provide this function with the signal implementation on the platform  \
     * used by the application. This must signal the _wait function provided \
     *                                                                       \
     * Note: Must not be NULL */                                             \
    void (*_signal)(void)

#define base_logger_attributes                                               \
    logger_status_t status;                                                  \
    /* Each bit represents a channel. If a bit at BIT_POS is equal to 1,     \
     * there is a channel registered with ID equal to BIT_POS. */            \
    uint64_t registered_channels;                                            \
    /* Array of channels. The index of each channel correponds to its ID. */ \
    base_channel_t **channels;                                               \
    /* Max. number of channels supported. */                                 \
    uint8_t no_channels;                                                     \
    /* Store the save location. Used to differentiate loggers. */            \
    char *destination;                                                       \
    /* Buffer of data_t structures to be written by the logger. */           \
    data_buffer_t *write_buffer

#define base_logger_fields \
    base_logger_functions; \
    base_logger_attributes

typedef struct base_logger
{
    base_logger_fields;
} base_logger_t;

/* Initialize logger: set number of channels and calls _init from
 * base_logger, set status to IDLE if initialization is succesful */
base_logger_t *logger_init(base_logger_t *logger, uint8_t no_channels);

/* Register Channel to Logger. This function will use the smaller ID that is free.
 * The ID represents the position of the channel in logger->channels plus one.
 * Example:
 *      Channel with ID 1 is located at logger->channels[0].
 * Note. Currently the Logger will not check if this name already is already registered */
base_channel_t *logger_register(base_logger_t *logger, char *name, base_channel_t *channel);

/* Unregister Channel from Logger. This function will set the channel id to zero. */
void logger_unregister(base_logger_t *logger, base_channel_t *channel);

/* Push data to write buffer. Logger will execute the write operation in its main thread.
 * The cb function is called when the write operation is executed. */
void logger_write_async(base_logger_t *logger, channel_id_t id, uint8_t *data, uint16_t size, signal_cb_t cb);

// TBD in further versions:
// /* Same as logger_write_async, but at the write operation, the logger will also add a 64 bits
//  * timestamp before the data. */
// void logger_write_timestamp_async(base_logger_t *logger, uint8_t *data, uint16_t size, signal_cb_t cb);

/* This function calls _start from logger. Also set the logger status to RUNNING. */
void logger_start(base_logger_t *logger, char *destination);

/* This function calls _stop from logger. Also set the logger status to IDLE. */
void logger_stop(base_logger_t *logger);

/* Loop that will execute the write/wait logic. Adding in .h to let this function
 * as the last one on the .c file.*/
static void logger_loop(base_logger_t *logger);

#endif