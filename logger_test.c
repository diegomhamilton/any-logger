#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "logger.h"

#define print() printf("%s\r\n", __func__);

op_res_t logger_test_init(void);
void logger_test_start(char *destination);
void logger_test_stop(void);
op_res_t logger_test_write(data_t *data);
void logger_test_wait(void);
void logger_test_signal(void);
void logger_test_lock(void);
void logger_test_unlock(void);

void write_performed() {
    print();
}

static data_buffer_t write_buffer;

#define NO_OF_CHANNELS 8

static base_channel_t channels[NO_OF_CHANNELS];
base_channel_t *channels_ptr = channels;

static char logger_test_destination[20] = "logger_test_stream";

static base_logger_t logger = {
    ._init = &logger_test_init,
    ._start = &logger_test_start,
    ._stop = &logger_test_stop,
    ._write = &logger_test_write,
    ._wait = &logger_test_wait,
    ._signal = &logger_test_signal,
    ._lock = &logger_test_lock,
    ._unlock = &logger_test_unlock,
    .status = 0,
    .registered_channels = 0,
    .channels = &channels_ptr,
    .no_channels = NO_OF_CHANNELS,
    .destination = logger_test_destination,
    .write_buffer = &write_buffer
};

static char channel1_test_name[10] = "channel1";
static char channel2_test_name[10] = "channel2";

static base_channel_t channel1;
static base_channel_t channel2;

/* OS section */
sem_t write_available;
pthread_mutex_t write_lock;
pthread_t logger_th;
pthread_t ch1_th, ch2_th;
void *logger_thread(void *arg);
void *channel1_thread(void *arg);
void *channel2_thread(void *arg);
/* End of OS section */

int main(void) {
    logger_init(&logger, NO_OF_CHANNELS);
    logger_register(&logger, channel1_test_name, &channel1);
    logger_register(&logger, channel2_test_name, &channel2);

    int err;
    sem_init(&write_available, 0, 0);
    pthread_mutex_init(&write_lock, NULL);
    err = pthread_create(&logger_th, NULL, logger_thread, NULL);
    err = pthread_create(&ch1_th, NULL, channel1_thread, NULL);
    err = pthread_create(&ch2_th, NULL, channel2_thread, NULL);

    pthread_join(ch1_th, NULL);
    pthread_join(ch2_th, NULL);
    logger_stop(&logger);
    pthread_join(logger_th, NULL);
    pthread_mutex_destroy(&write_lock);

    return 0;
}

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
    uint8_t *dummy = data->data;
    printf("channel %d: %d %d %d\r\n", data->id, *dummy, *(dummy + 1), *(dummy + 2));
    return SUCCESS;
}

void logger_test_wait(void) {
    print();
    sem_wait(&write_available);
    return;
}

void logger_test_signal(void) {
    print();
    sem_post(&write_available);
    return;
}

void logger_test_lock(void) {
    print();
    pthread_mutex_lock(&write_lock);
    return;
}

void logger_test_unlock(void) {
    print();
    pthread_mutex_unlock(&write_lock);
    return;
}

void *logger_thread(void *arg) {
    logger_start(&logger, logger_test_destination);
}

void *channel1_thread(void *arg) {
    uint8_t dummy[3] = {1, 2, 3};

    for(int i=0; i < 3; i++) {
        sleep(2);
        logger_write_async(&logger, channel1.id, dummy, 3, write_performed);
    }
}

void *channel2_thread(void *arg) {
    uint8_t dummy[3] = {4, 5, 6};

    for(int i=0; i < 3; i++) {
        sleep(2);
        logger_write_async(&logger, channel2.id, dummy, 3, write_performed);
    }
}