#include <lvgl.h>
#include <Arduino_GFX_Library.h>
#include <Ticker.h>
#include <OneButton.h>
#include "boardconf.h"
#include "BLE.h"
#include "file_io.h"
#include "clock.h"
#include "MjpegClass.h"
#include "esp32-hal-cpu.h"

#define EYES_FILE_COUNT 7 //总计多少个文件
#define DEFAULT_BRIGHTNESS 255

#if USE_ESP32S3
TaskHandle_t Task_Control;
#endif

uint8_t currPlay = 0;                     //当前播放的文件索引
bool isBreak = false;                     //是否播放中断
uint8_t playMode = 0;                     // 0顺序循环 1单个循环 2随机播放
uint16_t brightness = DEFAULT_BRIGHTNESS; //屏幕亮度
int16_t breath_step = 5;                  //亮度步长
uint8_t currCustomPlay = 0;               //自定义播放的当前索引
bool useBLE = true;                       //是否打开蓝牙BLE（省电）

const uint32_t screenWidth = 240;
const uint32_t screenHeight = 240;

String yan = "bcfhlsy";                         //神之眼文件名序列，如要自己定义请改这里和上面的FILE_COUNT
OneButton btn = OneButton(BTN_PIN, true, true); //初始化按键
Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO);

//初始化不同的屏幕
#if USE_LIYUE
Arduino_ST7789 *gfx = new Arduino_ST7789(bus, TFT_RST, 0, true, screenWidth, screenHeight);
#else
Arduino_GC9A01 *gfx = new Arduino_GC9A01(bus, TFT_RST, 0 , true); //第三个是旋转0=不旋转 1=90度 2=180度 3=270度
#endif

static MjpegClass mjpeg;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf;
static lv_disp_drv_t disp_drv;
static uint8_t *mjpeg_buf;

uint8_t currMode = 0; // 0神之眼 1时钟  2自定义文件（位于SD卡custom目录）

bool isReset = true;
bool isSDOK = false;
bool isFFOK = false;

Genshin_Clock myclock;

#if !USE_DS1302_RTC
Ticker timestampticker; //如果没有RTC模块，则采用Ticker模式
#endif

static bool isNumber(String s)
{
    bool re = true;
    for (int i = 0; i < s.length(); i++)
    {
        if (!isDigit(s[i]))
        {
            re = false;
            break;
        }
    }
    return re;
}

void disp_free_mem(String pre = "")
{
    if (pre != "")
        Serial.println(pre);
    Serial.printf("[FREE HEAP] INTL:%d,ALL:%d\n", esp_get_free_internal_heap_size(), esp_get_free_heap_size());
}

static int jpegDrawCallback(JPEGDRAW *pDraw)
{
#if (LV_COLOR_16_SWAP != 0)
    gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
#else
    gfx->draw16bitRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
#endif
    return 1;
}

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
    gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif

    lv_disp_flush_ready(disp);
}

void lvgl_init()
{
    Serial.println("[LVGL] Init start...");
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, screenWidth * screenHeight / LVGL_BUFFER_LEVEL);
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
    lv_obj_remove_style(lv_scr_act(), NULL, LV_PART_SCROLLBAR);
    Serial.println("[LVGL] Init finished.");
}

bool mem_alloc()
{
    //缓存分配到SRAM上，提高帧率
    disp_draw_buf = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * screenWidth * screenHeight / LVGL_BUFFER_LEVEL, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!disp_draw_buf)
    {
        Serial.println("[LVGL] Draw buff allocate failed!");
        gfx->println("[LVGL] Draw buff allocate failed!");
        return false;
    }
    mjpeg_buf = (uint8_t *)heap_caps_malloc(screenWidth * screenHeight / 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!mjpeg_buf)
    {
        Serial.println(F("[MJPEG] Draw buff malloc failed!"));
        gfx->println("[MJPEG] Draw buff malloc failed!");
        return false;
    }
    return true;
}

void singleClickHandler()
{
    Serial.println("[BTN] Single Click Event");
    if (currMode == 0)
    {
        //神之眼模式
        if (playMode == 0 || playMode == 1)
        {
            if (++currPlay >= EYES_FILE_COUNT)
            {
                currPlay = 0;
            }
        }
        else if (playMode == 2)
        {
            currPlay = random(7);
        }
        isBreak = true;
    }
    else if (currMode == 1)
    {
        myclock.changeClockMode();
    }
    else if (currMode == 2)
    {
        //自定义播放模式，范围0-254
        if (++currCustomPlay > 254)
        {
            currCustomPlay = 0;
        }
    }
}

void longClickHandler()
{
    Serial.println("[BTN] Long Click Event");
    isBreak = true;
    isReset = true;
    if (++currMode > 2)
    {
        currMode = 0;
    }
}

void btn_loop()
{
    btn.tick();
}

bool ble_sendmsg(String msg = "")
{
    String rtnMsg = msg;
    //没有参数则回报当前状态
    if (rtnMsg == "")
        rtnMsg = "_,t" + String(myclock.getTimestamp()) + ",m" + String(currMode) + ",p" + String(currPlay) + ",b" + String(playMode) + ",c" + String(myclock.getClockMode());

    memset(BLEbuf, 0, 32);
    memcpy(BLEbuf, rtnMsg.c_str(), 32);
    Serial.print("[BLE] Send msg:");
    Serial.println(rtnMsg);
    pTxCharacteristic->setValue(BLEbuf);
    pTxCharacteristic->notify();
    return true;
}

//蓝牙处理逻辑
void ble_proc()
{
    //蓝牙收到的字符串前五个是没用的，之所以要留出来，是因为之前测试的时候偶尔前三个字符出错
    if (ble_rcv_data.length() < 8)
    {
        Serial.println("[BLE] Wrong data recieved");
        return;
    }
    if (ble_rcv_data[5] == 's' && ble_rcv_data.length() >= 17)
    {
        //同步时间
        Serial.println("[BLE] Sync time");
        String ts = ble_rcv_data.substring(7, 17);
        //判断是否是数字
        if (isNumber(ts))
        {
            myclock.setDateTime(ts.toInt());
        }
        else
        {
            Serial.println("[BLE] Wrong number");
        }
    }
    else if (ble_rcv_data[5] == 'y')
    {
        //切换当前播放的神之眼，参数是0 1 2 3 4 5 6
        String ts = ble_rcv_data.substring(7, 8);
        if (isNumber(ts))
        {
            currPlay = ts.toInt();
            if (currPlay >= EYES_FILE_COUNT)
                currPlay = 0;
            isBreak = true;
            ble_sendmsg();
        }
        else
        {
            Serial.println("[BLE] Wrong number");
        }
    }
    else if (ble_rcv_data[5] == 'p')
    {
        //切换神之眼播放模式，参数是0 1 2
        String ts = ble_rcv_data.substring(7, 8);
        if (isNumber(ts))
        {
            playMode = ts.toInt();
            if (playMode >= 3)
                playMode = 0;
            ble_sendmsg();
        }
        else
        {
            Serial.println("[BLE] Wrong number");
        }
    }
    else if (ble_rcv_data[5] == 'm')
    {
        //切换模式，参数是0 1 2
        String ts = ble_rcv_data.substring(7, 8);
        if (isNumber(ts))
        {
            int aa = ts.toInt();
            if (aa >= 0 && aa < 3)
            {
                currMode = aa;
                isReset = true;
                isBreak = true;
                ble_sendmsg();
            }
        }
        else
        {
            Serial.println("[BLE] Wrong number");
        }
    }
    else if (ble_rcv_data[5] == 'c')
    {
        //切换时钟模式
        if (currMode == 1)
        {
            myclock.changeClockMode();
            ble_sendmsg();
        }
        else
        {
            Serial.println("[BLE] Must in clock mode");
        }
    }
    else if (ble_rcv_data[5] == 'i')
    {
        ble_sendmsg();
    }
    else if (ble_rcv_data[5] == 'q')
    {
        //自定义播放模式PREV
        if (currCustomPlay > 0)
        {
            --currCustomPlay;
            isBreak = true;
        }
    }
    else if (ble_rcv_data[5] == 'h')
    {
        //自定义播放模式NEXT
        if (++currCustomPlay > 254)
        {
            currCustomPlay = 0;
        }
        isBreak = true;
    }
    else
    {
        Serial.println("[BLE] No action");
    }
}

void ble_loop()
{
    if (deviceConnected && ble_rcv_data.length() > 0)
    {
        ble_proc();
        ble_rcv_data = "";
    }
    if (!deviceConnected && oldDeviceConnected) // 没有新连接时
    {
        delay(500);
        start_ble(); // 重新开始广播
        oldDeviceConnected = deviceConnected;
    }
    if (deviceConnected && !oldDeviceConnected) // 正在连接时
    {
        oldDeviceConnected = deviceConnected;
        Serial.println("[BLE] Client connected");
    }
}

#if USE_ESP32S3 //蓝牙循环和按键循环用单独的核心0,S3版使用
void task_control(void *pvParameters)
{
    for (;;)
    {
        btn_loop();
        if (useBLE)
            ble_loop();
        vTaskDelay(1);
    }
}
#else //控制中断循环，ESP32版使用
void control_loop()
{
    btn_loop();
    if (useBLE)
        ble_loop();
}
#endif

void breath_brightness()
{
    if (brightness >= 265) //调高这个数值可以延长最高亮度的持续时间，更像呼吸的感觉
    {
        breath_step = -5;
    }
    else if (brightness <= 100)
    {
        breath_step = 5;
    }
    brightness += breath_step;
    ledcWrite(1, brightness > 255 ? 255 : brightness);
}

void play_loop()
{
    isBreak = false;
    File mjpegFile;
#if USE_ESP32S3
    if (!isSDOK & isFFOK)
    {
        //当SD卡未就绪而内置flash就绪时，播放内置flash上的单个文件
        mjpegFile = FFat.open("/blankeye", FILE_READ);
    }
    else
    {
        if (currMode == 2)
        {
            mjpegFile = SD_MMC.open("/custom/my" + String(currCustomPlay) + ".mjpeg", FILE_READ);
            if (!mjpegFile || mjpegFile.isDirectory())
            {
                currCustomPlay = 0;
                mjpegFile = SD_MMC.open("/custom/my" + String(currCustomPlay) + ".mjpeg", FILE_READ);
            }
        }
        else
        {
            mjpegFile = SD_MMC.open("/mjpeg/" + String(yan[currPlay]) + ".mjpeg", FILE_READ);
        }
    }
#else
    if (currMode == 2)
    {
        mjpegFile = SD.open("/custom/my" + String(currCustomPlay) + ".mjpeg", FILE_READ);
        if (!mjpegFile || mjpegFile.isDirectory())
        {
            currCustomPlay = 0;
            mjpegFile = SD.open("/custom/my" + String(currCustomPlay) + ".mjpeg", FILE_READ);
        }
    }
    else
    {
        mjpegFile = SD.open("/mjpeg/" + String(yan[currPlay]) + ".mjpeg", FILE_READ);
    }
#endif

    if (!mjpegFile || mjpegFile.isDirectory())
    {
        Serial.println("[MJPEG] Failed to open mjpeg file");
        gfx->fillScreen(BLACK);
        gfx->setTextColor(WHITE);
        gfx->setCursor(20, 100);
        if (currMode == 0)
        {
            gfx->println("No eyes mjpeg file '/mjpeg/b.mjpeg'.");
            gfx->println("Filename like  'b.mjpeg' 'f.mjpeg'...");
            gfx->println("Pls copy it to 'mjpeg' folder in TF card and reboot");
        }
        else if (currMode == 2)
        {
            gfx->println("No custom mjpeg file '/custom/my0.mjpeg'.");
            gfx->println("Filename like 'my0.mjpeg' 'my1.mjpeg'...");
            gfx->println("Pls copy it to 'custom' folder  in TF card and reboot");
        }
        delay(60000); //延迟一分钟
        currMode = 1; //跳转时钟模式
        isReset = true;
        return;
    }
    else
    {
        Serial.println(F("[MJPEG] Video start"));
        mjpeg.setup(
            &mjpegFile, mjpeg_buf, jpegDrawCallback, true /* useBigEndian */,
            0 /* x */, 0 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */);
        while (mjpegFile.available() && mjpeg.readMjpegBuf())
        {
            mjpeg.drawJpg();
            
#if !USE_ESP32S3
            control_loop(); //跳转蓝牙循环和按键循环
#endif
            if (currMode == 0)
            {
                breath_brightness();
            }
            if (isBreak)
            {
                Serial.println(F("[MJPEG] Video break;"));
                break;
            }
        }
        Serial.println(F("[MJPEG] Video end"));
        if (!isBreak)
        {
            if (currMode == 0)
            {
                //如果正常播放结束，则根据逻辑切换下一个播放文件
                if (playMode == 0)
                {
                    if (++currPlay >= EYES_FILE_COUNT)
                    {
                        currPlay = 0;
                    }
                }
                else if (playMode == 2)
                {
                    currPlay = random(7);
                }
                brightness = 100;
            }
        }
        mjpegFile.close();
        isBreak = false;
    }
}

void setup()
{
    delay(500);
    Serial.begin(115200);

#if USE_ESP32S3
    Serial.println("ESP32S3 init start...");
#else
    Serial.println("ESP32 init start...");
#endif

#ifdef TFT_BLK
    ledcAttachPin(TFT_BLK, 1);
    ledcSetup(1, 12000, 8);
    ledcWrite(1, brightness);
#endif
    gfx->begin();
    gfx->fillScreen(BLACK);
    gfx->setTextColor(WHITE);

    //持续长按以禁用蓝牙
    pinMode(BTN_PIN, INPUT_PULLUP);
    uint32_t pressTime = millis();
    while (digitalRead(BTN_PIN) == LOW)
    {
        if (millis() - pressTime > 1499)
        {
            useBLE = false;
            gfx->setCursor(10, 100);
            gfx->setTextSize(2);
            gfx->println("BLE disabled.");
            delay(2000);
            break;
        }
        delay(50);
    }
    gfx->setTextSize(1);

#if USE_ESP32S3
    isSDOK = sdmmc_init();
    isFFOK = fatfs_init();
#else
    isSDOK = sd_init();
#endif

    if (!isSDOK && !isFFOK)
    {
        currMode = 1; // SD卡和FatFs初始失败，跳转时钟模式
    }

    // disp_free_mem();
#if !USE_ESP32S3
    //释放经典蓝牙的内存（14KB左右），优化ESP32内存
    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    Serial.println("Release BT classic mem");
#endif

    btn.reset();
    btn.setPressTicks(600); //按住600毫秒进入长按状态
    btn.attachClick(singleClickHandler);
    btn.attachLongPressStop(longClickHandler);

#if !USE_DS1302_RTC
    timestampticker.attach(1, Genshin_Clock::timestampAdd); //没有时钟芯片用Ticker计时
#endif
    if (!mem_alloc()) //给mjpeg和lvgl分配内存
    {
        Serial.println("Memory alloc failed!");
        gfx->println("Memory alloc failed!");
        delay(30000);
        ESP.restart();
    }
    // disp_free_mem("MEM OK");
    lvgl_init();
    if (useBLE)
        init_ble();

#if USE_ESP32S3
    //按键及蓝牙用CORE 0执行
    xTaskCreatePinnedToCore(task_control, "Task_Control", 4096, NULL, 1, &Task_Control, 0);
    Serial.println("[TASK] CONTROL USE CORE 0");
#endif
    disp_free_mem("INIT OK");
}

void loop()
{
    if (currMode == 1) //时钟模式
    {
        if (isReset)
        {
            isReset = false;
            brightness = DEFAULT_BRIGHTNESS;
            ledcWrite(1, brightness); //神之眼的呼吸模式会调整亮度，变回时钟时需重置亮度
            if (!myclock.isClockInit())
            {
                myclock.initClock();
            }
            //disp_free_mem("CLOCK MODE");
            if (getCpuFrequencyMhz() != 240)
            {
                setCpuFrequencyMhz(240);
            }
        }
        myclock.loop();
        lv_timer_handler();
    }
    else if (currMode == 0) //神之眼
    {
        //播放神之眼
        if (isReset)
        {
            isReset = false;
            if (!isSDOK)
            {
#if USE_ESP32S3
                isSDOK = sdmmc_init();
#else
                isSDOK = sd_init();
#endif
            }
            //disp_free_mem("EYE MODE");
            if (getCpuFrequencyMhz() != 160)
            {
                setCpuFrequencyMhz(160);
            }
        }
        play_loop();
    }
    else if (currMode == 2) //播放自定义文件
    {
        if (isReset)
        {
            isReset = false;
            brightness = DEFAULT_BRIGHTNESS;
            ledcWrite(1, brightness); //神之眼的呼吸模式会调整亮度，其他模式需重置亮度
            if (!isSDOK)
            {
#if USE_ESP32S3
                isSDOK = sdmmc_init();
#else
                isSDOK = sd_init();
#endif
            }
            if (getCpuFrequencyMhz() != 160)
            {
                setCpuFrequencyMhz(160);
            }
        }
        play_loop();
    }
#if !USE_ESP32S3
    control_loop();
#endif
}
