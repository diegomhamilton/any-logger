#include <stdio.h>
#include "logger.h"

#define print() printf("%s\r\n", __func__);

op_res_t logger_test_init(void) {
    print();
    return SUCCESS;
}

void logger_test_start(char *destination) {
    print();
    return;
}

void logger_test_stop(void) {
    print();
    return;
}

op_res_t logger_test_write(data_t *data) {
    print();
    return SUCCESS;
}

void logger_test_wait(void) {
    print();
    return;
}

void logger_test_signal(void) {
    print();
    return;
}

static data_buffer_t write_buffer;

#define NO_OF_CHANNELS 8

static base_channel_t channels[NO_OF_CHANNELS];

static char logger_test_destination[20] = "logger_test_stream";

static base_logger_t logger = {
    ._init = &logger_test_init,
    ._start = &logger_test_start,
    ._stop = &logger_test_stop,
    ._write = &logger_test_write,
    ._wait = &logger_test_wait,
    ._signal = &logger_test_signal,
    .status = 0,
    .registered_channels = 0,
    .channels = &channels,
    .no_channels = NO_OF_CHANNELS,
    .destination = logger_test_destination,
    .write_buffer = &write_buffer
};

static char channel1_test_name[10] = "channel1";

static base_channel_t channel1;

int main(void) {
    logger_init(&logger, NO_OF_CHANNELS);
    logger_register(&logger, channel1_test_name, &channel1);
    printf("Channel ID after register: %d\r\n", channel1.id);
    logger_unregister(&logger, &channel1);
    printf("Channel ID after unregister: %d\r\n", channel1.id);
    logger_register(&logger, channel1_test_name, &channel1);
    printf("Channel ID after register: %d\r\n", channel1.id);

    


    return 0;
}