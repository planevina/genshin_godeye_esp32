// DateTime format utility example

#include <Wire.h>
#include <RTClib.h>

DS1307 rtc;
//DS1302 rtc; // see ds1302 example for pin configuration
//DS3231 rtc;
//PCF8563 rtc;
//PCF8583 rtc;
//RTC_Millis rtc;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  rtc.begin();
  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
}

void loop() {
  DateTime now = rtc.now();
  char buf[100];
  strncpy(buf, "YYYY.MM.DD hh:mm:ss", 100);
  Serial.println(now.format(buf));
  delay(1000);
}
