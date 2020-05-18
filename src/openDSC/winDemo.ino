/*
 * KEYS:
 * UP LONG CLICK: change pad selection
 * UP VERY LONG CLICK: start/stop fast AZ/ALT counter
 * UP CLICK: AZ/ALT count 1 unit 
 * DWN LONG CLICK: change window
 * DWN VERY LONG CLICK: go to Menu
 */


#include "def.h"

void winDemo(byte section)
{
  char buf[48];
  static long int myDemoAlt = 0;
  static long int myDemoAz = 0;
  bool drawText = 0;
  bool drawGraph = 0;
  static byte sel = 0;
  static bool goFast = 0;

  if(section == SECTION_START)
  {
    tft.fillScreen(TFT_BLACK);
    drawHead(SECTION_START);
    tft.setTextSize(2);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(DarkerRed);
    tft.drawString("RA/AZ:", 100+20, 40);
    tft.drawString("DEC/ALT:", 100+20, 40+45);
    goFast = 0;
    sel = 0;
    dsc_Disable();
    drawText = 1;
    drawGraph = 1;
    myDemoAlt = dsc_GetAlt();
    myDemoAz = dsc_GetAz();
    //Serial.printf("Res> Az:%ld Alt:%ld\n", dsc_GetAzRes(), dsc_GetAltRes());
    //Serial.printf("Pos> Az:%ld Alt:%ld\n", dsc_GetAlt(), dsc_GetAz());
  }
  if(section == SECTION_RUN || section == SECTION_START)
  {    
    drawHead(SECTION_RUN);

    if(eventsReadCurrentEvent() == EVENT_BTN_UP_VERY_LONG_CLICK) goFast = !goFast;
    if(eventsReadCurrentEvent() == EVENT_BTN_UP_LONG_CLICK)
    {
      sel = (sel + 1) % 9;       
      drawGraph = 1;
      //Serial.printf("Sel: %d\n", sel);
    }
    if((eventsReadCurrentEvent() == EVENT_BTN_UP_CLICK) || (goFast && !(millis() % 50)))
    {   
      if(sel == 0)
      {
        myDemoAlt = 0;
        myDemoAz = 0;
        drawText = 1;
        goFast = 0;
      }
      else
      {
        if(sel == 1 || sel == 2 || sel == 8)
        {
          myDemoAlt++;
          drawText = 1;
        }
        if(sel == 2 || sel == 3 || sel == 4)
        {
          myDemoAz++;
          drawText = 1;
        }
        if(sel == 4 || sel == 5 || sel == 6)
        {
          myDemoAlt--;
          drawText = 1;
        }
        if(sel == 6 || sel == 7 || sel == 8)
        {
          myDemoAz--;
          drawText = 1;
        }    
      }        
      dsc_SetAlt(formatData(myDemoAlt, dsc_GetAltRes()));
      dsc_SetAz(formatData(myDemoAz, dsc_GetAzRes()));    
    }

  
    if(drawText)
    {
      tft.setTextSize(3);
      tft.setTextDatum(TL_DATUM);
      tft.setTextColor(DarkerRed, TFT_BLACK);
      sprintf(buf,"%c%05ld", dsc_GetAz() == abs(dsc_GetAz())?'+':'-',abs(dsc_GetAz()));
      tft.drawString(buf, 100+20, 40+20);
      sprintf(buf,"%c%05ld", dsc_GetAlt() == abs(dsc_GetAlt())?'+':'-',abs(dsc_GetAlt()));
      tft.drawString(buf, 100+20, 40+65);
    }

    if(drawGraph)
    {  
      if(sel == 0) tft.fillRect(24+8, 33+50-20, 20*2, 20*2, TFT_RED);
      else
      {
        tft.fillRect(24+8, 33+50-20, 20*2, 20*2, TFT_BLACK);
        tft.drawRect(24+8, 33+50-20, 20*2, 20*2, TFT_RED);
      }
      
      if(sel == 1 || sel == 2 || sel == 8) 
        tft.fillTriangle(52,35, 52+22,35+22, 52-24,35+22, TFT_RED);
      else
      {
        tft.fillTriangle(52,35, 52+22,35+22, 52-24,35+22, TFT_BLACK);
        tft.drawTriangle(52,35, 52+22,35+22, 52-24,35+22, TFT_RED);
      }
      
      if(sel == 2 || sel == 3 || sel == 4)
        tft.fillTriangle(102,33+50, 102-22,33+50+22, 102-22, 33+50-22, TFT_RED);
      else
      {
        tft.fillTriangle(102,33+50, 102-22,33+50+22, 102-22, 33+50-22, TFT_BLACK);
        tft.drawTriangle(102,33+50, 102-22,33+50+22, 102-22, 33+50-22, TFT_RED);
      }
      
      if(sel == 4 || sel == 5 || sel == 6)
        tft.fillTriangle(52,130, 52-22,130-22, 52+22,130-22, TFT_RED);
      else
      {
        tft.fillTriangle(52,130, 52-22,130-22, 52+22,130-22, TFT_BLACK);
        tft.drawTriangle(52,130, 52-22,130-22, 52+22,130-22, TFT_RED);
      }
      
      if(sel == 6 || sel == 7 || sel == 8)
        tft.fillTriangle(2,35+50, 24,35+50-22, 24,35+50+22, TFT_RED);
      else
      {
        tft.fillTriangle(2,35+50, 24,35+50-22, 24,35+50+22, TFT_BLACK);
        tft.drawTriangle(2,35+50, 24,35+50-22, 24,35+50+22, TFT_RED);   
      }
    }      
  }
  if(section == SECTION_END)
  {
    
  }
  tft.setTextSize(2);
}
