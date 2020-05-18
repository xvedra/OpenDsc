#define battery_max 4.2
#define battery_min 2.5
#define battery_alert 2.8
#define vref 1100

static byte winHeadLastBatteryOutput = -1;

void drawHead(byte section)
{
  bool draw = 0;
  
  if(section == SECTION_START)
  {
    winHeadLastBatteryOutput = -1;
    draw = 1;
    tft.setCursor(0, 0);
    tft.setTextDatum(MC_DATUM);    
    tft.setTextColor(Red);
    tft.setTextSize(3);
    
    sprintf(buff, "%s", MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Name);
    tft.drawString(buff, 0, 15);

    if(isBtConnected()) tft.pushImage(239-24-32-5*2, 0,  27, 32, &BTonoff[_BTonoff_on]); 
    else tft.pushImage(239-24-32-5*2, 0,  27, 32, &BTonoff[_BTonoff_off]);

    if(isWiFiConnected()) tft.pushImage(239-27-5, 0,  32, 32, &WIFIonoff[_WIFIonoff_on]); 
    else tft.pushImage(239-27-5, 0,  32, 32, &WIFIonoff[_WIFIonoff_off]);
  }

  if(section == SECTION_RUN || section == SECTION_START)
  {
    if(eventsReadCurrentEvent() == EVENT_WIRELESS_CHANGE)
    {
      if(isBtConnected()) tft.pushImage(239-24-32-5*2, 0,  27, 32, &BTonoff[_BTonoff_on]); 
      else tft.pushImage(239-24-32-5*2, 0,  27, 32, &BTonoff[_BTonoff_off]);
  
      if(isWiFiConnected()) tft.pushImage(239-27-5, 0,  32, 32, &WIFIonoff[_WIFIonoff_on]); 
      else tft.pushImage(239-27-5, 0,  32, 32, &WIFIonoff[_WIFIonoff_off]);
    }
  
    if((millis() % 100) == 0 || draw) //1Hz
    {
      uint16_t v = analogRead(ADC_PIN);
      float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
      byte output = 8 * ((battery_voltage - battery_min) / (battery_max - battery_min));
      if(output > 8) output = 8;
      if(output < 0) output = 0;
      
      if(output != winHeadLastBatteryOutput || draw)
      {
        winHeadLastBatteryOutput = output;      
  
        if(output < 2 )     tft.pushImage(239-22-16-32-5*4, 0,  16, 32, &VBatt[_VBatt_VERYLOW]);    
        else if(output < 4) tft.pushImage(239-22-16-32-5*4, 0,  16, 32, &VBatt[_VBatt_LOW]); 
        else if(output < 6) tft.pushImage(239-22-16-32-5*4, 0,  16, 32, &VBatt[_VBatt_MIDDLE]); 
        else                tft.pushImage(239-22-16-32-5*4, 0,  16, 32, &VBatt[_VBatt_HIGH]); 
      }
    }
  }
}

