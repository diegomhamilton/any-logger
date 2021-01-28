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
#include "dac_test.h"
#include "ch.h"
#include "hal.h"

#define DAC_BUFFER_SIZE 360

/*
 * DAC test buffer (sine wave).
 */
static const dacsample_t dac_buffer[DAC_BUFFER_SIZE] = {
  1500, 1517, 1535, 1553, 1571, 1589, 1607, 1624, 1642, 1660, 1677, 1695, 1712, 1730, 1747, 1765, 1782, 1799, 1816, 1833, 1850, 1866, 1883, 1900, 1916, 1932, 1948, 1964, 1980, 1996, 2012, 2027, 2042, 2057, 2072, 2087, 2101, 2116, 2130, 2144, 2158, 2171, 2185, 2198, 2211, 2224, 2236, 2248, 2260, 2272, 2284, 2295, 2306, 2317, 2328, 2338, 2348, 2358, 2368, 2377, 2386, 2395, 2404, 2412, 2420, 2428, 2435, 2442, 2449, 2455, 2462, 2468, 2473, 2479, 2484, 2489, 2493, 2497, 2501, 2505, 2508, 2511, 2514, 2516, 2518, 2520, 2521, 2522, 2523, 2523, 2524, 2523, 2523, 2522, 2521, 2520, 2518, 2516, 2514, 2511, 2508, 2505, 2501, 2497, 2493, 2489, 2484, 2479, 2473, 2468, 2462, 2455, 2449, 2442, 2435, 2428, 2420, 2412, 2404, 2395, 2386, 2377, 2368, 2358, 2348, 2338, 2328, 2317, 2306, 2295, 2284, 2272, 2260, 2248, 2236, 2224, 2211, 2198, 2185, 2171, 2158, 2144, 2130, 2116, 2101, 2087, 2072, 2057, 2042, 2027, 2012, 1996, 1980, 1964, 1948, 1932, 1916, 1900, 1883, 1866, 1850, 1833, 1816, 1799, 1782, 1765, 1747, 1730, 1712, 1695, 1677, 1660, 1642, 1624, 1607, 1589, 1571, 1553, 1535, 1517, 1500, 1482, 1464, 1446, 1428, 1410, 1392, 1375, 1357, 1339, 1322, 1304, 1287, 1269, 1252, 1234, 1217, 1200, 1183, 1166, 1149, 1133, 1116, 1099, 1083, 1067, 1051, 1035, 1019, 1003, 987, 972, 957, 942, 927, 912, 898, 883, 869, 855, 841, 828, 814, 801, 788, 775, 763, 751, 739, 727, 715, 704, 693, 682, 671, 661, 651, 641, 631, 622, 613, 604, 595, 587, 579, 571, 564, 557, 550, 544, 537, 531, 526, 520, 515, 510, 506, 502, 498, 494, 491, 488, 485, 483, 481, 479, 478, 477, 476, 476, 476, 476, 476, 477, 478, 479, 481, 483, 485, 488, 491, 494, 498, 502, 506, 510, 515, 520, 526, 531, 537, 544, 550, 557, 564, 571, 579, 587, 595, 604, 613, 622, 631, 641, 651, 661, 671, 682, 693, 704, 715, 727, 739, 751, 763, 775, 788, 801, 814, 828, 841, 855, 869, 883, 898, 912, 927, 942, 957, 972, 987, 1003, 1019, 1035, 1051, 1067, 1083, 1099, 1116, 1133, 1149, 1166, 1183, 1200, 1217, 1234, 1252, 1269, 1287, 1304, 1322, 1339, 1357, 1375, 1392, 1410, 1428, 1446, 1464, 1482
};

/*
 * DAC streaming callback.
 */
size_t nx = 0, ny = 0, nz = 0;
static void end_cb1(DACDriver *dacp) {

  nz++;
  if (dacIsBufferComplete(dacp)) {
    nx += DAC_BUFFER_SIZE / 2;
  }
  else {
    ny += DAC_BUFFER_SIZE / 2;
  }

  // if ((nz % 1000) == 0) { }
}

/*
 * DAC error callback.
 */
static void error_cb1(DACDriver *dacp, dacerror_t err) {

  (void)dacp;
  (void)err;

  chSysHalt("DAC failure");
}

static const DACConfig dac1cfg1 = {
  .init         = 2047U,
  .datamode     = DAC_DHRM_12BIT_RIGHT,
  .cr           = 0
};

static const DACConversionGroup dacgrpcfg1 = {
  .num_channels = 1U,
  .end_cb       = end_cb1,
  .error_cb     = error_cb1,
  .trigger      = DAC_TRG(0)
};

/*
 * GPT6 configuration.
 */
static const GPTConfig gpt6cfg1 = {
  .frequency    = 360U,
  .callback     = NULL,
  .cr2          = TIM_CR2_MMS_1,    /* MMS = 010 = TRGO on Update Event.    */
  .dier         = 0U
};

/*
 * Application entry point.
 */
void dac_start(void) {
  /*
   * Starting DAC1 driver, setting up the output pin as analog as suggested
   * by the Reference Manual.
   */
  palSetPadMode(GPIOA, 4, PAL_MODE_INPUT_ANALOG);
  dacStart(&DACD1, &dac1cfg1);

  /*
   * Starting GPT6 driver, it is used for triggering the DAC.
   */
  gptStart(&GPTD6, &gpt6cfg1);

  /*
   * Starting a continuous conversion.
   */
  dacStartConversion(&DACD1, &dacgrpcfg1,
                     (dacsample_t *)dac_buffer, DAC_BUFFER_SIZE);
  gptStartContinuous(&GPTD6, 2U);
}
