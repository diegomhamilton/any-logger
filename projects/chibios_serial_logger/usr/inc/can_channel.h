#ifndef _CAN_H
#define _CAN_H
#include "ch.h"
#include "hal.h"
#include "logger.h"

void can_channel_init(void);
void can_channel_register(base_logger_t *logger);
void can_channel_start(void); 

#endif  /* _CAN_H */