#ifndef ANALOG_CHANNEL_H
#define ANALOG_CHANNEL_H
#include "hal.h"
#include "logger.h"

#define TIMER_FREQUENCY     10000
#define TIMER_PRESCALER     50
/* This configuration will give a sampling frequency of 10000 / 50 which is 200 Hz */

/* ----------- */
/* ADC section */
/* ----------- */

#define ANALOG_BUFFER_DEPTH         256
#define ANALOG_NO_CHANNELS          4

void analog_channel_register(base_logger_t *logger);
void analog_channel_start(void);

#endif /* ANALOG_CHANNEL_H */