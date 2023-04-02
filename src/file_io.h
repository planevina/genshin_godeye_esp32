#ifndef MY_FILE_IO_H
#define MY_FILE_IO_H

#include "boardconf.h"
#include <FS.h>

//S3初始化SDMMC和FATFS
//ESP32初始化SD_SPI
#if USE_ESP32S3
#include <SD_MMC.h>
#include <FFat.h>
#else
#include <SD.h>
#include <SPI.h>
#endif


#if USE_ESP32S3
bool sdmmc_init()
{
    // pinMode(SD_CS, OUTPUT);
    // digitalWrite(SD_CS, HIGH); //SDMMC模式用不到CS脚
    SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_D0);
    //因为默认的40mhz频率在s3上有问题，会不时卡住，搜索过后得到解决方案就是降低频率到20mhz
    if (!SD_MMC.begin("/root", true, false, SDMMC_FREQ_DEFAULT)) //参数：挂载目录，启用1bit，挂载失败后格式化，速率
    {
        Serial.println("Card Mount Failed");
        return false;
    }
    uint8_t cardType = SD_MMC.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return false;
    }
    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC)
    {
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        Serial.println("SDHC");
    }
    else
    {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    return true;
}

bool fatfs_init(){
    if(!FFat.begin(false, "", 1)){
        Serial.println("[FATFS] NO FATFS.");
        return false;
    }
    Serial.printf("[FATFS] Total space: %10lu\n", FFat.totalBytes());
    Serial.printf("[FATFS] Free space: %10lu\n", FFat.freeBytes());
    return true;
}

#else

bool sd_init()
{
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS))
    {
        Serial.println("Card Mount Failed");
        return false;
    }
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return false;
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC)
    {
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        Serial.println("SDHC");
    }
    else
    {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    return true;
}

#endif

#endif













