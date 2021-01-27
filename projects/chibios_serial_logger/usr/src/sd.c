#include "sd.h"
#include "ff.h"
#include <string.h>
#include "chprintf.h"

/*
 * SPI Configurations
 *
 * Maximum speed SPI configuration (18MHz, CPHA=0, CPOL=0, MSb first).
 */
static const SPIConfig hs_spicfg = {
    false,
    NULL,
    SPI_CS_PORT,
    SPI_CS_PIN,
    0,
    0
};

/*
 * Low speed SPI configuration (281.250kHz, CPHA=0, CPOL=0, MSb first).
 */
static const SPIConfig ls_spicfg = {
    false,
    NULL,
    SPI_CS_PORT,
    SPI_CS_PIN,
    SPI_CR1_BR_2 | SPI_CR1_BR_1,
    0
};

/* MMC configurations */
static MMCConfig mmccfg = {SPI_DRIVER, &ls_spicfg, &hs_spicfg};
MMCDriver MMCD1;


/**
 * @brief FS object.
 */
static FATFS SDC_FS;
#define is_fs_ready(fs) fs->fs_type

/*
 * Setup I/O pins of SPI.
 */
static void spi_pin_config(void) {
    palSetPadMode(SPI_SCK_PORT, SPI_SCK_PIN, PAL_MODE_ALTERNATE(5) | PAL_STM32_OTYPE_PUSHPULL);   // SCK
    palSetPadMode(SPI_MISO_PORT, SPI_MISO_PIN, PAL_MODE_ALTERNATE(5) | PAL_STM32_OTYPE_PUSHPULL); // MISO
    palSetPadMode(SPI_MOSI_PORT, SPI_MOSI_PIN, PAL_MODE_ALTERNATE(5) | PAL_STM32_OTYPE_PUSHPULL); // MOSI
    palSetPadMode(SPI_CS_PORT, SPI_CS_PIN, PAL_MODE_OUTPUT_PUSHPULL);              // CS
    /* Enable Chip Select. If other SPI devices are connected to the same peripheralm,
     * be aware of setting the correct Chip Select during each transfer.
     */
    palSetPad(SPI_CS_PORT, SPI_CS_PIN);
}

/*
 * Start MMC over SPI.
 */
static void mmc_start_with_spi(MMCDriver *mmcd, MMCConfig *cfg) {
    spi_pin_config();
    mmcObjectInit(mmcd);
    mmcStart(mmcd, cfg);
}


/*
 * Mount FatFS on SD.
 */
static FRESULT sd_mount(FATFS *fs, MMCDriver *mmcd) {
    FRESULT res;

    res = f_mount(fs, "/", 1);
    if (res != FR_OK) {
        // chprintf((BaseSequentialStream *)&SD2, "Disconnecting...\r\n");
        mmcDisconnect(mmcd);
    } else {
        // chprintf((BaseSequentialStream *)&SD2, "FatFS mounted.\r\n");
    }

    return res;
}

static FRESULT scan_files(BaseSequentialStream *chp, char *path) {
    static FILINFO fno;
    FRESULT res;
    DIR dir;
    size_t i;
    char *fn;

    chprintf(chp, "scanning files\r\n");
    chThdSleepMilliseconds(1000);

    res = f_opendir(&dir, path);
    if (res == FR_OK) {
        i = strlen(path);
        while (((res = f_readdir(&dir, &fno)) == FR_OK) && fno.fname[0]) {
            if (FF_FS_RPATH && fno.fname[0] == '.') {
                chprintf(chp, "first if\r\n");
                continue;
            }

            fn = fno.fname;
            if (fno.fattrib & AM_DIR) {
                *(path + i) = '/';
                strcpy(path + i + 1, fn);
                chprintf(chp, "before recursive call\r\n");
                chThdSleepMilliseconds(200);
                res = scan_files(chp, path);
                chprintf(chp, "after recursive call\r\n");
                chThdSleepMilliseconds(200);
                
                *(path + i) = '\0';
                if (res != FR_OK)
                    break;
            } else {
                chprintf(chp, "%s/%s\r\n", path, fn);
            }
        }
    }
    return res;
}

/*
 * SD initialization.
 */
op_res_t sd_init(void) {
    mmc_start_with_spi(&MMCD1, &mmccfg);

    // chprintf((BaseSequentialStream *)&SD2, "Trying to connect: %d\r\n", MMCD1.state);
    
    /* Wait for SD card connection. */
    while (mmcConnect(&MMCD1)) {
        //TODO: implement card insertion detection.
        // chprintf((BaseSequentialStream *)&SD2, "Card not connected\r\n");
        chThdSleepMilliseconds(1000);
    }
    // chprintf((BaseSequentialStream *)&SD2, "Connected?\r\n");

    /* Mount SD FileSystem. */
    FRESULT res = sd_mount(&SDC_FS, &MMCD1);
    if (res == FR_OK) {
        return SUCCESS;
    } else {
        // chprintf((BaseSequentialStream *)&SD2, "Card isn't FAT formatted\r\n");
        return FAIL;
    }
}

void sd_start() {
    char b[100];
    b[0] = 0;
    
    scan_files((BaseSequentialStream *)&SD2, b);
}
