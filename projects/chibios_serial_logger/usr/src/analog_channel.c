#include "analog_channel.h"

/* Callback for Timer operation */
static void timer_cb(GPTDriver *gptp) {
    //TODO: Implement periodic action
    (void) gptp;
}

/* Timer 3 Configuration. */
static const GPTConfig xTIM3Config = {
    /* Frequency */
    .frequency = TIMER_FREQUENCY,
    /* Callback */
    .callback = timer_cb,
    .cr2 = 0,
    /* Dier */
    .dier = TIM_DIER_TIE | TIM_DIER_UIE
};

/* Initialize Timer 3 with TRGO enabled */
static void timer_init(void) {
    gptObjectInit(&GPTD3);
    gptStart(&GPTD3, &xTIM3Config);
    /* TRGO Event */
    GPTD3.tim->CR2 &= ~TIM_CR2_MMS;
    GPTD3.tim->CR2 |= TIM_CR2_MMS_1;
}

/* Start Timer 3 operation */
static void timer_start(void) {
    /* Period will be TIMER_FREQUENCY divided by TIMER_PRESCALER */
    gptStartContinuous(&GPTD3, TIMER_PRESCALER);
}

static adcsample_t analog_channel_buffer[ANALOG_BUFFER_DEPTH * ANALOG_NO_CHANNELS];

static base_logger_t *logger_ptr;
static base_channel_t analog_channel;
static char *analog_channel_name = "ch_analog";

static void adc_error_callback(ADCDriver *adcp, adcerror_t err) {
    (void)adcp;
    (void)err;
}

static const uint16_t half_buffer_size = sizeof(adcsample_t) * ANALOG_BUFFER_DEPTH * ANALOG_NO_CHANNELS/2;

static void adc_conv_callback(ADCDriver *adcp) {
    if (adcIsBufferComplete(adcp)) {
        /* Handle DMA full buffer complete */
        logger_write_async(logger_ptr, analog_channel.id, (uint8_t *) &analog_channel_buffer[half_buffer_size/sizeof(adcsample_t)], half_buffer_size, 0);
    } else {
        /* Handle DMA half buffer complete */
        logger_write_async(logger_ptr, analog_channel.id, (uint8_t *) &analog_channel_buffer[0], half_buffer_size, 0);
    }
}

/*
 * ADC conversion group.
 * Mode:        Circular buffer, 1 samples of 1 channels, triggered by timer event TRGO.
 * Channels:    IN0.
 */
static const ADCConversionGroup adcgrpcfg1 = {
    .circular = TRUE,
    .num_channels = ANALOG_NO_CHANNELS,
    .end_cb = adc_conv_callback,
    .error_cb = adc_error_callback,
    /* CFGR */
    .cfgr = (0b0100 << ADC_CFGR_EXTSEL_Pos) | (0b10 << ADC_CFGR_EXTEN_Pos),
    /* TR1 */
    .tr1 = 0,
    /* { SMPR1, SMPR2} */
    .smpr = {0, 0},
    /* { SQR1, SQR2, SQR3, SQR4 } */
    .sqr = { ADC_SQR1_NUM_CH(ANALOG_NO_CHANNELS)
        | ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1)   // mapped to A0 (PA_0)
        | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN2)   // mapped to A1 (PA_1)
        | ADC_SQR1_SQ3_N(ADC_CHANNEL_IN7)   // mapped to A4 (PC_1)
        | ADC_SQR1_SQ4_N(ADC_CHANNEL_IN6),  // mapped to A5 (PC_0)
        0, 0, 0 }
};

void analog_channel_init(void) {
    adcStart(&ADCD1, NULL);
    timer_init();
}

void analog_channel_register(base_logger_t *logger) {
    logger_ptr = logger;
    logger_register(logger_ptr, analog_channel_name, &analog_channel);
}

void analog_channel_start(void) {
    timer_start();
    adcStartConversion(&ADCD1, &adcgrpcfg1, &analog_channel_buffer[0], ANALOG_BUFFER_DEPTH);
}

void analog_channel_stop(void) {
    gptStopTimer(&GPTD3);
    adcStopConversion(&ADCD1);
}