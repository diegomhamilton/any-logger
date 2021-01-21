/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "logger.h"
#include "chprintf.h"

#define print() chprintf((BaseSequentialStream *) &SD2, "%s\r\n", __FUNCTION__)
#define exit() chThdExit((msg_t)NULL);

op_res_t chibios_serial_test_logger_init(void);
void chibios_serial_test_logger_start(char *destination);
void chibios_serial_test_logger_stop(void);
op_res_t chibios_serial_test_logger_write(data_t *data);
void chibios_serial_test_logger_wait(void);
void chibios_serial_test_logger_signal(void);
void chibios_serial_test_logger_lock(void);
void chibios_serial_test_logger_unlock(void);

signal_cb_t write_performed(uint8_t *data);

static data_buffer_t write_buffer;

#define NO_OF_CHANNELS 8

static base_channel_t *channels[NO_OF_CHANNELS];

static char chibios_serial_test_logger_destination[20] = "chibi_serial_stream";

static base_logger_t logger = {
    ._init = chibios_serial_test_logger_init,
    ._start = chibios_serial_test_logger_start,
    ._stop = chibios_serial_test_logger_stop,
    ._write = chibios_serial_test_logger_write,
    ._wait = chibios_serial_test_logger_wait,
    ._signal = chibios_serial_test_logger_signal,
    ._lock = chibios_serial_test_logger_lock,
    ._unlock = chibios_serial_test_logger_unlock,
    .status = 0,
    .registered_channels = 0,
    .channels = channels,
    .no_channels = NO_OF_CHANNELS,
    .destination = chibios_serial_test_logger_destination,
    .write_buffer = &write_buffer
};

static char channel1_test_name[10] = "channel1";
static char channel2_test_name[10] = "channel2";
static char channel3_test_name[10] = "channel3";

static base_channel_t channel1;
static base_channel_t channel2;
static base_channel_t channel3;


uint8_t dummy1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
uint8_t dummy2[10] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
uint8_t dummy3[10] = {20, 21, 22, 23, 24, 25, 26, 27, 28, 29};

static THD_WORKING_AREA(loggerThread, 512);
static THD_WORKING_AREA(ch1Thread, 128);
static THD_WORKING_AREA(ch2Thread, 128);
static THD_WORKING_AREA(ch3Thread, 128);

thread_t *logger_thread;
thread_t *ch1_thread;
thread_t *ch2_thread;
thread_t *ch3_thread;

static THD_FUNCTION(LoggerThread, arg);
static THD_FUNCTION(Ch1Thread, arg);
static THD_FUNCTION(Ch2Thread, arg);
static THD_FUNCTION(Ch3Thread, arg);

binary_semaphore_t write_available;

/*
 * Application entry point.
 */
int main(void)
{

    /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
    halInit();
    chSysInit();

    /*
   * Activates the serial driver 2 using the driver default configuration.
   */
    SerialConfig serial_cfg = {
        .speed = 115200};
    sdStart(&SD2, &serial_cfg);

    logger_init(&logger, NO_OF_CHANNELS);
    logger_register(&logger, channel1_test_name, &channel1);
    logger_register(&logger, channel2_test_name, &channel2);
    logger_register(&logger, channel3_test_name, &channel3);

    chBSemObjectInit(&write_available, false);
    
    /*
    * Creates application threads.
    */
    logger_thread = chThdCreateStatic(loggerThread, sizeof(loggerThread), NORMALPRIO, LoggerThread, NULL);
    ch1_thread = chThdCreateStatic(ch1Thread, sizeof(ch1Thread), NORMALPRIO + 3, Ch1Thread, NULL);
    ch2_thread = chThdCreateStatic(ch2Thread, sizeof(ch2Thread), NORMALPRIO + 2, Ch2Thread, NULL);
    ch3_thread = chThdCreateStatic(ch3Thread, sizeof(ch3Thread), NORMALPRIO + 1, Ch3Thread, NULL);

    
    while (true)
    {
        chprintf((BaseSequentialStream *)&SD2, "i'm alive\r\n");
        chThdSleepSeconds(5);
        if (chThdTerminatedX(ch1_thread) && chThdTerminatedX(ch2_thread) && chThdTerminatedX(ch3_thread)) {
            logger_stop(&logger);
            chprintf((BaseSequentialStream *)&SD2, "Logger Stopped! Status: %d\r\n", logger.status);
        }
    }
}

op_res_t chibios_serial_test_logger_init(void)
{
    print();
    return SUCCESS;
}

void chibios_serial_test_logger_start(char *destination)
{
    print();
    (void *) destination;
    return;
}

void chibios_serial_test_logger_stop(void)
{
    print();
    return;
}

op_res_t chibios_serial_test_logger_write(data_t *data)
{
    print();
    chprintf((BaseSequentialStream *)&SD2, "channel %d, %s: ", data->id, logger.channels[INDEX_OF(data->id)]->name);
    for (int i = 0; i < data->size; i++)
    {
        chprintf((BaseSequentialStream *)&SD2, "%d, ", data->data[i]);
    }
    chprintf((BaseSequentialStream *)&SD2, "\r\n");

    return SUCCESS;
}

signal_cb_t write_performed(uint8_t *data) {
    print();
}

void chibios_serial_test_logger_wait(void)
{
    print();
    chBSemWait(&write_available);
    return;
}

void chibios_serial_test_logger_signal(void)
{
    print();
    chBSemSignalI(&write_available);
    return;
}

void chibios_serial_test_logger_lock(void)
{
    print();
    return;
}

void chibios_serial_test_logger_unlock(void)
{
    print();
    return;
}

static THD_FUNCTION(LoggerThread, arg)
{
    (void)arg;

    chRegSetThreadName("logger_thread");
    logger_start(&logger, chibios_serial_test_logger_destination);
    exit();
}

static THD_FUNCTION(Ch1Thread, arg) {
    (void)arg;
    chRegSetThreadName("ch1_thread");

    for (int i = 0; i < 3; i++) {
        chThdSleepSeconds(2);
        print();
        logger_write_async(&logger, channel1.id, dummy1, 10, write_performed);
    }

    exit();
}

static THD_FUNCTION(Ch2Thread, arg) {
    (void)arg;
    chRegSetThreadName("ch2_thread");
    
    for (int i = 0; i < 3; i++) {
        chThdSleepSeconds(2);
        print();
        logger_write_async(&logger, channel2.id, dummy2, 10, write_performed);
    }

    exit();
}

static THD_FUNCTION(Ch3Thread, arg) {
    (void)arg;
    chRegSetThreadName("ch3_thread");

    for (int i = 0; i < 3; i++) {
        chThdSleepSeconds(2);
        print();
        logger_write_async(&logger, channel3.id, dummy3, 10, write_performed);
    }

    exit();
}
