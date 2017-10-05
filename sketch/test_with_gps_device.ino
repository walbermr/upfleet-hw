#include <SoftwareSerial.h>
#include <TinyGPS.h>

static const byte RXPin = 3;
static const byte TXPin = 4;
float LASTVALIDlon = TinyGPS::GPS_INVALID_F_ANGLE;
float LASTVALIDlat = TinyGPS::GPS_INVALID_F_ANGLE;
unsigned long LASTVALIDage = TinyGPS::GPS_INVALID_AGE;

TinyGPS gps;
SoftwareSerial ss(TXPin, RXPin);

static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);

void setup()
{
  Serial.begin(115200);
  
  Serial.print("TinyGPS library v. "); Serial.println(TinyGPS::library_version());
  ss.begin(9600); //diferentes baudrates para diferentes gps, checar datasheet
}

void loop()
{
  float flat, flon;
  unsigned long age;

  gps.f_get_position(&flat, &flon, &age);

  LASTVALIDlat = (flat == TinyGPS::GPS_INVALID_F_ANGLE) ? LASTVALIDlat : flat;
  LASTVALIDlon = (flon == TinyGPS::GPS_INVALID_F_ANGLE) ? LASTVALIDlon : flon;
  LASTVALIDage = (age == TinyGPS::GPS_INVALID_AGE) ? LASTVALIDage : age;

  print_float(LASTVALIDlat, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6);
  print_float(LASTVALIDlon, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6);
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);

  Serial.println();
  
  smartdelay(1000);
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

static void print_float(float val, float invalid, int len, int prec)
{
  if (val == invalid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
  smartdelay(0);
}

static void print_int(unsigned long val, unsigned long invalid, int len)
{
  char sz[32];
  if (val == invalid)
    strcpy(sz, "*******");
  else
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
  smartdelay(0);
}
