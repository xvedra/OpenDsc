void loadEEPROM()
{
  byte* p;
  #ifdef USE_DEBUG
  Serial.print("Loading EEPROM...\n");
  #endif
  p = (byte*)&MyConfig;
  for(int i = 0; i < EEPROM_SIZE; i++) p[i] = EEPROM.read(i);

  eventsPutFifo(EVENT_LOAD_DATA);
}

void uploadEEPROM()
{
  byte* p;
  int ctr = 0;

  validateEEPROM();
  #ifdef USE_DEBUG
  Serial.print("Uploading EEPROM...");
  #endif
  p = (byte*)&MyConfig;
  for(int i = 0; i < EEPROM_SIZE; i++)
  {
    if(EEPROM.read(i) != p[i])
    {
      EEPROM.write(i, p[i]); 
      ctr++;
    }
  }
  EEPROM.commit();
  #ifdef USE_DEBUG
  Serial.printf("%d bytes\n", ctr);
  #endif

  eventsPutFifo(EVENT_SAVE_DATA);
}
void loadDefEEPROM()
{
  #ifdef USE_DEBUG
  Serial.print("Loading default config...\n");
  #endif
  MyConfig.Version = EEPROM_VERSION;
  MyConfig.CurrentMount = DEF_CURRENT_MOUNT;
  /*
  //MyConfig.WiFi = DEF_WIFI;
  //MyConfig.Bluetooth = DEF_BT;
  //MyConfig.USB = DEF_USB_SERIAL_BAUDRATE;  
  */
  strncpy(MyConfig.Tele[0].Name,DEF_NAME1, BT_NAME_SIZE);      
  MyConfig.Tele[0].RA_Az_SensorType = DEF_AZ_ENC1;
  MyConfig.Tele[0].Dec_Alt_SensorType = DEF_ALT_ENC1;
  MyConfig.Tele[0].RA_Az_Res = DEF_AZ_RES1;
  MyConfig.Tele[0].Dec_Alt_Res = DEF_ALT_RES1;

  strncpy(MyConfig.Tele[1].Name,DEF_NAME2, BT_NAME_SIZE);     
  MyConfig.Tele[1].RA_Az_SensorType = DEF_AZ_ENC2;
  MyConfig.Tele[1].Dec_Alt_SensorType = DEF_ALT_ENC2;
  MyConfig.Tele[1].RA_Az_Res = DEF_AZ_RES2;
  MyConfig.Tele[1].Dec_Alt_Res = DEF_ALT_RES2;

  strncpy(MyConfig.Tele[2].Name,DEF_NAME3, BT_NAME_SIZE);     
  MyConfig.Tele[2].RA_Az_SensorType = DEF_AZ_ENC3;
  MyConfig.Tele[2].Dec_Alt_SensorType = DEF_ALT_ENC3;
  MyConfig.Tele[2].RA_Az_Res = DEF_AZ_RES3;
  MyConfig.Tele[2].Dec_Alt_Res = DEF_ALT_RES3;

  strncpy(MyConfig.Tele[3].Name,DEF_NAME4, BT_NAME_SIZE);    
  MyConfig.Tele[3].RA_Az_SensorType = DEF_AZ_ENC4;
  MyConfig.Tele[3].Dec_Alt_SensorType = DEF_ALT_ENC4;
  MyConfig.Tele[3].RA_Az_Res = DEF_AZ_RES4;
  MyConfig.Tele[3].Dec_Alt_Res = DEF_ALT_RES4;

  MyConfig.AutoPowerOff = DEF_AUTOPOWEROFF;
  MyConfig.Backlight = DEF_BACKLIGHT;

  eventsPutFifo(EVENT_RESTORE_DATA);
}

bool checkVersionEEPROM()
{
  return(EEPROM.read(EEPROM_ADD) == EEPROM_VERSION);
}

void validateEEPROM()
{
  if(MyConfig.Version != EEPROM_VERSION) MyConfig.Version = EEPROM_VERSION;
  if(MyConfig.CurrentMount < 0) MyConfig.CurrentMount = 0;
  if(MyConfig.CurrentMount > (MAX_TELESCOPES-1)) MyConfig.CurrentMount = (MAX_TELESCOPES-1);
  for(int i = 0; i < MAX_TELESCOPES; i++)
  {      
      if(strlen(MyConfig.Tele[i].Name) > 8) sprintf(MyConfig.Tele[i].Name,"Mount %d", i + 1);
      if(MyConfig.Tele[i].RA_Az_SensorType < 0) MyConfig.Tele[i].RA_Az_SensorType = 0;
      if(MyConfig.Tele[i].RA_Az_SensorType > ST_MAX_SENSOR) MyConfig.Tele[i].RA_Az_SensorType = ST_MAX_SENSOR;
      
      if(MyConfig.Tele[i].Dec_Alt_SensorType < 0) MyConfig.Tele[i].Dec_Alt_SensorType = 0;
      if(MyConfig.Tele[i].Dec_Alt_SensorType > ST_MAX_SENSOR) MyConfig.Tele[i].Dec_Alt_SensorType = ST_MAX_SENSOR;
      
      if(MyConfig.Tele[i].RA_Az_Res < MIN_RES) MyConfig.Tele[i].RA_Az_Res = MIN_RES;
      if(MyConfig.Tele[i].RA_Az_Res > MAX_RES) MyConfig.Tele[i].RA_Az_Res = MAX_RES;
      
      if(MyConfig.Tele[i].Dec_Alt_Res < MIN_RES) MyConfig.Tele[i].Dec_Alt_Res = MIN_RES;
      if(MyConfig.Tele[i].Dec_Alt_Res > MAX_RES) MyConfig.Tele[i].Dec_Alt_Res = MAX_RES;
  }
  if(MyConfig.Backlight < 0 ||  MyConfig.Backlight > 100) MyConfig.Backlight = DEF_BACKLIGHT;
}

