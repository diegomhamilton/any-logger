#include "logger.h"
#include "channel.h"
#include "buffer.h"

base_logger_t *logger_init(base_logger_t *logger, uint8_t no_channels) {
    if (logger->_init() == SUCCESS) {
        logger->status = IDLE;
        logger->no_channels = no_channels;
    } else {
        logger->status = UNINITIALIZED;
        logger->no_channels = 0;
    }

    return logger;
}

channel_id_t register_new_channel(base_logger_t *logger) {
    logger->registered_channels |= 1;
    
    return 1;
}

base_channel_t *logger_register(base_logger_t *logger, char *name, base_channel_t *channel) {
    /* Cannot register channel while logger is running, returns the channel as it is */
    if (logger->status != IDLE) {
        return channel;
    }

    channel->id = register_new_channel(logger);
    channel->name = name;
    if (channel->id < logger->no_channels) {
        logger->channels[INDEX_OF(channel->id)] = channel;
    }
    return channel;
}

void logger_unregister(base_logger_t *logger, base_channel_t *channel) {
    /* Cannot unregister channel while logger is running, if channel
     * is not registered or if ID is out of range. */
    if (logger->status != IDLE
        || channel->id == 0
        || channel->id > logger->no_channels) {
        return;
    }

    /* Clear channel from registered_channels */
    logger->registered_channels &= (1 >> channel->id);
    /* Set channel to null in logger's channel pointers array */
    (logger->channels)[INDEX_OF(channel->id)] = 0;
    /* Set channel id to zero */
    channel->id = 0;
}

void logger_write_async(base_logger_t *logger, channel_id_t id, uint8_t *data, uint16_t size, signal_cb_t cb) {
    int8_t res = 0;
    data_t temp = {
        .data = data,
        .id = id,
        .size = size,
        .write_cb = cb
    };

    buffer_push(*(logger->write_buffer), temp, res);
    //TODO: Add condition to only call _signal() if push was successful
    logger->_signal();
}

void logger_start(base_logger_t *logger, char *destination) {
    if (logger->status == IDLE) {
        buffer_reset(*(logger->write_buffer));
        logger->_start(destination);
        logger->status = RUNNING;
        logger_loop(logger);
    }
}

void logger_stop(base_logger_t *logger) {
    logger->status = IDLE;
    logger->_stop();
}

static void perform_write(base_logger_t *logger) {
    data_t temp;

    buffer_pop(*(logger->write_buffer), temp);
    if (logger->_write(&temp) == SUCCESS) {
        temp.write_cb(temp.data);
    }
}

static void logger_loop(base_logger_t *logger) {
    while(logger->status == RUNNING) {
        if(is_buffer_empty(*(logger->write_buffer))) {
            logger->_wait();
        }
        perform_write(logger);
    }
}

// int main(void) {
//     return 0;
// }