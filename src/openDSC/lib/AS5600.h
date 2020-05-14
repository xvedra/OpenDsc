/*
  AS5600.h - Library for interacting with the AS5600.
  Created by Kane C. Stoboi, 22 August, 2018.
  Released into the public domain.
*/

#ifndef AS5600_h
#define AS5600_h

#include "Arduino.h"
#include <Wire.h>

#define AS5600_RES 4096

class AS5600
{
  public:
    AS5600(int sdapin, int sclpin);
    long getPosition();
    int getAngle();
    int getStatus();
    int getGain();
    int getMagnitude();
    void setZero();
  	void setEncoderResolution(long res);
  	void getEncoderResolution(long* res);
  	void clearEncoderResolution();
  	void EncoderLoop();
  	long getOutput();
  	int getInc();
    void setZeroEncoder();
  	void setFilterLevel(int FilterLevel);
  	void setOutput(long out);
    byte readRegister(byte add);
  	
  	//Friendly functions:
  	long read();
  	void write(long value);
  	void loop();
  	void setup(long res, int filter);
    void enable();
    void disable();
    bool isEnabled();
    
  private:      
    int _AS5600Address = 0x36;
    
    byte _ZMCOAddress = 0x00;
    byte _ZPOSAddressMSB = 0x01;
    byte _ZPOSAddressLSB = 0x02;
    byte _MPOSAddressMSB = 0x03;
    byte _MPOSAddressLSB = 0x04;
    byte _MANGAddressMSB = 0x05;
    byte _MANGAddressLSB = 0x06;
    byte _CONFAddressMSB = 0x07;
    byte _CONFAddressLSB = 0x08;
    byte _RAWANGLEAddressMSB = 0x0C;
    byte _RAWANGLEAddressLSB = 0x0D;
    byte _ANGLEAddressMSB = 0x0E;
    byte _ANGLEAddressLSB = 0x0F;
    byte _STATUSAddress = 0x0B;
    byte _AGCAddress = 0x1A;
    byte _MAGNITUDEAddressMSB = 0x1B;
    byte _MAGNITUDEAddressLSB = 0x1C;
    byte _BURNAddress = 0xFF;

    long _msb;
    long _lsb;
    long _msbMask = 0b00001111;

    int SCLpin;
    int SDApin;	  
	  
	  long EncoderResolution = 0;
	  int EncoderPosition = 0;
	  int EncoderLastPosition = 0;
	  int EncoderInc = 0;
	  long EncoderOutput = 0;
	  long EncoderFilteredOutput = 0;
	  long _FilterSum = 0;
	  int _FilterLevel = 0;
	  int Output = 0;
    bool _en = 0;

    long _getRegisters2(byte registerMSB, byte registerLSB);
    long _getRegisters2Seq(byte registerAdd);
    int _getRegister(byte register1);
};

AS5600::AS5600(int sdapin, int sclpin)
{
  SCLpin = sclpin;
  SDApin = sdapin;
}

long AS5600::getPosition()
{
  return _getRegisters2Seq(_RAWANGLEAddressMSB);
}

int AS5600::getAngle()
{
  return _getRegisters2Seq(_ANGLEAddressMSB); 
}

int AS5600::getStatus()
{
  return _getRegister(_STATUSAddress) & 0b00111000;
}

int AS5600::getGain()
{
  return _getRegister(_AGCAddress);
}

int AS5600::getMagnitude()
{
  return _getRegisters2Seq(_MAGNITUDEAddressMSB);
}

void AS5600::setEncoderResolution(long res)
{
	EncoderResolution = res;
}

void AS5600::getEncoderResolution(long* res)
{
	if(res != NULL) *res = EncoderResolution;
}

void AS5600::clearEncoderResolution()
{
	EncoderResolution = 0;
}

void AS5600::EncoderLoop()
{
  if(_en == 0) return;
  
  //Read encoder:
	EncoderPosition = getPosition();

  //Get incremental position:
	if((EncoderLastPosition < (AS5600_RES/4)) && (EncoderPosition > (AS5600_RES*3/4))) 
	  EncoderInc = -(AS5600_RES-1) + EncoderPosition - EncoderLastPosition; //Decremento por paso por cero 
  else if((EncoderLastPosition > (AS5600_RES*3/4)) && (EncoderPosition < (AS5600_RES/4))) 
    EncoderInc = (AS5600_RES-1) - EncoderLastPosition + EncoderPosition; //Incremento por paso por cero 
  else 
    EncoderInc = EncoderPosition - EncoderLastPosition;    
	
  EncoderLastPosition = EncoderPosition;     

  if(EncoderResolution < 0) EncoderOutput = EncoderOutput - EncoderInc; 
  else EncoderOutput = EncoderOutput + EncoderInc; 

  //filter:
	if(_FilterLevel != 0)
	{
		_FilterSum =  _FilterSum + EncoderOutput - EncoderFilteredOutput;
		EncoderFilteredOutput = _FilterSum / _FilterLevel;
	}
	else EncoderFilteredOutput = EncoderOutput;

  //Set output:
  if(EncoderFilteredOutput > 0) Output = EncoderFilteredOutput % abs(EncoderResolution);
  else if(EncoderFilteredOutput < 0) Output = abs(EncoderResolution) - (abs(EncoderFilteredOutput) % abs(EncoderResolution));
	else Output = 0;
  if(EncoderResolution < 0) Output = -Output;
}

void AS5600::setOutput(long out)
{
	EncoderInc = 0;  
	EncoderLastPosition = EncoderPosition = getPosition(); 
	EncoderOutput = EncoderFilteredOutput = Output = out;
	if(_FilterLevel != 0) _FilterSum = (long)EncoderFilteredOutput * _FilterLevel;
}

void AS5600::setFilterLevel(int FilterLevel)
{
	_FilterLevel = FilterLevel;
	_FilterSum = (long)EncoderFilteredOutput * FilterLevel;
}

long AS5600::getOutput()
{
	return(Output);
}

int AS5600::getInc()
{
	return(EncoderInc);
}

void AS5600::setZeroEncoder()
{
	int pos;
	
	pos = getPosition();
	EncoderInc = 0;
	EncoderLastPosition = EncoderPosition = pos; 
	EncoderFilteredOutput = EncoderOutput = 0;
	if(_FilterLevel != 0) _FilterSum = (long)EncoderFilteredOutput * _FilterLevel;
}

int AS5600::_getRegister(byte register1)
{ 
  Wire.begin(SDApin,SCLpin);
  Wire.beginTransmission(_AS5600Address);
  Wire.write(register1);
  Wire.endTransmission();
  delayMicroseconds(100);
  Wire.requestFrom(_AS5600Address, 1);
  if (Wire.available()) _msb = Wire.read();
  Wire.endTransmission(true);

  return _msb;
}

long AS5600::_getRegisters2Seq(byte registerAdd)
{
  byte buf[2] = {0,0};

  Wire.begin(SDApin,SCLpin);
  Wire.beginTransmission(_AS5600Address);
  Wire.write(registerAdd);
  Wire.endTransmission();
  //delayMicroseconds(100);
  Wire.requestFrom(_AS5600Address, 2);
  for (uint8_t i = 0; i < 2; i++) 
  {
      //delayMicroseconds(100);
      if (Wire.available()) buf[i] = Wire.read(); 
  }  
  _lsb = buf[1];
  _msb = buf[0];
  Wire.endTransmission(true);
  
  return (_lsb) + (_msb & _msbMask) * 256;
}


/*
* Friendly functions:
*/

long AS5600::read()
{
	return(getOutput());
}

void AS5600::write(long value)
{
	setOutput(value);
}

void AS5600::loop()
{
	EncoderLoop();
}

void AS5600::setup(long res, int filter)
{
	setFilterLevel(filter);
	setEncoderResolution(res);
}

void AS5600::enable()
{
  _en = 1;
}

void AS5600::disable()
{
  _en = 0;
}

bool AS5600::isEnabled()
{
  return(_en);
}

byte AS5600::readRegister(byte add)
{
  return(_getRegister(add));
}
#endif
