#include <SPI.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <string.h>
#include "mcp_can.h"
 extern "C"
 {
 	#include "abrasion.h"	
 }

union Pos {  // union consegue definir vários tipos de dados na mesma posição de memória
	char b[4];
	float f;
};

static const byte RXPin = 3;
static const byte TXPin = 4;

static const byte SigRXPin = 5;
static const byte SigTXPin = 6;

union Pos LASTVALIDLON;
union Pos LASTVALIDLAT;

unsigned long LASTVALIDage = TinyGPS::GPS_INVALID_AGE;
short count = 0;

unsigned char stmp[8] = {0x02, 0x01, 0x0C, 0, 0, 0, 0, 0};

char msg[12];


// Set CS pin
const int SPI_CS_PIN = 9;
MCP_CAN CAN(SPI_CS_PIN);    

// GPS init
TinyGPS gps;
SoftwareSerial ssGps(TXPin, RXPin);

//SIGFOX init
SoftwareSerial ssSigfox(SigTXPin, SigRXPin);

static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);

void setup() {
	pinMode(13, OUTPUT);
	digitalWrite(13, LOW);
	Serial.begin(115200); // use the same baud-rate as the python side
	ssSigfox.begin(9600);
	ssGps.begin(9600); //diferentes baudrates para diferentes gps, checar datasheet
	Serial.print("TinyGPS library v. "); Serial.println(TinyGPS::library_version());

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
	unsigned char data[2], buf[8];
	float flat, flon;
	unsigned long age;
	int rpm_engine_value = 0;
	LASTVALIDLON.f = TinyGPS::GPS_INVALID_F_ANGLE;
	LASTVALIDLAT.f = TinyGPS::GPS_INVALID_F_ANGLE;

	while(count < 4096)
	{
		gps.f_get_position(&flat, &flon, &age);

		LASTVALIDLAT.f = (flat == TinyGPS::GPS_INVALID_F_ANGLE) ? LASTVALIDLAT.f : flat;
		LASTVALIDLON.f = (flon == TinyGPS::GPS_INVALID_F_ANGLE) ? LASTVALIDLON.f : flon;
		LASTVALIDage = (age == TinyGPS::GPS_INVALID_AGE) ? LASTVALIDage : age;

		print_float(LASTVALIDLAT.f, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6);
		print_float(LASTVALIDLON.f, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6);
		print_int(age, TinyGPS::GPS_INVALID_AGE, 5);

		Serial.println();
		Serial.println(count);

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

			accumulateWear(rpm_engine_value, 0, 0);

			Serial.print("RPM: ");
			Serial.print(rpm_engine_value);
			Serial.println();
		}
		count++;
		smartdelay(100); // atualiza dados a cada 100ms
	}

	wearData(data);
	memcpy(msg, &data, 1);
	memcpy(msg+1, LASTVALIDLAT.b, 4);
	memcpy(msg+5, LASTVALIDLON.b, 4);
	count = 0;
	sendPKG();
	resetWear(4);

}


static void sendPKG()
{
	digitalWrite(13, HIGH);
	Serial.print("Data sent: ");
	Serial.println(msg);
	digitalWrite(13, LOW);

	ssSigfox.write(msg);
	return;
}


static void smartdelay(unsigned long ms)
{
	unsigned long start = millis();
	do 
	{
		while (ssGps.available())
		gps.encode(ssGps.read());
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
