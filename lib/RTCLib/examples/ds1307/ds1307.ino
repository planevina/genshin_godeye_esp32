// Date and time functions using a DS1307 RTC connected via I2C and Wire lib

#include <Wire.h>
#include <RTClib.h>

DS1307 rtc;

// buffer for DateTime.tostr
char buf[20];

void setup() {
  Serial.begin(9600);
#ifdef AVR
  Wire.begin();
#else
  Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
#endif
  rtc.begin();

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
}

void loop() {
  DateTime now = rtc.now();

  Serial.println(now.tostr(buf));

  Serial.print(" since midnight 1/1/1970 = ");
  Serial.print(now.unixtime());
  Serial.print("s = ");
  Serial.print(now.unixtime() / 86400L);
  Serial.println("d");

  // calculate a date which is 7 days and 30 seconds into the future
  DateTime future(now + (7 * 86400L + 30));

  Serial.print(" now + 7d + 30s: ");
  Serial.println(future.tostr(buf));

  // calculate a date which is 30 days before
  DateTime past(now - TimeDelta(30 * 86400L));

  Serial.print(" now - 30d: ");
  Serial.println(past.tostr(buf));
  Serial.println();
  delay(3000);
}
