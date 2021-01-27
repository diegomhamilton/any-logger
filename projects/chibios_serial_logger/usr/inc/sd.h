#ifndef _SD_H
#define _SD_H

#include "ch.h"
#include "hal.h"
#include "mcuconf.h"
#include "logger.h"

#ifdef STM32_SPI_USE_SPI2
#define SPI_SCK_PORT    GPIOB
#define SPI_SCK_PIN     13
#define SPI_MISO_PORT   GPIOB
#define SPI_MISO_PIN    14
#define SPI_MOSI_PORT   GPIOB
#define SPI_MOSI_PIN    15
#define SPI_CS_PORT     GPIOB
#define SPI_CS_PIN      12
#define SPI_DRIVER      (SPIDriver *)&SPID2
#endif

op_res_t sd_init(void);
void sd_start(void);

#endif /* _SD_H */