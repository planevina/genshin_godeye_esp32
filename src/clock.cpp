
#include "clock.h"

#if USE_DS1302_RTC
DS1302 ds1302(39, 2, 1);
#endif

Genshin_Clock::Genshin_Clock()
{
}

void Genshin_Clock::begin()
{
#if USE_DS1302_RTC
    ds1302.begin();
    if (ds1302.isrunning())
    {
        Serial.println("[RTC] DS1302 is running!");
    }
    else
    {
        Serial.println("[RTC] DS1302 is NOT running!");
    }
    clock_time = ds1302.now();
#endif
}

/**
 * @brief 计算4个图标透明度
 *
 * @param
 */
void Genshin_Clock::calcOpa()
{
    if (clock_time.hour() >= 0 && clock_time.hour() < 6)
    {
        opa[0] = 255 - (clock_time.hour() * 60 + clock_time.minute()) * 0.6;
        opa[1] = 39 + (clock_time.hour() * 60 + clock_time.minute()) * 0.6;
        opa[2] = 0;
        opa[3] = 0;
    }
    else if (clock_time.hour() >= 6 && clock_time.hour() < 12)
    {
        opa[0] = 0;
        opa[1] = 255 - ((clock_time.hour() - 6) * 60 + clock_time.minute()) * 0.6;
        opa[2] = 39 + ((clock_time.hour() - 6) * 60 + clock_time.minute()) * 0.6;
        opa[3] = 0;
    }
    else if (clock_time.hour() >= 12 && clock_time.hour() < 18)
    {
        opa[0] = 0;
        opa[1] = 0;
        opa[2] = 255 - ((clock_time.hour() - 12) * 60 + clock_time.minute()) * 0.6;
        opa[3] = 39 + ((clock_time.hour() - 12) * 60 + clock_time.minute()) * 0.6;
    }
    else
    {
        //>=18
        opa[0] = 39 + ((clock_time.hour() - 18) * 60 + clock_time.minute()) * 0.6;
        opa[1] = 0;
        opa[2] = 0;
        opa[3] = 255 - ((clock_time.hour() - 18) * 60 + clock_time.minute()) * 0.6;
    }
}


/**
 * @brief 初始化时钟
 *
 * @param
 */
void Genshin_Clock::initClock()
{
    initClockBg();
#if !USE_DS1302_RTC
    last_timestamp = currtimestamp;
#endif
    clock_time = getDateTime();
    uint8_t hr = clock_time.hour();
    if (hr == 0)
    {
        hr = 12;
    }
    else if (hr > 12)
    {
        hr -= 12;
    }
#if LIYUE_VER
    last_m = clock_time.minute();
#endif
    lvHour = lv_img_create(lv_scr_act());
    lv_img_set_src(lvHour, &hand_h);
    lv_obj_align(lvHour, LV_ALIGN_CENTER, 0, -44);
    lv_img_set_pivot(lvHour, 16, 81);
#if LIYUE_VER
    hPos = hr * 300 + clock_time.minute() * 5 + 450;
#else
    hPos = hr * 300 + clock_time.minute() * 5;
#endif
    lv_img_set_angle(lvHour, hPos);

    lvMinute = lv_img_create(lv_scr_act());
    lv_img_set_src(lvMinute, &hand_m);
    lv_obj_align(lvMinute, LV_ALIGN_CENTER, 0, -43);
    lv_img_set_pivot(lvMinute, 12, 92);
#if LIYUE_VER
    mPos = clock_time.minute() * 60 + 450;
#else
    mPos = clock_time.minute() * 60;
#endif
    lv_img_set_angle(lvMinute, mPos);

    lvSecond = lv_img_create(lv_scr_act());
    lv_img_set_src(lvSecond, &hand_s);
    lv_obj_align(lvSecond, LV_ALIGN_CENTER, 0, -58);
    lv_img_set_pivot(lvSecond, 6, 110);
#if LIYUE_VER
    lv_img_set_angle(lvSecond, clock_time.second() * 60 + 450);
#else
    lv_img_set_angle(lvSecond, clock_time.second() * 60);
#endif

    //初始化圆弧
    arc = lv_arc_create(lv_scr_act());
    lv_arc_set_mode(arc, LV_ARC_MODE_NORMAL);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);

    lv_arc_set_bg_start_angle(arc, mPos < 900 ? 270 + mPos / 10 : (mPos - 900) / 10);
    lv_arc_set_bg_end_angle(arc, hPos < 900 ? 270 + hPos / 10 : (hPos - 900) / 10);

    lv_obj_set_size(arc, 192, 192);
    lv_obj_align(arc, LV_ALIGN_CENTER, 0, 0);
    lv_style_init(&arc_style);
    lv_style_set_arc_img_src(&arc_style, &arc_mask);
    lv_style_set_arc_width(&arc_style, 4);
    lv_style_set_arc_rounded(&arc_style, false);
    lv_obj_add_style(arc, &arc_style, 0);
    lv_obj_center(arc);

    lv_obj_add_flag(arc, LV_OBJ_FLAG_HIDDEN);

    //初始化图标，仅限方屏
#if LIYUE_VER
    calcOpa();

    lvMoonrise = lv_img_create(lv_scr_act());
    lv_img_set_src(lvMoonrise, &moonrise);
    lv_obj_align(lvMoonrise, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_img_opa(lvMoonrise, opa[0], 0); //透明度范围是0-255  每分钟更新一次

    lvSunrise = lv_img_create(lv_scr_act());
    lv_img_set_src(lvSunrise, &sunrise);
    lv_obj_align(lvSunrise, LV_ALIGN_TOP_LEFT, opa[0], 0);
    lv_obj_set_style_img_opa(lvSunrise, opa[1], 0);

    lvDaytime = lv_img_create(lv_scr_act());
    lv_img_set_src(lvDaytime, &daytime);
    lv_obj_align(lvDaytime, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_style_img_opa(lvDaytime, opa[2], 0);

    lvSunset = lv_img_create(lv_scr_act());
    lv_img_set_src(lvSunset, &sunset);
    lv_obj_align(lvSunset, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_img_opa(lvSunset, opa[3], 0);
#endif

    _isClockInit = true;
}

/**
 * @brief 初始化时钟背景
 *
 * @param
 */
void Genshin_Clock::initClockBg()
{
    //去掉滚动条
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(lv_scr_act(), LV_SCROLLBAR_MODE_OFF);

    //背景
    lv_obj_t *bgimg = lv_img_create(lv_scr_act());

#if USE_ESP32S3
    lv_img_set_src(bgimg, &star_bg); // S3直接用196x196的背景图
#else
    lv_img_set_src(bgimg, &star_bg_98); // ESP32用一张98x98的图来放大，节约空间
    lv_img_set_zoom(bgimg, 512);
#endif
    lv_obj_align(bgimg, LV_ALIGN_CENTER, 0, 0);

    //右上齿轮
    lv_obj_t *right_top_gear = lv_img_create(lv_scr_act());
    lv_img_set_src(right_top_gear, &Horoscope03);
#if LIYUE_VER
    lv_obj_align(right_top_gear, LV_ALIGN_CENTER, 27, 3);
#else
    lv_obj_align(right_top_gear, LV_ALIGN_CENTER, 22, -17);
#endif
    //左下大齿轮
    lv_obj_t *left_btm_gear = lv_img_create(lv_scr_act());
    lv_img_set_src(left_btm_gear, &Horoscope04);

#if LIYUE_VER
    lv_obj_align(left_btm_gear, LV_ALIGN_CENTER, -37, -9);
#else
    lv_obj_align(left_btm_gear, LV_ALIGN_CENTER, -33, 20);
#endif
    //中心齿轮
    lv_obj_t *center_gear = lv_img_create(lv_scr_act());
    lv_img_set_src(center_gear, &Horoscope06);
    lv_obj_align(center_gear, LV_ALIGN_CENTER, 0, 0);

    //右下中齿轮
    lv_obj_t *right_btm_gear = lv_img_create(lv_scr_act());
    lv_img_set_src(right_btm_gear, &Horoscope05);

#if LIYUE_VER
    lv_obj_align(right_btm_gear, LV_ALIGN_CENTER, 28, 110);
#else
    lv_obj_align(right_btm_gear, LV_ALIGN_CENTER, 97, 59);
#endif
    //顶层遮罩
    lv_obj_t *mask_img = lv_img_create(lv_scr_act());
#if LIYUE_VER
    lv_img_set_src(mask_img, &star_mask_liyue);
#else
    lv_img_set_src(mask_img, &star_mask);
#endif
    lv_obj_align(mask_img, LV_ALIGN_CENTER, 0, 0);

    //齿轮动画效果
    lv_anim_init(&anim_center_gear);
    lv_anim_set_var(&anim_center_gear, center_gear);
    lv_anim_set_exec_cb(&anim_center_gear, rotate_img);
    lv_anim_set_values(&anim_center_gear, 3600, 0);
    lv_anim_set_time(&anim_center_gear, 75000);
    lv_anim_set_repeat_count(&anim_center_gear, LV_ANIM_REPEAT_INFINITE);

    lv_anim_init(&anim_left_btm_gear);
    lv_anim_set_var(&anim_left_btm_gear, left_btm_gear);
    lv_anim_set_exec_cb(&anim_left_btm_gear, rotate_img);
    lv_anim_set_values(&anim_left_btm_gear, 3600, 0);
    lv_anim_set_time(&anim_left_btm_gear, 75000);
    lv_anim_set_repeat_count(&anim_left_btm_gear, LV_ANIM_REPEAT_INFINITE);

    lv_anim_init(&anim_right_top_gear);
    lv_anim_set_var(&anim_right_top_gear, right_top_gear);
    lv_anim_set_exec_cb(&anim_right_top_gear, rotate_img);
    lv_anim_set_values(&anim_right_top_gear, 3600, 0);
    lv_anim_set_time(&anim_right_top_gear, 75000);
    lv_anim_set_repeat_count(&anim_right_top_gear, LV_ANIM_REPEAT_INFINITE);

    lv_anim_init(&anim_right_btm_gear);
    lv_anim_set_var(&anim_right_btm_gear, right_btm_gear);
    lv_anim_set_exec_cb(&anim_right_btm_gear, rotate_img);
    lv_anim_set_values(&anim_right_btm_gear, 0, 3600);
    lv_anim_set_time(&anim_right_btm_gear, 50000); //根据齿比计算得，为了让齿轮咬合完全一致
    lv_anim_set_repeat_count(&anim_right_btm_gear, LV_ANIM_REPEAT_INFINITE);

    lv_anim_start(&anim_center_gear);
    lv_anim_start(&anim_left_btm_gear);
    lv_anim_start(&anim_right_top_gear);
    lv_anim_start(&anim_right_btm_gear);
}

/**
 * @brief refresh函数
 *
 * @param
 */
void Genshin_Clock::refresh()
{
#if !USE_DS1302_RTC
    last_timestamp = currtimestamp;
#endif
    clock_time = getDateTime();
    uint8_t hr = clock_time.hour();
    if (hr == 0)
        hr = 12;
    else if (hr > 12)
        hr -= 12;
#if LIYUE_VER
    hPos = hr * 300 + clock_time.minute() * 5 + 450;
#else
    hPos = hr * 300 + clock_time.minute() * 5;
#endif
    lv_img_set_angle(lvHour, hPos);
#if LIYUE_VER
    mPos = clock_time.minute() * 60 + 450;
#else
    mPos = clock_time.minute() * 60;
#endif
    lv_img_set_angle(lvMinute, mPos);
#if LIYUE_VER
    lv_img_set_angle(lvSecond, clock_time.second() * 60 + 450);
#else
    lv_img_set_angle(lvSecond, clock_time.second() * 60);
#endif

#if LIYUE_VER
    if (clock_time.minute() != last_m)
    {
        last_m = clock_time.minute();
        calcOpa();
        lv_obj_set_style_img_opa(lvMoonrise, opa[0], 0);
        lv_obj_set_style_img_opa(lvSunrise, opa[1], 0);
        lv_obj_set_style_img_opa(lvDaytime, opa[2], 0);
        lv_obj_set_style_img_opa(lvSunset, opa[3], 0);
    }

#endif

}

/**
 * @brief specRefresh函数
 *
 * @param
 */
void Genshin_Clock::specRefresh()
{
    mPos += 36;
    if (mPos >= 3600)
        mPos -= 3600;
    hPos += 3;
    if (hPos >= 3600)
        hPos -= 3600;
    lv_img_set_angle(lvHour, hPos);
    lv_img_set_angle(lvMinute, mPos);
    lv_arc_set_bg_start_angle(arc, mPos < 900 ? 270 + mPos / 10 : (mPos - 900) / 10);
    lv_arc_set_bg_end_angle(arc, hPos < 900 ? 270 + hPos / 10 : (hPos - 900) / 10);
}

/**
 * @brief loop函数
 *
 * @param
 */
void Genshin_Clock::loop()
{
    if (clockMode == 0)
    {
        if (isTimeChanged())
        {
            refresh();
        }
    }
    else if (clockMode == 1)
    {
        specRefresh();
    }
}

/**
 * @brief 给时钟芯片和系统时间设置时间信息
 *
 * @param timestamp
 */
void Genshin_Clock::setDateTime(uint32_t timestamp)
{
#if USE_DS1302_RTC
    ds1302.adjust(DateTime(timestamp));
#else
    currtimestamp = timestamp;
#endif
}

/**
 * @brief 返回系统时间
 *
 * @param
 */
DateTime Genshin_Clock::getDateTime()
{
#if USE_DS1302_RTC
    return ds1302.now();
#else
    return DateTime(currtimestamp);
#endif
}

/**
 * @brief 判断系统时间是否等于时钟显示的时间
 *
 * @param
 */
bool Genshin_Clock::isTimeChanged()
{
#if USE_DS1302_RTC
    return ds1302.now() != clock_time;
#else
    return last_timestamp != currtimestamp;
#endif
}

#if !USE_DS1302_RTC
void Genshin_Clock::timestampAdd()
{
    currtimestamp++;
}
#endif

/**
 * @brief 获取系统时间戳
 *
 * @param
 */
uint32_t Genshin_Clock::getTimestamp()
{
#if USE_DS1302_RTC
    return ds1302.now().unixtime();
#else
    return currtimestamp;
#endif
}

/**
 * @brief 是否已初始化时钟
 *
 */
bool Genshin_Clock::isClockInit()
{
    return _isClockInit;
}

/**
 * @brief 改变时钟模式
 */
void Genshin_Clock::changeClockMode()
{
    changeClockMode(clockMode == 0 ? 1 : 0);
}

/**
 * @brief 改变时钟模式
 */
void Genshin_Clock::changeClockMode(uint8_t mode)
{
    if (mode == clockMode)
    {
        return;
    }
    clockMode = mode;
    if (clockMode == 0)
    {
        //还原速度
        lv_anim_set_time(&anim_center_gear, 75000);
        lv_anim_set_time(&anim_left_btm_gear, 75000);
        lv_anim_set_time(&anim_right_top_gear, 75000);
        lv_anim_set_time(&anim_right_btm_gear, 50000);
        lv_anim_start(&anim_center_gear);
        lv_anim_start(&anim_left_btm_gear);
        lv_anim_start(&anim_right_top_gear);
        lv_anim_start(&anim_right_btm_gear);
        lv_obj_add_flag(arc, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(lvSecond, LV_OBJ_FLAG_HIDDEN);
    }
    else if (clockMode == 1)
    {
        //加快背景齿轮运动速度，因为实在是太卡了，所以只提升到5倍速
        //如要完全还原游戏中的快进速度，需要提升到10倍，也就是7500，5000
        lv_anim_set_time(&anim_center_gear, 15000);
        lv_anim_set_time(&anim_left_btm_gear, 15000);
        lv_anim_set_time(&anim_right_top_gear, 15000);
        lv_anim_set_time(&anim_right_btm_gear, 10000);
        lv_anim_start(&anim_center_gear);
        lv_anim_start(&anim_left_btm_gear);
        lv_anim_start(&anim_right_top_gear);
        lv_anim_start(&anim_right_btm_gear);

        //隐藏秒针
        lv_obj_add_flag(lvSecond, LV_OBJ_FLAG_HIDDEN);
        //显示圆弧
        lv_obj_clear_flag(arc, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief 获取时钟模式
 */
uint8_t Genshin_Clock::getClockMode()
{
    return clockMode;
}
