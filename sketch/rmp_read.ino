#include <SPI.h>
#include "mcp_can.h"

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin


unsigned char len = 0;
unsigned char buf[8];
char str[20];

void setup()
{
    Serial.begin(115200);

    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok!");

    //attachInterrupt(0, MCP2515_ISR, FALLING); // start interrupt
}


unsigned char stmp[8] = {0x02, 0x01, 0x0C, 0, 0, 0, 0, 0};
void loop()
{
    // send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
    int rpm_engine_value = 0;
    delay(100);                       // send data per 100ms
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
    //}
}


byte discretize(int value, int thresh[], byte len) {
  byte out;

  for (out = 0; out < len; out++) {
    if (value <= thresh[out]) {
      break;
    }
  }

  return out;
}

byte verifyWear(byte high, byte low, byte low_bits, byte wear[]) {
  byte i = high << low_bits + low;

  return wear[i];
}

