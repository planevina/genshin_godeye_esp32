// Simple date conversions and calculations

// #include <Wire.h>
#include <RTClib.h>

void showDate(const char* txt, const DateTime& dt) {
  // buffer for DateTime.tostr
  static char buf[20];

  Serial.print(txt);
  Serial.print(' ');
  Serial.print(dt.tostr(buf));

  Serial.print(" = ");
  Serial.print(dt.unixtime());
  Serial.print("s / ");
  Serial.print(dt.unixtime() / 86400L);
  Serial.print("d since 1970");

  Serial.println();
}

void setup() {
  Serial.begin(9600);

  DateTime dt0(0, 1, 1, 0, 0, 0);
  showDate("dt0", dt0);

  DateTime dt1(1, 1, 1, 0, 0, 0);
  showDate("dt1", dt1);

  DateTime dt2(2009, 1, 1, 0, 0, 0);
  showDate("dt2", dt2);

  DateTime dt3(2009, 1, 2, 0, 0, 0);
  showDate("dt3", dt3);

  DateTime dt4(2009, 1, 27, 0, 0, 0);
  showDate("dt4", dt4);

  DateTime dt5(2009, 2, 27, 0, 0, 0);
  showDate("dt5", dt5);

  DateTime dt6(2009, 12, 27, 0, 0, 0);
  showDate("dt6", dt6);

  DateTime dt7(dt6.unixtime() + 3600); // one hour later
  showDate("dt7", dt7);

  DateTime dt8(dt6.unixtime() + 86400L); // one day later
  showDate("dt8", dt8);

  DateTime dt9(dt6 + TimeDelta(7 * 86400L)); // one week later
  showDate("dt9", dt9);
}

void loop() {
  // do nothing
}
