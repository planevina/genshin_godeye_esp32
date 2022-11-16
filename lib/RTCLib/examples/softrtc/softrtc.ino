// Date and time functions using just software, based on millis() & timer

#include <RTClib.h>

RTC_Millis rtc;

// buffer for DateTime.tostr
char buf[20];

void setup() {
  Serial.begin(9600);
  // following line sets the RTC to the date & time this sketch was compiled
  rtc.begin(DateTime(__DATE__, __TIME__));
}

void loop() {
  DateTime now = rtc.now();

  Serial.println(now.tostr(buf));

  Serial.print(" seconds since 1970: ");
  Serial.println(now.unixtime());

  // calculate a date which is 7 days and 30 seconds into the future
  DateTime future(now + (7 * 86400L + 30));

  Serial.println(future.tostr(buf));

  Serial.println();
  delay(3000);
}
