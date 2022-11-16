// Code by JeeLabs http://news.jeelabs.org/code/
// Released to the public domain! Enjoy!

#pragma once

#if (ARDUINO >= 100)
  #include <Arduino.h>
#else
  #include <WProgram.h>
#endif

#include <Wire.h>

#ifdef __AVR__
  #include <avr/pgmspace.h>
  #define WIRE Wire
#elif defined ESP8266
  #include <pgmspace.h>
  #define WIRE Wire
#elif defined ARDUINO_RASPBERRY_PI_PICO
  #include <pgmspace.h>
  #define WIRE Wire
  #define BUFFER_LENGTH WIRE_BUFFER_SIZE
#else
  #define PROGMEM
  #define pgm_read_byte(addr) (*(const unsigned char*)(addr))
  #define WIRE Wire1
#endif

#define DS1302_RAMSIZE 31 // bytes
#define DS1307_RAMSIZE 56 // bytes

#define SECONDS_PER_DAY 86400L
#define SECONDS_FROM_1970_TO_2000 946684800L


struct alarm_flags {
    char minute;
    char hour;
    char day;
    char wday;
};

// TimeDelta which can represent changes in time with seconds accuracy.
// TODO: handle negative delta
class TimeDelta {
public:
    TimeDelta(uint32_t seconds = 0, bool neg = false);
    TimeDelta(uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds);
    TimeDelta(const TimeDelta& copy);
    uint16_t days() const { return _sec / 86400L; }
    uint8_t hours() const { return _sec / 3600 % 24; }
    uint8_t minutes() const { return _sec / 60 % 60; }
    uint8_t seconds() const { return _sec % 60; }
    int32_t totalseconds() const { return _sec; }

    bool operator==(const TimeDelta& td) const;
    bool operator!=(const TimeDelta& td) const;
    bool operator>(const TimeDelta& td) const;
    bool operator<(const TimeDelta& td) const;
    bool operator>=(const TimeDelta& td) const;
    bool operator<=(const TimeDelta& td) const;

    TimeDelta operator+(uint32_t t) const;
    TimeDelta operator+(const TimeDelta& td) const;
    TimeDelta operator-(uint32_t t) const;
    TimeDelta operator-(const TimeDelta& td) const;

    TimeDelta& operator+=(uint32_t t);
    TimeDelta& operator+=(const TimeDelta& td);
    TimeDelta& operator-=(uint32_t t);
    TimeDelta& operator-=(const TimeDelta& td);

protected:
    uint32_t _sec;
};

// Simple general-purpose date/time class (no TZ / DST / leap second handling!)
class DateTime {
public:
    char* format(char* ret);
    char* tostr(char* charr);
    DateTime(uint32_t t = 0);
    DateTime(uint16_t year, uint8_t month, uint8_t day,
             uint8_t hour = 0, uint8_t min = 0, uint8_t sec = 0);
    DateTime(const char* date, const char* time);
    DateTime(const __FlashStringHelper* date, const __FlashStringHelper* time);
    DateTime(const char* sdate); // Do we really need this?
    uint16_t year() const { return 2000 + yOff; }
    uint8_t month() const { return m; }
    uint8_t day() const { return d; }
    uint8_t hour() const { return hh; }
    uint8_t minute() const { return mm; }
    uint8_t second() const { return ss; }
    uint8_t dayOfWeek() const;
    void SetTime(const char* time);
    void SetDate(const char* date);
    void setyear(uint16_t year) { yOff = year - (year >= 2000 ? 2000 : 0); }
    void setmonth(uint8_t month) { m = month; }
    void setday(uint8_t day) { d = day; }
    void sethour(uint8_t hour) { hh = hour % 24; }
    void setminute(uint8_t minute) { mm = minute % 60; }
    void setsecond(uint8_t second) { ss = second % 60; }
    // 32-bit UNIX timestamp
    // An uint32_t should be able to store up to 2106,
    // which is beyond most chip's upper bound 2099
    void setunixtime(uint32_t t);
    uint32_t unixtime() const;

    bool operator==(const DateTime& date) const;
    bool operator==(const char* sdate) const;
    bool operator!=(const DateTime& date) const;
    bool operator!=(const char* sdate) const;
    bool operator<(const DateTime& date) const;
    bool operator>(const DateTime& date) const;
    bool operator<=(const DateTime& date) const;
    bool operator>=(const DateTime& date) const;

    DateTime operator+(uint32_t t) const;
    DateTime operator+(const TimeDelta& delta) const;
    DateTime operator-(uint32_t t) const;
    DateTime operator-(const TimeDelta& delta) const;
    TimeDelta operator-(const DateTime& date) const;

    DateTime& operator+=(uint32_t t);
    DateTime& operator+=(const TimeDelta& delta);
    DateTime& operator-=(uint32_t t);
    DateTime& operator-=(const TimeDelta& delta);

protected:
    uint8_t yOff, m, d, hh, mm, ss;
};

// RTC based on the DS1302 chip connected via pins
class DS1302 {
    // RAII class for data transferring
    class TransferHelper {
    public:
        TransferHelper(uint8_t ce_pin, uint8_t sck_pin);
        ~TransferHelper();

    protected:
        uint8_t ce, sck;

        const static uint8_t ce_to_sck_setup = 4;
        const static uint8_t ce_inactive_time = 4;
    };

    uint8_t read();
    void write(uint8_t val);

public:
    DS1302(uint8_t ce_pin = 4, uint8_t sck_pin = 5, uint8_t io_pin = 6);
    uint8_t read(uint8_t addr);
    void write(uint8_t addr, uint8_t val);

    void begin();
    uint8_t isrunning();
    DateTime now();
    void adjust(const DateTime& dt);
    uint8_t readram(uint8_t addr);
    void writeram(uint8_t addr, uint8_t val);
    uint8_t* getram(uint8_t* arr, uint8_t len);
    void putram(const uint8_t* arr, uint8_t len);

protected:
    uint8_t ce, sck, io;
};

// RTC based on the DS1307 chip connected via I2C and the Wire library
class DS1307 {
public:
    uint8_t begin();
    void adjust(const DateTime& dt);
    uint8_t isrunning();
    DateTime now();
    uint8_t read(uint8_t addr);
    void write(uint8_t addr, uint8_t val);
    uint8_t readram(uint8_t addr);
    void writeram(uint8_t addr, uint8_t val);
    uint8_t* getram(uint8_t* arr, uint8_t len);
    void putram(const uint8_t* arr, uint8_t len);
};

class DS3231 {
public:
    uint8_t begin();
    void adjust(const DateTime& dt);
    uint8_t isrunning();
    double getTemp();
    void set_alarm(const DateTime& dt); // TODO: implement DS3231 alarm
    DateTime now();
    uint8_t read(uint8_t addr);
    void write(uint8_t addr, uint8_t val);
};

class PCF8583 {
    int address;

public:
    PCF8583();
    PCF8583(int device_address);
    uint8_t begin();
    DateTime now();
    uint8_t isrunning();
    void adjust(const DateTime& dt);
    void off_alarm();
    DateTime get_alarm();
    void set_alarm(const DateTime& dt);
};

class PCF8563 {
    int address;
    int status1;
    int status2;

public:
    PCF8563();
    PCF8563(int device_address);
    uint8_t begin();
    DateTime now();
    uint8_t isrunning();
    uint8_t isvalid();
    void adjust(const DateTime& dt);
    void off_alarm();
    void on_alarm();
    DateTime get_alarm();
    void set_alarm(const DateTime& dt, alarm_flags flags);
};

// RTC using the internal millis() clock, has to be initialized before use
// NOTE: this clock won't be correct once the millis() timer rolls over (~49.7d)
// TODO: handle millis() overflow (if possible)
class RTC_Millis {
    bool running = false;

public:
    void begin();
    void begin(const DateTime& dt);
    void stop();
    void adjust(const DateTime& dt);
    DateTime now();
    bool isrunning();

protected:
    uint32_t offset;
};
