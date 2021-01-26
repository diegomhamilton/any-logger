#include "can_channel.h"
#include "logger.h"
#include "chprintf.h"

/*
 * Internal loopback mode, 500KBaud, automatic wakeup, automatic recover
 * from abort mode.
 */
static const CANConfig cancfg = {
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
    CAN_BTR_LBKM | CAN_BTR_SJW(0) | CAN_BTR_TS2(1) |
    CAN_BTR_TS1(8) | CAN_BTR_BRP(6)
};

static base_logger_t *logger_ptr;
static base_channel_t can_channel;
static char can_channel_name[7] = "can_ch";

static volatile op_res_t can_message_written = SUCCESS;
static volatile op_res_t channel_active = false;
static CANRxFrame rxmsg;

static void write_callback(uint8_t *data) {
    if (data == (uint8_t *)&rxmsg) {
      can_message_written = SUCCESS;
    }
    else {
      can_message_written = FAIL;
    }
}

/*
 * Receiver thread.
 */
static THD_WORKING_AREA(can_rx_wa, 256);
static THD_FUNCTION(can_rx, p) {
    event_listener_t el;

    (void)p;
    chRegSetThreadName("receiver");
    chEvtRegister(&CAND1.rxfull_event, &el, 0);
    while (channel_active == true) {
        if (chEvtWaitAnyTimeout(ALL_EVENTS, TIME_MS2I(100)) == 0) {
            continue;
        }
        while ((canReceive(&CAND1, CAN_ANY_MAILBOX, &rxmsg, TIME_IMMEDIATE) == MSG_OK) && (can_message_written == SUCCESS)) {
            can_message_written = FAIL;
            if (logger_ptr->status == RUNNING)
              logger_write_async(logger_ptr, can_channel.id, (uint8_t *)&rxmsg, sizeof(CANRxFrame), write_callback);
        }
    }
    chEvtUnregister(&CAND1.rxfull_event, &el);
}

/*
 * Transmitter thread.
 */
static THD_WORKING_AREA(can_tx_wa, 256);
static THD_FUNCTION(can_tx, p) {
    CANTxFrame txmsg;

    (void)p;
    chRegSetThreadName("transmitter");
    txmsg.IDE = CAN_IDE_EXT;
    txmsg.EID = 0x01234567;
    txmsg.RTR = CAN_RTR_DATA;
    txmsg.DLC = 8;
    txmsg.data32[0] = 0x55AA55AA;
    txmsg.data32[1] = 0x00FF00FF;

    while (channel_active == true) {
        canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, TIME_MS2I(100));
        chThdSleepMilliseconds(500);
    }
}

void can_channel_init(void) {
    canStart(&CAND1, &cancfg);
}

void can_channel_register(base_logger_t *logger) {
    logger_ptr = logger;
    logger_register(logger, can_channel_name, &can_channel);
}

void can_channel_start(void) {
    channel_active = true;
    chThdCreateStatic(can_rx_wa, sizeof(can_rx_wa), NORMALPRIO + 7, can_rx, NULL);
    chThdCreateStatic(can_tx_wa, sizeof(can_tx_wa), NORMALPRIO + 7, can_tx, NULL);
}

void can_channel_stop(void) {
    channel_active = false;
}