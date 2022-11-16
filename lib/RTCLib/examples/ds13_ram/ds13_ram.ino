// Example for using internal RAM on DS series chips

#include <Wire.h>
#include <RTClib.h>

// For DS1302 pin configuration, please check ds1302 example
DS1302 rtc;
//DS1307 rtc;
//DS3231 rtc;

#define BUFSIZE DS1302_RAMSIZE
//#define BUFSIZE DS1307_RAMSIZE

// buffer
char buf[BUFSIZE];

void printArray(const byte* ptr, int len) {
  Serial.print("[");
  for (int i = 0; i < len - 1; i++) {
    Serial.print(*(ptr + i), DEC);
    Serial.print(", ");
  }
  Serial.print(*(ptr + len - 1), DEC);
  Serial.println("]");
}

void setup() {
  Serial.begin(9600);
  Wire.begin(); // for DS1307
  rtc.begin();

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }

  byte b = 63;
  Serial.print("Set first byte of RAM to: ");
  rtc.writeram(0, b);
  Serial.println(b);

  Serial.print("Get first byte of RAM: ");
  b = rtc.readram(0);
  Serial.println(b);
  Serial.println();

  // test data
  for (int i = 0; i < BUFSIZE; i++) buf[i] = i;

  rtc.putram(buf, BUFSIZE);
}

void loop() {
  static int counter = 0;

  DateTime now = rtc.now();

  Serial.println("Previous data in RAM:");
  rtc.getram(buf, BUFSIZE);
  printArray(buf, BUFSIZE);

  Serial.println("Setting new data to RAM:");
  snprintf(buf, BUFSIZE, "%s count %d", now.tostr(buf), counter);
  Serial.println(buf);
  rtc.putram(buf, BUFSIZE);

  counter++;

  Serial.println();
  delay(3000);
}
