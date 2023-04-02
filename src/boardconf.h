#ifndef BOARD_CONF_H
#define BOARD_CONF_H

//璃月版
#define LIYUE_VER 1

//ESP32版请将以下选项都改为0
//S3版，如果没有焊接时钟芯片，下面这个也要改为0

//启用RTC时钟
#define USE_DS1302_RTC 0

//使用S3模组
#define USE_ESP32S3 0
#if USE_ESP32S3
    #define TFT_MISO -1        //没有实际连接
    #define TFT_SCK 12
    #define TFT_MOSI 13
    #define TFT_CS 11
    #define TFT_BLK 14
    #define TFT_DC 10
    #define TFT_RST 21
    #define BTN_PIN 38
    #define LVGL_BUFFER_LEVEL 1     //lvgl缓存大小，1=100%（全屏），2=50%（半屏），3=33.3%（三分之一屏），4=25%（四分之一屏）
    //修改前需确认分辨率高能被这个数值整除

    //#define SD_CS 8
    #define SDMMC_CMD 42
    #define SDMMC_CLK 41
    #define SDMMC_D0 40
#else
    #define TFT_MISO 2
    #define TFT_SCK 14
    #define TFT_MOSI 15
    #define TFT_CS 5
    #define TFT_BLK 22
    #define TFT_DC 27
    #define TFT_RST 33
#if LIYUE_VER
    #define BTN_PIN 23
#else
    #define BTN_PIN 16
#endif
    #define LVGL_BUFFER_LEVEL 3    // ESP32只能开半屏缓存，就这样还剩10KB可用内存 

    #define SD_MISO 2
    #define SD_SCK 14
    #define SD_MOSI 15
    #define SD_CS 13
#endif

#endif