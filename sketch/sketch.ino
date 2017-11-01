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

#define CAN_ID_PID			0x7DF
#define PID_ENGINE_RPM		0x0C
#define PID_VEHICLE_SPEED	0x0D
#define PID_COOLANT_TEMP	0x05


static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);
static void set_mask_filt();
static void sendPid(unsigned char __pid)


static const byte RXPin = 3;
static const byte TXPin = 4;

static const byte SigRXPin = 5;
static const byte SigTXPin = 6;

union Pos LASTVALIDLON;
union Pos LASTVALIDLAT;

unsigned long LASTVALIDage = TinyGPS::GPS_INVALID_AGE;
short count = 0;

// Set CS pin
const int SPI_CS_PIN = 9;
MCP_CAN CAN(SPI_CS_PIN);    

// GPS init
TinyGPS gps;
SoftwareSerial ssGps(TXPin, RXPin);

//SIGFOX init
SoftwareSerial ssSigfox(SigTXPin, SigRXPin);

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
	set_mask_filt();
}


void loop() {
	unsigned char len = 0;
	unsigned char data[2], buf[8];
	unsigned char can_len = 0;
	unsigned char can_buf[8];
	float flat, flon;
	unsigned long age;
	int rpm_engine_value = 0;
  
	LASTVALIDLON.f = TinyGPS::GPS_INVALID_F_ANGLE;
	LASTVALIDLAT.f = TinyGPS::GPS_INVALID_F_ANGLE;
	unsigned char vehicle_speed_value = 0;

	while(count < 100)
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

		sendPid(PID_ENGINE_RPM);
		while (CAN_MSGAVAIL == CAN.checkReceive()) 
		{
			// read data,  len: data length, buf: data buf
			CAN.readMsgBuf(&can_len, can_buf);

			rpm_engine_value = (buf[3]*256+buf[4])/4;


			Serial.print("RPM: ");
			Serial.print(rpm_engine_value);
			Serial.println();
		}
    
		count++;
    
		sendPid(PID_ENGINE_RPM);
		while (CAN_MSGAVAIL == CAN.checkReceive()) 
		{
			// read data,  len: data length, buf: data buf
			CAN.readMsgBuf(&can_len, can_buf);

			vehicle_speed_value = buf[3];


			Serial.print("SPEED: ");
			Serial.print(vehicle_speed_value);
			Serial.println();
		}

		accumulateWear(rpm_engine_value, vehicle_speed_value, 0);

		smartdelay(100); // atualiza dados a cada 100ms
	}

	wearData(data);
	memcpy(msg, data, 1);
	memcpy(msg+1, LASTVALIDLAT.b, 4);
	memcpy(msg+5, LASTVALIDLON.b, 4);
	count = 0;
	sendPKG();
	resetWear(4);

}


static void sendPKG()
{
	digitalWrite(13, HIGH);
	char bytes_sent = ssSigfox.write(msg, 12);
	Serial.print("Bytes sent: ");
	printHex((int) msg, 12);
	Serial.println();
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

void printHex(int num, int precision) {
     char tmp[16];
     char format[128];

     sprintf(format, "0x%%.%dX", precision);

     sprintf(tmp, format, num);
     Serial.print(tmp);
}

void set_mask_filt()
{
    /*
     * set mask, set both the mask to 0x3ff
     */
    CAN.init_Mask(0, 0, 0x7FC);
    CAN.init_Mask(1, 0, 0x7FC);

    /*
     * set filter, we can receive id from 0x04 ~ 0x09
     */
    CAN.init_Filt(0, 0, 0x7E8);                 
    CAN.init_Filt(1, 0, 0x7E8);

    CAN.init_Filt(2, 0, 0x7E8);
    CAN.init_Filt(3, 0, 0x7E8);
    CAN.init_Filt(4, 0, 0x7E8); 
    CAN.init_Filt(5, 0, 0x7E8);
}

void sendPid(unsigned char __pid)
{
    unsigned char tmp[8] = {0x02, 0x01, __pid, 0, 0, 0, 0, 0};
    Serial.print("SEND PID: 0x");
    Serial.println(__pid, HEX);
    CAN.sendMsgBuf(CAN_ID_PID, 0, 8, tmp);
}