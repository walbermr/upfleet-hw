#include "abrasion.h"

char _rpmbuf[2];
char _brakebuf[2];
char _speedbuf[2];
char _axisbuf[3];

unsigned short int _brakeactuations = 0;
bool _brakeactionated = false;
unsigned long int _braketime = 0;
char _brakespeed = 0;
unsigned long int _rpmchagetime = 0;
char _lastrpmcycle = 0xFF;


void setBrakeBuf(char buff[2])
{
	_brakebuf[0] = buff[0];
	_brakebuf[1] = buff[1];
	return;
}

void setRpmBuf(char buff[2])
{
	_rpmbuf[0] = buff[0];
	_rpmbuf[1] = buff[1];
	return;
}

void setSpeedBuf(char buff[2])
{
	_speedbuf[0] = buff[0];
	_speedbuf[1] = buff[1];
	return;
}

void setAxisBuf(char buff[2])
{
	_axisbuf[0] = buff[0];
	_axisbuf[1] = buff[1];
}

char instantBrakesAbrasion()
{
	//medida da abrasão
	char brake_abrasion = 0;

	//freadas bruscas
	char brake_derivation = (_brakebuf[1] - _brakebuf[0])/2;

	//acionamentos do freio e inicio de contagem da freada
	if((brake_derivation > 0) && (_brakeactionated == false))
	{
		_braketime = millis();
		_brakeactionated = true;
		_brakeactuations++;
		_brakespeed = _speedbuf[1];
	}
	//contabiliza o fim de uma freada
	else if(brake_derivation < 0)
	{
		unsigned long int brake_time = millis() - _braketime;
		//abrasão devido a muito tempo #0-128
		if(brake_time >= BRAKE_TIME_HIGH)
		{
			if(_brakespeed >= SPEED_HIGH)
				brake_abrasion += ABRASION_ULTRA_HIGH;

			else if(_brakespeed >= SPEED_MEDIUM)
				brake_abrasion += ABRASION_HIGH;

			else if(_brakespeed >= SPEED_LOW)
				brake_abrasion += ABRASION_MEDIUM;
			
		}
		else if(brake_time >= BRAKE_TIME_MEDIUM)
		{
			if(_brakespeed >= SPEED_HIGH)
				brake_abrasion += ABRASION_ULTRA_HIGH;

			else if(_brakespeed >= SPEED_MEDIUM)
				brake_abrasion += ABRASION_HIGH;
			
			else if(_brakespeed >= SPEED_LOW)
				brake_abrasion += ABRASION_MEDIUM;
		}
		else if(brake_time >= BRAKE_TIME_LOW)
		{
			if(_brakespeed >= SPEED_HIGH)
				brake_abrasion += ABRASION_MEDIUM;

			else if(_brakespeed >= SPEED_MEDIUM)
				brake_abrasion += ABRASION_LOW;
			
			else if(_brakespeed >= SPEED_LOW)
				brake_abrasion += ABRASION_LOW;
		}
		//abrasão devido a uma freada brusca #0-128
		if(brake_derivation >= DERIVATIVE_HIGH)
		{
			if(_brakespeed >= SPEED_HIGH)
				brake_abrasion += ABRASION_ULTRA_HIGH;

			else if(_brakespeed >= SPEED_MEDIUM)
				brake_abrasion += ABRASION_HIGH;

			else if(_brakespeed >= SPEED_LOW)
				brake_abrasion += ABRASION_MEDIUM;
		}
		else if(brake_derivation >= DERIVATIVE_MEDIUM)
		{
			if(_brakespeed >= SPEED_HIGH)
				brake_abrasion += ABRASION_HIGH;

			else if(_brakespeed >= SPEED_MEDIUM)
				brake_abrasion += ABRASION_MEDIUM;
			
			else if(_brakespeed >= SPEED_LOW)
				brake_abrasion += ABRASION_LOW;

		}
		else if(brake_derivation >= DERIVATIVE_LOW)
		{
			if(_brakespeed >= SPEED_HIGH)
				brake_abrasion += ABRASION_MEDIUM;

			else if(_brakespeed >= SPEED_MEDIUM)
				brake_abrasion += ABRASION_LOW;
			
			else if(_brakespeed >= SPEED_LOW)
				brake_abrasion += ABRASION_LOW;
		}

		_brakeactionated = false;
	}

	return brake_abrasion;
}


char instantClutchAbrasion()
{
	//medida de abrasão
	char clutch_abrasion = 0;

	//aceleração/desaceleração bruscas
	char rpm_derivation = (_rpmbuf[1] - _rpmbuf[0])/2;

	//pega grandes variações se não houver freada
	if((rpm_derivation >= DERIVATIVE_LOW) && (_brakebuf[1] == 0))
	{
		if(rpm_derivation >= DERIVATIVE_HIGH)
			clutch_abrasion += ABRASION_ULTRA_HIGH;

		else if(rpm_derivation >= DERIVATIVE_MEDIUM)
			clutch_abrasion += ABRASION_HIGH;

		else if(rpm_derivation >= DERIVATIVE_LOW)
			clutch_abrasion += ABRASION_MEDIUM;
	}

	//capturar buracos e variação de velocidade

	return clutch_abrasion;

}

char instantEngineCondition()
{
	//medida de abrasão
	char engine_abrasion = 0;
	char rpm = (_rpmbuf[0] + _rpmbuf[1])/2;
	unsigned long int last_rpm_time = _rpmchagetime;
	bool rpm_chaged = false;

	if((rpm >= RPM_ULTRA_HIGH) && (_lastrpmcycle != 0))
	{
		_lastrpmcycle = 0;
		_rpmchagetime = millis();
		rpm_chaged = true;
	}	
	else if((rpm >= RPM_ULTRA_HIGH) && (_lastrpmcycle != 1))
	{
		_lastrpmcycle = 1;
		_rpmchagetime = millis();
		rpm_chaged = true;
	}
	else if((rpm >= RPM_MEDIUM) && (_lastrpmcycle != 2))
	{
		_lastrpmcycle = 2;
		_rpmchagetime = millis();
		rpm_chaged = true;
	}
	else if((rpm >= RPM_LOW) && (_lastrpmcycle != 3))
	{
		_lastrpmcycle = 3;
		_rpmchagetime = millis();
		rpm_chaged = true;
	}

	if(rpm_chaged == true)
	{
		unsigned long int deltatime = _rpmchagetime - last_rpm_time;
		if(deltatime >= RPM_TIME_HIGH)
		{
			if(_lastrpmcycle == 0)
				engine_abrasion += ABRASION_ULTRA_HIGH;

			else if(_lastrpmcycle == 1)
				engine_abrasion += ABRASION_ULTRA_HIGH;

			else if(_lastrpmcycle == 2)
				engine_abrasion += ABRASION_MEDIUM;

			else if(_lastrpmcycle == 3)
				engine_abrasion += ABRASION_LOW;

		}
		else if(deltatime >= RPM_TIME_MEDIUM)
		{
			if(_lastrpmcycle == 0)
				engine_abrasion += ABRASION_HIGH;

			else if(_lastrpmcycle == 1)
				engine_abrasion += ABRASION_MEDIUM;

			else if(_lastrpmcycle == 2)
				engine_abrasion += ABRASION_LOW;

			else if(_lastrpmcycle == 3)
				engine_abrasion += 0;

		}
		else if(deltatime >= RPM_TIME_LOW)
		{
			if(_lastrpmcycle == 0)
				engine_abrasion += ABRASION_MEDIUM;

			else if(_lastrpmcycle == 1)
				engine_abrasion += ABRASION_LOW;

			else if(_lastrpmcycle == 2)
				engine_abrasion += 0;

			else if(_lastrpmcycle == 3)
				engine_abrasion += 0;

		}
	}

	return engine_abrasion;
}