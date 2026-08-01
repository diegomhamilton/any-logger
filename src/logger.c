#include "logger.h"
#include "channel.h"
#include "buffer.h"

base_logger_t *logger_init(base_logger_t *logger, uint8_t no_channels) {
    if (logger->_init() == SUCCESS) {
        logger->status = IDLE;
        logger->no_channels = no_channels;
        logger->registered_channels = 0;
    } else {
        logger->status = UNINITIALIZED;
        logger->no_channels = 0;
    }

    return logger;
}

static channel_id_t register_new_channel(base_logger_t *logger) {
    uint8_t limit = logger->no_channels;

    if (limit > 64) {
        limit = 64;
    }

    for (channel_id_t id = 1; id <= limit; id++) {
        if ((logger->registered_channels & (1ULL << INDEX_OF(id))) == 0) {
            return id;
        }
    }

    return 0;
}

base_channel_t *logger_register(base_logger_t *logger, char *name, base_channel_t *channel) {
    /* Cannot register channel while logger is running, returns NULL */
    if (!logger || !channel || logger->status != IDLE) {
        return 0;
    }

    channel->id = register_new_channel(logger);
    if (channel->id == 0) {
        return 0;
    }

    logger->registered_channels |= 1ULL << INDEX_OF(channel->id);
    channel->name = name;
    if (channel->id <= logger->no_channels) {
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
    logger->registered_channels &= ~(1ULL << INDEX_OF(channel->id));
    /* Set channel to null in logger's channel pointers array */
    (logger->channels)[INDEX_OF(channel->id)] = 0;
    /* Set channel id to zero */
    channel->id = 0;
}

void logger_write_async(base_logger_t *logger, channel_id_t id, uint8_t *data, uint16_t size, signal_cb_t cb) {
    data_t temp = {
        .data = data,
        .id = id,
        .size = size,
        .write_cb = cb
    };

    if(logger->_lock) logger->_lock();
    cb_push(*(logger->write_buffer), temp);
    if(logger->_unlock) logger->_unlock();
    //TODO: Add condition to only call _signal() if push was successful
    logger->_signal();
}

static void perform_write(base_logger_t *logger) {
    data_t temp;
    
    if(!cb_empty(*(logger->write_buffer))) {
        if(logger->_lock) logger->_lock();
        cb_pop(*(logger->write_buffer), temp);
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

void logger_start(base_logger_t *logger, char *destination) {
    if (logger->status == IDLE) {
        cb_init(*(logger->write_buffer), data_t, WRITE_BUFFER_SIZE);
        logger->destination = destination;
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
