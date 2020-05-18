#include "def.h"

#define GRAPH_MAX_POINTS 60
static int winMainLastAlt = -1;
static int winMainLastAz = -1;
static float winMainLastBatteryVoltage = -1.0;


void winMain(byte section)
{  
  static signed int data[GRAPH_MAX_POINTS];
  int i;
  static int index = 0;
  static unsigned long int LastGetPorReq = 0;
  static unsigned int currentTextColor = 0;   
  bool draw = 0;

  if(section == SECTION_START)
  {
    winMainLastAlt = -1;
    winMainLastAz = -1;
    winMainLastBatteryVoltage = -1.0;    
    TimerDrawData = InitTimer(1000);
    draw = 1;
    
    tft.fillScreen(TFT_BLACK);      
    drawHead(SECTION_START);

    if(dsc_GetAzSensor() < 0)
    {
      #ifdef _okfail1
      tft.pushImage(239-32, 3+30,  32, 32, &okfail1[_okfail1_fail]);
      #else
      #endif
    }
    else
    {
      #ifdef _okfail1
      tft.pushImage(239-32, 3+30,  32, 32, &okfail1[_okfail1_ok]); 
      #else
      #endif
    }
    
    if(dsc_GetAltSensor() < 0)
    {
      #ifdef _okfail1
      tft.pushImage(239-32, 3+32*2,  32, 32, &okfail1[_okfail1_fail]);
      #else
      #endif
    }
    else
    {
      #ifdef _okfail1
      tft.pushImage(239-32, 3+32*2,  32, 32, &okfail1[_okfail1_ok]); 
      #else
      #endif
    }
      
    tft.setTextSize(2);  
    tft.setTextDatum(MC_DATUM);     
    tft.setTextColor(DarkerRed);
    tft.drawString("RA/AZ:", 5, 25+32);
    tft.drawString("DEC/ALT:", 5, 25+32*2);
    tft.drawString("BAT:", 5, 25+32*3); 
    tft.setTextSize(3);
  }

  if(section == SECTION_RUN || section == SECTION_START)
  {
    drawHead(SECTION_RUN);
    
    if(TimerDrawData == 0)
    {
      for(i = 0; i < GRAPH_MAX_POINTS; i++) data[i] = 0;
      index = 0;
      TimerDrawData = InitTimer(0);
    }
  
    if(checkTimer(TimerDrawData, 1000) || draw) TimerDrawData = InitTimer(0);
    else return;
  
    //Request rate:
    data[index] = ReadGetPosReqCtr() - LastGetPorReq;
    if(data[index] < 0) data[index] = 0;
      
    
    // Zone A
    if(dsc_GetAz() != winMainLastAz)
    {
      tft.setTextSize(3);
      tft.setTextDatum(MC_DATUM);  
      winMainLastAz = dsc_GetAz();
      tft.setTextColor(Red, TFT_BLACK);
      sprintf(buff, "%c%05ld", abs(dsc_GetAz())==dsc_GetAz()?'+':'-',abs(dsc_GetAz()));    
      //sprintf(buff, "%05ld",abs(dsc_GetAz())); 
      //tft.fillRect(100, 10+32-2, 105, 21, TFT_BLACK); //fillRect
      tft.drawString(buff, 160-8, 20 + 32 * 1);
    }
  
    // Zone B
    if(dsc_GetAlt() != winMainLastAlt)
    {
      tft.setTextSize(3);
      tft.setTextDatum(MC_DATUM);  
      winMainLastAlt = dsc_GetAlt();    
      tft.setTextColor(Red, TFT_BLACK);
      sprintf(buff, "%c%05ld", abs(dsc_GetAlt())==dsc_GetAlt()?'+':'-',abs(dsc_GetAlt()));
      //sprintf(buff, "%05ld", abs(dsc_GetAlt()));
      //tft.fillRect(100, 10+32*2-2, 105, 21, TFT_BLACK);
      tft.drawString(buff, 160-8, 20 + 32 * 2);  
    }
  
    // Zone C
    uint16_t v = analogRead(ADC_PIN);
    float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    if(battery_voltage != winMainLastBatteryVoltage)
    {
      if(battery_voltage < battery_alert)
      {
        //1Hz:
        if(millis() & 0x400) tft.setTextColor(TFT_ORANGE);
        else tft.setTextColor(TFT_DARKGREY);
      }
      else tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
      tft.setTextSize(3);
      tft.setTextDatum(MC_DATUM);  
      sprintf(buff, "%01.2fV", battery_voltage);
      tft.drawString(buff, 160-8, 20 + 32 * 3);  
    }
  
  /*
    // Zone E: Graph
    tft.fillRect(0, 134-50, 240, 50, TFT_BLACK);
    //tft.drawRect(0, 134-50, 240, 50, TFT_RED);//debug
    if(myEvent != 0)
    {
      tft.setTextColor(Red);
      if(myEvent == 1)
      {
        sprintf(buff, "Connected");
        tft.pushImage(215, 0,  24, 32, BT);
      }
      if(myEvent == 2)
      {
        sprintf(buff, "Disconnected");
        tft.pushImage(215, 0,  24, 32, BToff);
      }
      if(myEvent != 0) tft.drawString(buff, 10, 100 + 30);  
      myEvent = 0;
    }
    else
    {
      for(i = 0; i < GRAPH_MAX_POINTS; i++)
      {
        int val = data[(index + i)%GRAPH_MAX_POINTS] * 4;         
        if(val > 40) val = 40;
        if(val < 0) val = 0; 
        tft.drawRect(i*4, 100 + (32 - val), 2, val, TFT_BROWN);
      }
    }  
  */ 
  
    index--;
    if(index < 0) index = GRAPH_MAX_POINTS - 1;  
    LastGetPorReq = ReadGetPosReqCtr();  
  
    tft.setTextSize(2);
  }
}

