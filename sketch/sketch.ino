#include <SPI.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

#include "mcp_can.h"
#include "abrasion.h"

static const byte RXPin = 3;
static const byte TXPin = 4;
float LASTVALIDLON = TinyGPS::GPS_INVALID_F_ANGLE;
float LASTVALIDLAT = TinyGPS::GPS_INVALID_F_ANGLE;
unsigned long LASTVALIDage = TinyGPS::GPS_INVALID_AGE;

unsigned char stmp[8] = {0x02, 0x01, 0x0C, 0, 0, 0, 0, 0};

// Set CS pin
const int SPI_CS_PIN = 9;
MCP_CAN CAN(SPI_CS_PIN);    

TinyGPS gps;
SoftwareSerial ss(TXPin, RXPin);

static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);

void setup() {
	Serial.begin(115200); // use the same baud-rate as the python side
	Serial.print("TinyGPS library v. "); Serial.println(TinyGPS::library_version());
	ss.begin(9600); //diferentes baudrates para diferentes gps, checar datasheet

	while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
	{
		Serial.println("CAN BUS Shield init fail");
		Serial.println(" Init CAN BUS Shield again");
		delay(100);
	}
	Serial.println("CAN BUS Shield init ok!");
}

void loop() {
	unsigned char len = 0;
	unsigned char buf[8];
	float flat, flon;
	unsigned long age;
	int rpm_engine_value = 0;

	gps.f_get_position(&flat, &flon, &age);

	LASTVALIDLAT = (flat == TinyGPS::GPS_INVALID_F_ANGLE) ? LASTVALIDLAT : flat;
	LASTVALIDLON = (flon == TinyGPS::GPS_INVALID_F_ANGLE) ? LASTVALIDLON : flon;
	LASTVALIDage = (age == TinyGPS::GPS_INVALID_AGE) ? LASTVALIDage : age;

	print_float(LASTVALIDLAT, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6);
	print_float(LASTVALIDLON, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6);
	print_int(age, TinyGPS::GPS_INVALID_AGE, 5);

	Serial.println();
                  
    CAN.sendMsgBuf(0x7DF, 0, 0, 8, stmp);

    // iterate over all pending messages
    // If either the bus is saturated or the MCU is busy,
    // both RX buffers may be in use and reading a single
    // message does not clear the IRQ conditon.
    while (CAN_MSGAVAIL == CAN.checkReceive()) 
    {
        // read data,  len: data length, buf: data buf
        CAN.readMsgBuf(&len, buf);

        // print the data
        for(int i = 0; i<len; i++)
        {
            Serial.print(buf[i]);Serial.print("\t");
        }
        Serial.println();

        Serial.println();
        rpm_engine_value = 0;
        rpm_engine_value = ((buf[3]*256)+buf[4])/4;
        Serial.print("RPM: ");
        Serial.print(rpm_engine_value);
        Serial.print("\t");
    }

	smartdelay(100); // send data per 100ms
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
