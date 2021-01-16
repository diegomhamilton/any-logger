#include "logger.h"
#include "channel.h"
#include "buffer.h"
#include <stdio.h>

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
    uint64_t regchan = logger->registered_channels;
    uint8_t temp = 0;
    channel_id_t id = 0;

    do {
        temp = (regchan >> id) & 0x01;
        id += 1;
    } while(temp != 0 || id == logger->no_channels);

    return id;
}

base_channel_t *logger_register(base_logger_t *logger, char *name, base_channel_t *channel) {
    /* Cannot register channel while logger is running, returns NULL */
    if (logger->status != IDLE) {
        return 0;
    }

    channel->id = register_new_channel(logger);
    logger->registered_channels |= 1 << INDEX_OF(channel->id);
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
    logger->registered_channels &= ~(1 >> channel->id);
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

    if(logger->_lock) logger->_lock();
    buffer_push(*(logger->write_buffer), temp, res);
    if(logger->_unlock) logger->_unlock();
    //TODO: Add condition to only call _signal() if push was successful
    logger->_signal();
}

void logger_start(base_logger_t *logger, char *destination) {
    if (logger->status == IDLE) {
        buffer_reset(*(logger->write_buffer));
        if (logger->_start) logger->_start(destination);
        logger->status = RUNNING;
        logger_loop(logger);
    }
}

void logger_stop(base_logger_t *logger) {
    logger->status = IDLE;
    /* this is intended to solve an issue where the logger stops
     * but the loop isn't destroyed */
    logger->_signal();
    if (logger->_stop) logger->_stop();
}

static void perform_write(base_logger_t *logger) {
    data_t temp;
    
    if(!is_buffer_empty(*(logger->write_buffer))) {
        if(logger->_lock) logger->_lock();
        buffer_pop(*(logger->write_buffer), temp);
        if(logger->_unlock) logger->_unlock();

        if (logger->_write(&temp) == SUCCESS) {
            if(temp.write_cb) {
                temp.write_cb(temp.data);
            }
        }
    } else {
        logger->_wait();
    }
}

static void logger_loop(base_logger_t *logger) {
    while(logger->status == RUNNING) {
        perform_write(logger);
    }
}