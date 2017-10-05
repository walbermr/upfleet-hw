#ifndef ABRASION_H
#define ABRASION_H

#define ABRASION_ULTRA_HIGH		63
#define ABRASION_HIGH			31
#define ABRASION_MEDIUM 		15
#define ABRASION_LOW 			7

#define BRAKE_TIME_HIGH 		10000
#define BRAKE_TIME_MEDIUM 		5000
#define BRAKE_TIME_LOW 			2500

#define SPEED_HIGH 				100
#define SPEED_MEDIUM 			70
#define SPEED_LOW 				40

#define DERIVATIVE_HIGH 		200
#define DERIVATIVE_MEDIUM 		100
#define DERIVATIVE_LOW			50

#define RPM_ULTRA_HIGH			5000
#define RPM_HIGH 				4500
#define RPM_MEDIUM				3000
#define RPM_LOW					1500

#define RPM_TIME_HIGH			6000
#define RPM_TIME_MEDIUM			3000
#define RPM_TIME_LOW			1000


void setBrakeBuf(char buff[2]);
void setRpmBuf(char buff[2]);
void setSpeedBuf(char buff[2]);
void setAxisBuf(char buff[2]);
char instantBrakesAbrasion();
char instantClutchAbrasion();
char instantEngineCondition();
//unsigned long int millis();

#endif // ABRASION_H