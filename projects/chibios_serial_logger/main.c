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
#include "can_channel.h"
#include "dac_test.h"

#define print() //chprintf((BaseSequentialStream *) &SD2, "%s\r\n", __FUNCTION__)
#define exit() chThdExit((msg_t)NULL);

op_res_t chibios_fsae_logger_init(void);
void chibios_fsae_logger_start(char *destination);
void chibios_fsae_logger_stop(void);
op_res_t chibios_fsae_logger_write(data_t *data);
void chibios_fsae_logger_wait(void);
void chibios_fsae_logger_signal(void);
void chibios_fsae_logger_lock(void);
void chibios_fsae_logger_unlock(void);
void register_channels(base_logger_t *logger_ptr);

static data_buffer_t write_buffer;

#define NO_OF_CHANNELS 8

static base_channel_t *channels[NO_OF_CHANNELS];

static char chibios_fsae_logger_destination[20] = "chibios_fsae_logger";

base_logger_t logger = {
    ._init = chibios_fsae_logger_init,
    ._start = chibios_fsae_logger_start,
    ._stop = chibios_fsae_logger_stop,
    ._write = chibios_fsae_logger_write,
    ._wait = chibios_fsae_logger_wait,
    ._signal = chibios_fsae_logger_signal,
    ._lock = 0,
    ._unlock = 0,
    .status = 0,
    .registered_channels = 0,
    .channels = channels,
    .no_channels = NO_OF_CHANNELS,
    .destination = chibios_fsae_logger_destination,
    .write_buffer = &write_buffer
};

static THD_WORKING_AREA(loggerThread, 512);

thread_t *logger_thread;

static THD_FUNCTION(LoggerThread, arg);

binary_semaphore_t write_available;

SerialConfig serial_cfg = { .speed = 115200 };

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

    logger_init(&logger, NO_OF_CHANNELS);
    register_channels(&logger);

    /*
    * Initialize semaphore and creates application thread.
    */
    chBSemObjectInit(&write_available, false);
    logger_thread = chThdCreateStatic(loggerThread, sizeof(loggerThread), NORMALPRIO, LoggerThread, NULL);
    
    /* Start operation of DAC for testing Analog Channels */
    dac_start();

    /* Blinker main thread to verify logger functionality */
    while (true)
    {
        palTogglePad(GPIOA, GPIOA_LED_GREEN);
        chThdSleepSeconds(1);
    }
}

op_res_t chibios_fsae_logger_init(void)
{
    /* Activates the serial driver 2. */
    sdStart(&SD2, &serial_cfg);
    /* Initialize acquisition channels */
    analog_channel_init();
    can_channel_init();
    return SUCCESS;
}

void chibios_fsae_logger_start(char *destination)
{
    (void) destination;
    analog_channel_start();
    can_channel_start();
    return;
}

void chibios_fsae_logger_stop(void)
{
    return;
}

op_res_t chibios_fsae_logger_write(data_t *data)
{
    chprintf((BaseSequentialStream *)&SD2, "CHANNEL %d (%s): ", data->id, logger.channels[INDEX_OF(data->id)]->name);
    for (int i = 0; i < data->size; i += 2)
    {
        chprintf((BaseSequentialStream *)&SD2, "%d,", data->data[i] + (data->data[i+1] << 8));
    }
    chprintf((BaseSequentialStream *)&SD2, "\r\n");

    return SUCCESS;
}

void chibios_fsae_logger_wait(void)
{
    chBSemWait(&write_available);
    return;
}

void chibios_fsae_logger_signal(void)
{
    chBSemSignalI(&write_available);
    return;
}

void register_channels(base_logger_t *logger_ptr) {
    analog_channel_register(logger_ptr);
    can_channel_register(logger_ptr);
}

static THD_FUNCTION(LoggerThread, arg)
{
    (void)arg;

    chRegSetThreadName("logger_thread");
    logger_start(&logger, chibios_fsae_logger_destination);
    exit();
}