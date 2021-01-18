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
#include "analog_channel.h"

#define print() chprintf((BaseSequentialStream *) &SD2, "%s\r\n", __FUNCTION__)
#define exit() chThdExit((msg_t)NULL);

op_res_t chibios_serial_logger_init(void);
void chibios_serial_logger_start(char *destination);
void chibios_serial_logger_stop(void);
op_res_t chibios_serial_logger_write(data_t *data);
void chibios_serial_logger_wait(void);
void chibios_serial_logger_signal(void);
void chibios_serial_logger_lock(void);
void chibios_serial_logger_unlock(void);

static data_buffer_t write_buffer;

#define NO_OF_CHANNELS 8

static base_channel_t *channels[NO_OF_CHANNELS];

static char chibios_serial_logger_destination[20] = "chibi_serial_stream";

base_logger_t logger = {
    ._init = chibios_serial_logger_init,
    ._start = chibios_serial_logger_start,
    ._stop = chibios_serial_logger_stop,
    ._write = chibios_serial_logger_write,
    ._wait = chibios_serial_logger_wait,
    ._signal = chibios_serial_logger_signal,
    ._lock = chibios_serial_logger_lock,
    ._unlock = chibios_serial_logger_unlock,
    .status = 0,
    .registered_channels = 0,
    .channels = channels,
    .no_channels = NO_OF_CHANNELS,
    .destination = chibios_serial_logger_destination,
    .write_buffer = &write_buffer
};

static THD_WORKING_AREA(loggerThread, 512);

thread_t *logger_thread;

static THD_FUNCTION(LoggerThread, arg);

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
    analog_channel_register(&logger);

    chBSemObjectInit(&write_available, false);
    /*
    * Creates application threads.
    */
    logger_thread = chThdCreateStatic(loggerThread, sizeof(loggerThread), NORMALPRIO, LoggerThread, NULL);
    
    while (true)
    {
        chprintf((BaseSequentialStream *)&SD2, "i'm alive\r\n");
        chThdSleepSeconds(5);
    }
}

op_res_t chibios_serial_logger_init(void)
{
    print();
    return SUCCESS;
}

void chibios_serial_logger_start(char *destination)
{
    print();
    (void) destination;
    analog_channel_start();
    return;
}

void chibios_serial_logger_stop(void)
{
    print();
    return;
}

op_res_t chibios_serial_logger_write(data_t *data)
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

void chibios_serial_logger_wait(void)
{
    print();
    chBSemWait(&write_available);
    return;
}

void chibios_serial_logger_signal(void)
{
    print();
    chBSemSignalI(&write_available);
    return;
}

void chibios_serial_logger_lock(void)
{
    print();
    return;
}

void chibios_serial_logger_unlock(void)
{
    print();
    return;
}

static THD_FUNCTION(LoggerThread, arg)
{
    (void)arg;

    chRegSetThreadName("logger_thread");
    logger_start(&logger, chibios_serial_logger_destination);
    exit();
}