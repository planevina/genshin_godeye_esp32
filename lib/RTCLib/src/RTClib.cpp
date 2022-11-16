// Code by JeeLabs http://news.jeelabs.org/code/
// Released to the public domain! Enjoy!

#include "RTClib.h"

#define RTCLIB_WIRE Wire

#ifndef BUFFER_LENGTH
  #define BUFFER_LENGTH 32
#endif

#define DS1307_ADDRESS            0x68
#define PCF8563_ADDRESS           0xA3
#define PCF8583_ADDRESS           0xA0

#define PCF8563_ALARM_REG_OFF     0x80
#define PCF8563_ALARM_AIE         0x02
#define PCF8563_ALARM_AF          0x08 // 0x08 : not 0x04!!!!
// optional val for no alarm setting
#define PCF8563_NO_ALARM          0x99

// i2c slave address of the DS3231 chip
#define DS3231_ADDRESS            0x68

// registers
#define DS1307_TIME_ADDR          0x00
#define DS1307_RAM_ADDR           0x08
#define DS3231_TIME_ADDR          0x00
#define DS3231_ALARM_1_ADDR       0x07
#define DS3231_ALARM_2_ADDR       0x0B
#define DS3231_CONTROL_ADDR       0x0E
#define DS3231_STATUS_ADDR        0x0F
#define DS3231_AGING_OFFSET_ADDR  0x10
#define DS3231_TEMPERATURE_ADDR   0x11

// control bits
#define DS3231_ALARM_1_IE         0x1
#define DS3231_ALARM_2_IE         0x2
#define DS3231_INTC               0x4

// status bits
#define DS3231_ALARM_1_AF         0x1
#define DS3231_ALARM_2_AF         0x2
#define DS3231_OSF                0x80

////////////////////////////////////////////////////////////////////////////////
// utility code, some of this could be exposed in the DateTime API if needed
static uint8_t bcd2bin (uint8_t val) { return val - 6 * (val >> 4); }
static uint8_t bin2bcd (uint8_t val) { return val + 6 * (val / 10); }

const uint8_t daysInMonth[] PROGMEM = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

// number of days since 2000/01/01, valid for 2001..2099
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) {
    if (y >= 2000)
        y -= 2000;
    uint16_t days = d;
    for (uint8_t i = 1; i < m; i++)
        days += pgm_read_byte(daysInMonth + i - 1);
    if (m > 2 && y % 4 == 0)
        days++;
    return days + 365 * y + (y + 3) / 4 - 1;
}

static inline long time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
    return ((days * 24L + h) * 60 + m) * 60 + s;
}

static uint8_t conv2d(const char* p) {
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}

static uint8_t _read(int dev, uint8_t addr) {
    RTCLIB_WIRE.beginTransmission(dev);
    RTCLIB_WIRE.write(addr);
    RTCLIB_WIRE.endTransmission();
    RTCLIB_WIRE.requestFrom(dev, 1);
    uint8_t s = RTCLIB_WIRE.read();
    return s;
}

static void _write(int dev, uint8_t addr, uint8_t val) {
    RTCLIB_WIRE.beginTransmission(dev);
    RTCLIB_WIRE.write(addr);
    RTCLIB_WIRE.write(val);
    RTCLIB_WIRE.endTransmission();
}

////////////////////////////////////////////////////////////////////////////////
// DateTime implementation - ignores time zones and DST changes
// NOTE: also ignores leap seconds, see http://en.wikipedia.org/wiki/Leap_second
DateTime::DateTime(uint32_t t) {
    setunixtime(t);
}

DateTime::DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec) {
    if (year >= 2000)
        year -= 2000;
    yOff = year;
    m = month;
    d = day;
    hh = hour;
    mm = min;
    ss = sec;
}

// A convenient constructor for using "the compiler's time":
//   DateTime now(__DATE__, __TIME__);
// NOTE: using PSTR would further reduce the RAM footprint
DateTime::DateTime(const char* date, const char* time) {
    SetDate(date);
    SetTime(time);
}

// A convenient constructor for using "the compiler's time":
// This version will save RAM by using PROGMEM to store it by using the F macro.
// DateTime now(F(__DATE__), F(__TIME__));
DateTime::DateTime(const __FlashStringHelper* date, const __FlashStringHelper* time) {
    // sample input: date = "Dec 26 2009", time = "12:34:56"
    char _date[11];
    char _time[8];
    memcpy_P(_date, date, 11);
    memcpy_P(_time, time, 8);
    SetDate(_date);
    SetTime(_time);
}

DateTime::DateTime(const char* sdate) : DateTime(sdate, sdate + 11) {}

/*********************************************/
/*         Comparison & Modification         */
/*********************************************/

bool DateTime::operator==(const DateTime& date) const {
    return unixtime() == date.unixtime();
}

bool DateTime::operator==(const char* sdate) const {
    return *this == DateTime(sdate);
}

bool DateTime::operator!=(const DateTime& date) const {
    return unixtime() != date.unixtime();
}

bool DateTime::operator!=(const char* sdate) const {
    return *this != DateTime(sdate);
}

bool DateTime::operator<(const DateTime& date) const {
    return unixtime() < date.unixtime();
}

bool DateTime::operator>(const DateTime& date) const {
    return unixtime() > date.unixtime();
}

bool DateTime::operator<=(const DateTime& date) const {
    return unixtime() <= date.unixtime();
}

bool DateTime::operator>=(const DateTime& date) const {
    return unixtime() >= date.unixtime();
}

DateTime DateTime::operator+(uint32_t t) const {
    return DateTime(unixtime() + t);
}

DateTime DateTime::operator+(const TimeDelta& delta) const {
    return *this + delta.totalseconds();
}

DateTime DateTime::operator-(uint32_t t) const {
    return DateTime(unixtime() - t);
}

DateTime DateTime::operator-(const TimeDelta& delta) const {
    return *this - delta.totalseconds();
}

TimeDelta DateTime::operator-(const DateTime& date) const {
    return TimeDelta(unixtime() - date.unixtime());
}

DateTime& DateTime::operator+=(uint32_t t) {
    setunixtime(unixtime() + t);
    return *this;
}

DateTime& DateTime::operator+=(const TimeDelta& delta) {
    return *this += delta.totalseconds();
}

DateTime& DateTime::operator-=(uint32_t t) {
    setunixtime(unixtime() - t);
    return *this;
}

DateTime& DateTime::operator-=(const TimeDelta& delta) {
    return *this -= delta.totalseconds();
}

uint8_t DateTime::dayOfWeek() const {
    uint16_t day = date2days(yOff, m, d);
    return (day + 6) % 7; // Jan 1, 2000 is a Saturday, i.e. returns 6
}

void DateTime::SetTime(const char* time) {
    hh = conv2d(time);
    mm = conv2d(time + 3);
    ss = conv2d(time + 6);
}

void DateTime::SetDate(const char* date) {
    // sample input: date = "Dec 26 2009", time = "12:34:56"
    // or date = "26-12-2009"
    yOff = conv2d(date + 9);
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    d = conv2d(date + 4);
    switch (date[0]) {
        case 'J': m = date[1] == 'a' ? 1 : date[2] == 'n' ? 6 : 7; break;
        case 'F': m = 2; break;
        case 'A': m = date[1] == 'p' ? 4 : 8; break;
        case 'M': m = date[2] == 'r' ? 3 : 5; break;
        case 'S': m = 9; break;
        case 'O': m = 10; break;
        case 'N': m = 11; break;
        case 'D': m = 12; break;
        default:
            yOff = conv2d(date + 8);
            m = conv2d(date + 3);
            d = conv2d(date);
    }
}

void DateTime::setunixtime(uint32_t t) {
    t -= SECONDS_FROM_1970_TO_2000; // bring to 2000 timestamp from 1970

    ss = t % 60;
    t /= 60;
    mm = t % 60;
    t /= 60;
    hh = t % 24;
    uint16_t days = t / 24;
    uint8_t leap;
    for (yOff = 0; ;yOff++) {
        leap = yOff % 4 == 0;
        if (days < 365u + leap)
            break;
        days -= 365 + leap;
    }

    for (m = 1; ; m++) {
        uint8_t daysPerMonth = pgm_read_byte(daysInMonth + m - 1);
        if (leap && m == 2)
            daysPerMonth++;
        if (days < daysPerMonth)
            break;
        days -= daysPerMonth;
    }

    d = days + 1;
}

uint32_t DateTime::unixtime() const {
    uint16_t days = date2days(yOff, m, d);
    uint32_t t = time2long(days, hh, mm, ss);
    t += SECONDS_FROM_1970_TO_2000;

    return t;
}

// TODO follow strftime format, could be difficult
char* DateTime::format(char* ret) {
    for (size_t i = 0; i < strlen(ret); i++) {
        if (ret[i] == 'h' && ret[i + 1] == 'h') {
            ret[i] = '0' + hh / 10;
            ret[i + 1] = '0' + hh % 10;
        }

        if (ret[i] == 'm' && ret[i + 1] == 'm') {
            ret[i] = '0' + mm / 10;
            ret[i + 1] = '0' + mm % 10;
        }

        if (ret[i] == 's' && ret[i + 1] == 's') {
            ret[i] = '0' + ss / 10;
            ret[i + 1] = '0' + ss % 10;
        }

        if (ret[i] == 'D' && ret[i + 1] == 'D') {
            ret[i] = '0' + d / 10;
            ret[i + 1] = '0' + d % 10;
        }

        if (ret[i] == 'M' && ret[i + 1] == 'M') {
            ret[i] = '0' + m / 10;
            ret[i + 1] = '0' + m % 10;
        }

        if (ret[i] == 'Y') {
            if (ret[i + 3] == 'Y') {
                ret[i] = '2';
                ret[i + 1] = '0';
                ret[i + 2] = '0' + (yOff / 10) % 10;
                ret[i + 3] = '0' + yOff % 10;
            }
            else if (ret[i + 1] == 'Y') {
                ret[i] = '0' + (yOff / 10) % 10;
                ret[i + 1] = '0' + yOff % 10;
            }
        }
    }
    return ret;
}

char* DateTime::tostr(char* charr) {
    charr[0] = '2'; charr[1] = '0';
    charr[4] = charr[7] = '/';
    charr[13] = charr[16] = ':';
    charr[10] = ' ';
    charr[2] = '0' + yOff / 10; charr[3] = '0' + yOff % 10;
    charr[5] = '0' + m / 10; charr[6] = '0' + m % 10;
    charr[8] = '0' + d / 10; charr[9] = '0' + d % 10;
    charr[11] = '0' + hh / 10; charr[12] = '0' + hh % 10;
    charr[14] = '0' + mm / 10; charr[15] = '0' + mm % 10;
    charr[17] = '0' + ss / 10; charr[18] = '0' + ss % 10;
    charr[19] = '\0';

    return charr;
}

////////////////////////////////////////////////////////////////////////////////
// TimeDelta implementation
TimeDelta::TimeDelta (uint32_t seconds, bool neg) {
    _sec = seconds;
}

TimeDelta::TimeDelta(uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds) {
    _sec = days * 86400L + hours * 3600 + minutes * 60 + seconds;
}

TimeDelta::TimeDelta(const TimeDelta& copy) {
    _sec = copy._sec;
}

bool TimeDelta::operator==(const TimeDelta& td) const {
    return totalseconds() == td.totalseconds();
}

bool TimeDelta::operator!=(const TimeDelta& td) const {
    return totalseconds() != td.totalseconds();
}

bool TimeDelta::operator<(const TimeDelta& td) const {
    return totalseconds() < td.totalseconds();
}

bool TimeDelta::operator>(const TimeDelta& td) const {
    return totalseconds() > td.totalseconds();
}

bool TimeDelta::operator<=(const TimeDelta& td) const {
    return totalseconds() <= td.totalseconds();
}

bool TimeDelta::operator>=(const TimeDelta& td) const {
    return totalseconds() >= td.totalseconds();
}

TimeDelta TimeDelta::operator+(uint32_t t) const {
    return TimeDelta(_sec + t);
}

TimeDelta TimeDelta::operator+(const TimeDelta& delta) const {
    return *this + delta._sec;
}

TimeDelta TimeDelta::operator-(uint32_t t) const {
    return TimeDelta(_sec - t);
}

TimeDelta TimeDelta::operator-(const TimeDelta& delta) const {
    return *this - delta._sec;
}

////////////////////////////////////////////////////////////////////////////////
// RTC_DS1302 implementation
DS1302::TransferHelper::TransferHelper(uint8_t ce_pin, uint8_t sck_pin) {
    ce = ce_pin;
    sck = sck_pin;

    digitalWrite(sck, LOW);
    digitalWrite(ce, HIGH);

    delayMicroseconds(ce_to_sck_setup);
}

DS1302::TransferHelper::~TransferHelper() {
    digitalWrite(ce, LOW);

    delayMicroseconds(ce_inactive_time);
}

DS1302::DS1302(uint8_t ce_pin, uint8_t sck_pin, uint8_t io_pin) {
    ce = ce_pin;
    sck = sck_pin;
    io = io_pin;
}

uint8_t DS1302::read() {
    pinMode(io, INPUT);
    // FIXME: this works while shiftIn() don't - Issue #31
    uint8_t value = 0;
    for (uint8_t i = 0; i < 8; ++i) {
        uint8_t bit = digitalRead(io);
        value |= (bit << i);  // Bits are read LSB first.
        digitalWrite(sck, HIGH);
        digitalWrite(sck, LOW);
    }
    return value;
}

void DS1302::write(uint8_t val) {
    pinMode(io, OUTPUT);
    shiftOut(io, sck, LSBFIRST, val);
}

void DS1302::begin() {
    pinMode(ce, OUTPUT);
    pinMode(sck, OUTPUT);
    pinMode(io, INPUT);
    write(7, 0);
    uint8_t sec = read(0);
    if (sec & 0x80) {
        sec &= 0x7F;
        write(0, sec);
    }
}

uint8_t DS1302::read(uint8_t addr) {
    TransferHelper data_transfer(ce, sck);

    write((0x81 | (addr << 1)));
    return read();
}

void DS1302::write(uint8_t addr, uint8_t val) {
    TransferHelper data_transfer(ce, sck);

    write((0x80 | (addr << 1)));
    write(val);
}

uint8_t DS1302::isrunning() {
    return !(read(0) >> 7);
}

DateTime DS1302::now() {
    TransferHelper data_transfer(ce, sck);

    write(0xBF); // Clock Burst Read
    uint8_t ss = bcd2bin(read() & 0x7F);
    uint8_t mm = bcd2bin(read());
    uint8_t hh = bcd2bin(read());
    uint8_t d = bcd2bin(read());
    uint8_t m = bcd2bin(read());
    read(); // ignore dayOfWeek register since we have our own method
    uint8_t yOff = bcd2bin(read());
    return DateTime(yOff + 2000, m, d, hh, mm, ss);
}

void DS1302::adjust(const DateTime& dt) {
    TransferHelper data_transfer(ce, sck);

    write(0xBE); // Clock Burst Write
    write(bin2bcd(dt.second()));
    write(bin2bcd(dt.minute()));
    write(bin2bcd(dt.hour()));
    write(bin2bcd(dt.day()));
    write(bin2bcd(dt.month()));
    write(bin2bcd(dt.dayOfWeek()));
    write(bin2bcd(dt.year() - 2000));
    write(0);
}

uint8_t DS1302::readram(uint8_t addr) {
    addr %= DS1302_RAMSIZE;

    TransferHelper data_transfer(ce, sck);

    write(0xC1 + 2 * addr);
    return read();
}

void DS1302::writeram(uint8_t addr, uint8_t val) {
    addr %= DS1302_RAMSIZE;

    TransferHelper data_transfer(ce, sck);

    write(0xC0 + 2 * addr);
    write(val);
}

uint8_t* DS1302::getram(uint8_t* arr, uint8_t len) {
    if (len > DS1302_RAMSIZE) {
        // Limit to avoid undefined operation
        len = DS1302_RAMSIZE;
    }
    TransferHelper data_transfer(ce, sck);

    write(0xFF); // RAM Burst Read
    for(uint8_t i = 0; i < len; i++) {
        *(arr + i) = read();
    }

    return arr;
}

void DS1302::putram(const uint8_t* arr, uint8_t len) {
    if (len > DS1302_RAMSIZE) {
        // Limit to avoid undefined operation
        len = DS1302_RAMSIZE;
    }
    TransferHelper data_transfer(ce, sck);

    write(0xFE); // RAM Burst Write
    for(uint8_t i = 0; i < len; i++) {
        write(*(arr + i));
    }
}

////////////////////////////////////////////////////////////////////////////////
// RTC_DS1307 implementation
uint8_t DS1307::begin() {
    return 1;
}

uint8_t DS1307::read(uint8_t addr) {
    return _read(DS1307_ADDRESS, addr);
}

void DS1307::write(uint8_t addr, uint8_t val) {
    _write(DS1307_ADDRESS, addr, val);
}

uint8_t DS1307::isrunning() {
    uint8_t ss = read(0);
    return !(ss>>7);
}

void DS1307::adjust(const DateTime& dt) {
    RTCLIB_WIRE.beginTransmission(DS1307_ADDRESS);
    RTCLIB_WIRE.write(0);
    RTCLIB_WIRE.write(bin2bcd(dt.second()));
    RTCLIB_WIRE.write(bin2bcd(dt.minute()));
    RTCLIB_WIRE.write(bin2bcd(dt.hour()));
    RTCLIB_WIRE.write(bin2bcd(dt.dayOfWeek()));
    RTCLIB_WIRE.write(bin2bcd(dt.day()));
    RTCLIB_WIRE.write(bin2bcd(dt.month()));
    RTCLIB_WIRE.write(bin2bcd(dt.year() - 2000));
    RTCLIB_WIRE.write(0);
    RTCLIB_WIRE.endTransmission();
}

DateTime DS1307::now() {
    RTCLIB_WIRE.beginTransmission(DS1307_ADDRESS);
    RTCLIB_WIRE.write(0);
    RTCLIB_WIRE.endTransmission();

    RTCLIB_WIRE.requestFrom(DS1307_ADDRESS, 7);
    uint8_t ss = bcd2bin(RTCLIB_WIRE.read() & 0x7F);
    uint8_t mm = bcd2bin(RTCLIB_WIRE.read());
    uint8_t hh = bcd2bin(RTCLIB_WIRE.read());
    RTCLIB_WIRE.read(); // ignore dayOfWeek register
    uint8_t d = bcd2bin(RTCLIB_WIRE.read());
    uint8_t m = bcd2bin(RTCLIB_WIRE.read());
    uint8_t yOff = bcd2bin(RTCLIB_WIRE.read());

    return DateTime(yOff + 2000, m, d, hh, mm, ss);
}

uint8_t DS1307::readram(uint8_t addr) {
    return _read(DS1307_ADDRESS, DS1307_RAM_ADDR + addr % DS1307_RAMSIZE);
}

void DS1307::writeram(uint8_t addr, uint8_t val) {
    _write(DS1307_ADDRESS, DS1307_RAM_ADDR + addr % DS1307_RAMSIZE, val);
}

uint8_t* DS1307::getram(uint8_t* arr, uint8_t len) {
    // Because Wire's internal buffer only has 32 bytes (BUFFER_LENGTH)
    // we need to read twice
    const uint8_t separator = BUFFER_LENGTH - 1;

    if (len > DS1307_RAMSIZE) {
        // Limit to avoid undefined operation
        len = DS1307_RAMSIZE;
    }

    // how many bytes to write first
    uint8_t first_wave = max(len, separator);

    RTCLIB_WIRE.beginTransmission(DS1307_ADDRESS);
    RTCLIB_WIRE.write(DS1307_RAM_ADDR);
    RTCLIB_WIRE.endTransmission();

    RTCLIB_WIRE.requestFrom(DS1307_ADDRESS, (int)first_wave);
    for(int i = 0; i < first_wave; i++) {
        arr[i] = RTCLIB_WIRE.read();
    }

    // if more data are present
    if (len > separator) {
        RTCLIB_WIRE.beginTransmission(DS1307_ADDRESS);
        RTCLIB_WIRE.write(DS1307_RAM_ADDR + separator);
        RTCLIB_WIRE.endTransmission();

        // read unread data
        RTCLIB_WIRE.requestFrom(DS1307_ADDRESS, len - separator);
        for (int i = separator; i < len; i++) {
            arr[i] = RTCLIB_WIRE.read();
        }
    }

    return arr;
}

void DS1307::putram(const uint8_t* arr, uint8_t len) {
    // Because Wire's internal buffer only has 32 bytes (BUFFER_LENGTH)
    // we need to write twice
    const uint8_t separator = BUFFER_LENGTH - 1;

    if (len > DS1307_RAMSIZE) {
        // Limit to avoid undefined operation
        len = DS1307_RAMSIZE;
    }

    // how many bytes to write first
    uint8_t first_wave = max(len, separator);

    RTCLIB_WIRE.beginTransmission(DS1307_ADDRESS);
    RTCLIB_WIRE.write(DS1307_RAM_ADDR);
    for (int i = 0; i < first_wave; i++) {
        RTCLIB_WIRE.write(*(arr + i));
    }
    RTCLIB_WIRE.endTransmission();

    // if more data are present
    if (len > separator) {
        // write unwritten data
        RTCLIB_WIRE.beginTransmission(DS1307_ADDRESS);
        RTCLIB_WIRE.write(DS1307_RAM_ADDR + separator);
        for (int i = separator; i < len; i++) {
            RTCLIB_WIRE.write(*(arr + i));
        }
        RTCLIB_WIRE.endTransmission();
    }
}

////////////////////////////////////////////////////////////////////////////////
// DS3231 implementation
uint8_t DS3231::begin() {
    write(DS3231_CONTROL_ADDR, DS3231_INTC);
    return 0;
}

uint8_t DS3231::read(uint8_t addr) {
    return _read(DS3231_ADDRESS, addr);
}

void DS3231::write(uint8_t addr, uint8_t val) {
    _write(DS3231_ADDRESS, addr, val);
}

uint8_t DS3231::isrunning() {
    uint8_t ss = read(0);
    return !(ss>>7);
}

void DS3231::adjust(const DateTime& dt) {
    uint8_t year, cen;
    if (dt.year() > 2000) {
        year = dt.year() - 2000;
        cen = 0x80;
    }
    else {
        year = dt.year() - 1900;
        cen = 0;
    }

    RTCLIB_WIRE.beginTransmission(DS3231_ADDRESS);
    RTCLIB_WIRE.write(DS3231_TIME_ADDR);
    RTCLIB_WIRE.write(bin2bcd(dt.second()));
    RTCLIB_WIRE.write(bin2bcd(dt.minute()));
    RTCLIB_WIRE.write(bin2bcd(dt.hour()));
    RTCLIB_WIRE.write(bin2bcd(0));
    RTCLIB_WIRE.write(bin2bcd(dt.day()));
    RTCLIB_WIRE.write(bin2bcd(dt.month())+ cen);
    RTCLIB_WIRE.write(bin2bcd(year));
    RTCLIB_WIRE.endTransmission();
}

DateTime DS3231::now() {
    RTCLIB_WIRE.beginTransmission(DS3231_ADDRESS);
    RTCLIB_WIRE.write(DS3231_TIME_ADDR);
    RTCLIB_WIRE.endTransmission();
    RTCLIB_WIRE.requestFrom(DS3231_ADDRESS, 7);
    uint8_t ss = bcd2bin(RTCLIB_WIRE.read() & 0x7F);
    uint8_t mm = bcd2bin(RTCLIB_WIRE.read());
    uint8_t hh = bcd2bin(RTCLIB_WIRE.read());
    RTCLIB_WIRE.read();
    uint8_t d = bcd2bin(RTCLIB_WIRE.read());
    uint8_t cen = RTCLIB_WIRE.read();
    uint8_t m = bcd2bin(cen & 0x1F);
    uint16_t y = bcd2bin(RTCLIB_WIRE.read());
    if ((cen & 0x80) >> 7 == 1) {
        y += 2000;
    }
    else {
        y += 1900;
    }
    return DateTime(y, m, d, hh, mm, ss);
}

double DS3231::getTemp() {
    double temp;
    RTCLIB_WIRE.beginTransmission(DS3231_ADDRESS);
    RTCLIB_WIRE.write(DS3231_TEMPERATURE_ADDR);
    RTCLIB_WIRE.endTransmission();
    RTCLIB_WIRE.requestFrom(DS3231_ADDRESS, 2);
    temp = (double) RTCLIB_WIRE.read();
    temp += 0.25 * (RTCLIB_WIRE.read() >> 6);
    return temp;
}

////////////////////////////////////////////////////////////////////////////////
// PCF8583 implementation
// provide device address as a full 8 bit address (like the datasheet)
PCF8583::PCF8583(int device_address) {
    address = device_address >> 1; // convert to 7 bit so Wire doesn't choke
}

PCF8583::PCF8583() {
    address = PCF8583_ADDRESS >> 1; // convert to 7 bit so Wire doesn't choke
}

// initialization
uint8_t PCF8583::begin() {
    _write(address, 0x00, 0x04); // Set alarm on int\ will turn to vcc
    return 1;
}

DateTime PCF8583::now() {
    RTCLIB_WIRE.beginTransmission(address);
    RTCLIB_WIRE.write(0xC0); // stop counting, don't mask
    RTCLIB_WIRE.endTransmission();

    RTCLIB_WIRE.beginTransmission(address);
    RTCLIB_WIRE.write(0x02);
    RTCLIB_WIRE.endTransmission();
    RTCLIB_WIRE.requestFrom(address, 5);

    uint8_t second = bcd2bin(RTCLIB_WIRE.read());
    uint8_t minute = bcd2bin(RTCLIB_WIRE.read());
    uint8_t hour = bcd2bin(RTCLIB_WIRE.read());
    uint8_t incoming = RTCLIB_WIRE.read(); // year/date counter
    uint8_t day = bcd2bin(incoming & 0x3f);
    uint8_t year = (int)((incoming >> 6) & 0x03); // it will only hold 4 years...
    incoming = RTCLIB_WIRE.read();
    uint8_t month = bcd2bin(incoming & 0x1f);
    // uint8_t dow = incoming >> 5; // unused

    // but that's not all - we need to find out what the base year is
    // so we can add the 2 bits we got above and find the real year
    RTCLIB_WIRE.beginTransmission(address);
    RTCLIB_WIRE.write(0x10);
    RTCLIB_WIRE.endTransmission();
    RTCLIB_WIRE.requestFrom(address, 2);

    uint8_t year_base = RTCLIB_WIRE.read();
    year_base <<= 8;
    year_base |= RTCLIB_WIRE.read();
    year += year_base;
    return DateTime(year, month, day, hour, minute, second);
}

void PCF8583::adjust(const DateTime& dt) {
    RTCLIB_WIRE.beginTransmission(address);
    RTCLIB_WIRE.write(0xC0); // stop counting, don't mask
    RTCLIB_WIRE.endTransmission();

    RTCLIB_WIRE.beginTransmission(address);
    RTCLIB_WIRE.write(0x02);
    RTCLIB_WIRE.write(bin2bcd(dt.second()));
    RTCLIB_WIRE.write(bin2bcd(dt.minute()));
    RTCLIB_WIRE.write(bin2bcd(dt.hour()));
    RTCLIB_WIRE.write(((uint8_t)(dt.year() - 2000) << 6) | bin2bcd(dt.day()));
    RTCLIB_WIRE.write((dt.dayOfWeek() << 5) | (bin2bcd(dt.month()) & 0x1f));
    RTCLIB_WIRE.endTransmission();

    RTCLIB_WIRE.beginTransmission(address);
    RTCLIB_WIRE.write(0x10);
    RTCLIB_WIRE.write(2000 >> 8);
    RTCLIB_WIRE.write(2000 & 0x00ff);
    RTCLIB_WIRE.endTransmission();

    begin(); // reset the control/status register to 0x04
}

uint8_t PCF8583::isrunning() {
    uint8_t ss = _read(address, 0);
    return !(ss>>7);
}

// Get the alarm at 0x09 adress
DateTime PCF8583::get_alarm() {
    RTCLIB_WIRE.beginTransmission(address);
    RTCLIB_WIRE.write(0x0A); // Set the register pointer to (0x0A)
    RTCLIB_WIRE.endTransmission();

    RTCLIB_WIRE.requestFrom(address, 4); // Read 4 values

    uint8_t second = bcd2bin(RTCLIB_WIRE.read());
    uint8_t minute = bcd2bin(RTCLIB_WIRE.read());
    uint8_t hour = bcd2bin(RTCLIB_WIRE.read());

    RTCLIB_WIRE.beginTransmission(address);
    RTCLIB_WIRE.write(0x0E);
    RTCLIB_WIRE.endTransmission();

    RTCLIB_WIRE.requestFrom(address, 1); // Read weekday value

    uint8_t day = bcd2bin(RTCLIB_WIRE.read());
    return DateTime(2000, 01, day, hour, minute, second);
}

// Set a daily alarm
void PCF8583::set_alarm(const DateTime& dt) {
    RTCLIB_WIRE.beginTransmission(address);
    RTCLIB_WIRE.write(0x08);
    RTCLIB_WIRE.write(0x90); // daily alarm set
    RTCLIB_WIRE.endTransmission();

    RTCLIB_WIRE.beginTransmission(address);
    RTCLIB_WIRE.write(0x09); // Set the register pointer to (0x09)
    RTCLIB_WIRE.write(0x00); // Set 00 at milisec
    RTCLIB_WIRE.write(bin2bcd(dt.second()));
    RTCLIB_WIRE.write(bin2bcd(dt.minute()));
    RTCLIB_WIRE.write(bin2bcd(dt.hour()));
    RTCLIB_WIRE.write(0x00); // Set 00 at day
    RTCLIB_WIRE.endTransmission();
}

void PCF8583::off_alarm() {
    RTCLIB_WIRE.beginTransmission(address);
    RTCLIB_WIRE.write(0x08);
    RTCLIB_WIRE.write(0x00); // off alarm set
    RTCLIB_WIRE.endTransmission();
}

////////////////////////////////////////////////////////////////////////////////
// PCF8563 implementation
// provide device address as a full 8 bit address (like the datasheet)
PCF8563::PCF8563(int device_address) {
    address = device_address >> 1; // convert to 7 bit so Wire doesn't choke
}

PCF8563::PCF8563() {
    address = PCF8563_ADDRESS >> 1; // convert to 7 bit so Wire doesn't choke
}

// initialization
uint8_t PCF8563::begin() {
    RTCLIB_WIRE.begin();
    RTCLIB_WIRE.beginTransmission(address);
    RTCLIB_WIRE.write(0x00); // Start address
    RTCLIB_WIRE.write(0); // Control and status 1
    RTCLIB_WIRE.write(0); // Control and status 2
    return RTCLIB_WIRE.endTransmission();
}

DateTime PCF8563::now() {
    RTCLIB_WIRE.beginTransmission(address);
    RTCLIB_WIRE.write(0x00);
    RTCLIB_WIRE.endTransmission();
    RTCLIB_WIRE.requestFrom(address, 9);

    status1 = RTCLIB_WIRE.read();
    status2 = RTCLIB_WIRE.read();
    uint8_t second = bcd2bin(RTCLIB_WIRE.read() & 0x7F);
    uint8_t minute = bcd2bin(RTCLIB_WIRE.read() & 0x7F);
    uint8_t hour = bcd2bin(RTCLIB_WIRE.read() & 0x3F);
    uint8_t day = bcd2bin(RTCLIB_WIRE.read() & 0x3F);
    RTCLIB_WIRE.read(); // year/date counter
    uint8_t month = RTCLIB_WIRE.read();
    // uint8_t century = month >> 7; // unused
    month = bcd2bin(month & 0x1F);
    uint8_t year = bcd2bin(RTCLIB_WIRE.read()); // it will only hold 4 years...
    return DateTime(year, month, day, hour, minute, second);
}

void PCF8563::adjust(const DateTime& dt) {
    RTCLIB_WIRE.beginTransmission(address);
    RTCLIB_WIRE.write(0x02);                       // Start address
    RTCLIB_WIRE.write(bin2bcd(dt.second()));       // Second (0-59)
    RTCLIB_WIRE.write(bin2bcd(dt.minute()));       // Minute (0-59)
    RTCLIB_WIRE.write(bin2bcd(dt.hour()));         // Hour (0-23)
    RTCLIB_WIRE.write(bin2bcd(dt.day()));          // Day (1-31)
    RTCLIB_WIRE.write(bin2bcd(dt.dayOfWeek())); // Weekday (0-6 = Sunday-Saturday)
    RTCLIB_WIRE.write(bin2bcd(dt.month()) | 0x80); // Month (1-12, century bit (bit 7) = 1)
    RTCLIB_WIRE.write(bin2bcd(dt.year() % 100));   // Year (00-99)
    RTCLIB_WIRE.endTransmission();

    begin(); // re set the control/status register to 0x04
}

uint8_t PCF8563::isrunning() {
    RTCLIB_WIRE.beginTransmission(address);
    RTCLIB_WIRE.write(0);
    RTCLIB_WIRE.endTransmission();

    RTCLIB_WIRE.requestFrom(address, 2);

    status1 = RTCLIB_WIRE.read();
    status2 = RTCLIB_WIRE.read();
    return !(bitRead(status1, 5));
}

uint8_t PCF8563::isvalid() {
    RTCLIB_WIRE.beginTransmission(address);
    RTCLIB_WIRE.write(0);
    RTCLIB_WIRE.endTransmission();

    RTCLIB_WIRE.requestFrom(address, 3);

    status1 = RTCLIB_WIRE.read();
    status2 = RTCLIB_WIRE.read();
    uint8_t VL_seconds = RTCLIB_WIRE.read();
    return !(bitRead(VL_seconds, 7));
}

// Get the alarm at 0x09 adress
DateTime PCF8563::get_alarm() {
    RTCLIB_WIRE.beginTransmission(address);
    RTCLIB_WIRE.write(0x09); // Set the register pointer to (0x0A)
    RTCLIB_WIRE.endTransmission();

    RTCLIB_WIRE.requestFrom(address, 4); // Read 4 values

    uint8_t minute = bcd2bin(RTCLIB_WIRE.read());
    uint8_t hour = bcd2bin(RTCLIB_WIRE.read());
    uint8_t day = bcd2bin(RTCLIB_WIRE.read());
    uint8_t wday = bcd2bin(RTCLIB_WIRE.read());
    return DateTime(0, wday, day, hour, minute, 0);
}

// Set a daily alarm
void PCF8563::set_alarm(const DateTime& dt, alarm_flags flags) {
    uint8_t minute = bin2bcd(dt.minute());
    uint8_t hour = bin2bcd(dt.hour());
    uint8_t day = bin2bcd(dt.day());
    uint8_t dayOfWeek = bin2bcd(dt.month());

    if (!flags.minute)
        minute |= PCF8563_ALARM_REG_OFF;

    if (!flags.hour)
        hour |= PCF8563_ALARM_REG_OFF;

    if (!flags.day)
        day |= PCF8563_ALARM_REG_OFF;

    if (!flags.wday)
        dayOfWeek |= PCF8563_ALARM_REG_OFF;

    RTCLIB_WIRE.beginTransmission(address);
    RTCLIB_WIRE.write(0x09); // Set the register pointer to (0x09)
    RTCLIB_WIRE.write(minute);
    RTCLIB_WIRE.write(hour);
    RTCLIB_WIRE.write(day);
    RTCLIB_WIRE.write(dayOfWeek);
    RTCLIB_WIRE.endTransmission();
}

void PCF8563::off_alarm() {
    // set status2 AF val to zero to reset alarm
    status2 &= ~PCF8563_ALARM_AF;
    RTCLIB_WIRE.beginTransmission(address);
    RTCLIB_WIRE.write(0x01);
    RTCLIB_WIRE.write(status2);
    RTCLIB_WIRE.endTransmission();
}

void PCF8563::on_alarm() {
    // set status2 AF val to zero
    status2 &= ~PCF8563_ALARM_AF;
    // enable the interrupt
    status2 |= PCF8563_ALARM_AIE;
    RTCLIB_WIRE.beginTransmission(address); // Issue I2C start signal
    RTCLIB_WIRE.write(0x01);
    RTCLIB_WIRE.write(status2);
    RTCLIB_WIRE.endTransmission();
}

////////////////////////////////////////////////////////////////////////////////
// RTC_Millis implementation
void RTC_Millis::begin() {
    offset = millis();
    running = true;
}

void RTC_Millis::begin(const DateTime& dt) {
    if (!running) {
        adjust(dt);
        running = true;
    }
}

void RTC_Millis::adjust(const DateTime& dt) {
    offset = dt.unixtime() * 1000 - millis();
}

DateTime RTC_Millis::now() {
    return DateTime((offset + millis()) / 1000);
}

bool RTC_Millis::isrunning() {
    return running;
}
