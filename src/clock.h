#ifndef GENSHIN_CLOCK_H
#define GENSHIN_CLOCK_H

#include <lvgl.h>
#include <RTClib.h>
#include "boardconf.h"

//引用图片文件
LV_IMG_DECLARE(hand_h);
LV_IMG_DECLARE(hand_m);
LV_IMG_DECLARE(hand_s);
#if USE_ESP32S3
LV_IMG_DECLARE(star_bg);
#else
LV_IMG_DECLARE(star_bg_98);
#endif
LV_IMG_DECLARE(star_mask);
LV_IMG_DECLARE(Horoscope03);
LV_IMG_DECLARE(Horoscope04);
LV_IMG_DECLARE(Horoscope05);
LV_IMG_DECLARE(Horoscope06);
// LV_IMG_DECLARE(arc_mask);

//定义时分秒指针图像对象
static lv_obj_t *lvMinute;
static lv_obj_t *lvHour;
static lv_obj_t *lvSecond;
// static lv_obj_t *arc;
// static lv_style_t arc_style;
//bool isCountdown = false;

// static uint32_t countdownts = 0;            //倒计时结束的时间戳
#if !USE_DS1302_RTC
    static uint32_t currtimestamp = 1664781120; //没有时间的默认初始时间
#endif


static void rotate_img(void *img, int32_t v)
{
    lv_img_set_angle((lv_obj_t *)img, v);
}

class Genshin_Clock
{
private:
    DateTime clock_time;

    bool _isClockInit = false;

    uint32_t last_timestamp = 0;

public:
    Genshin_Clock();

    void setDateTime(uint32_t timestamp);

    void initClock();

    void initClockBg();

    uint32_t getTimestamp();

    bool isClockInit();

    void loop();

    void refresh();

    DateTime getDateTime();

    bool isTimeChanged();
#if !USE_DS1302_RTC
    static void timestampAdd();
#endif
};

#endif