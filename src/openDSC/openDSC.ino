/******************************************
 * openDSC
 * Bluetooth and WiFi AP
 * ESP-TTGP Module
 * https://github.com/xvedra/OpenDsc
 * Debug: USB
 * 
 * see:
 * https://github.com/espressif/arduino-esp32
 * https://github.com/Bodmer/TFT_eSPI
 * https://github.com/Xinyuan-LilyGO/TTGO-T-Display
 * https://github.com/LennartHennigs/Button2
 * https://github.com/Akisoft41/Documentations/wiki/TTGO-T-Display
 * http://www.rinkydinkelectronics.com/t_imageconverter565.php
 * https://github.com/neu-rah/ArduinoMenu/wiki
 * https://www.thingiverse.com/thing:4183337
 * 
 * based in:
 * https://github.com/synfinatic/teensy-dsc
 * https://github.com/synfinatic/esp-dsc
 *****************************************/
 /*
  * Note:
https://github.com/Xinyuan-LilyGO/TTGO-T-Display/issues/6
So you can set PWR_EN to "1" and read voltage in BAT_ADC, if this voltage more than 4.3 V device have powered from USB. 
If less then 4.3 V - device have power from battery. 
To save battery you can set PWR_EN to "0" and in this case USB converter will be power off and do not use your battery. 
When you need to measure battery voltage first set PWR_EN to "1", measure voltage and then set PWR_EN back to "0" for save battery
*/


#include <EEPROM.h>
#include <WiFi.h>
//#include <Wire.h>
#include <esp_wifi.h>

#include "def.h"
#include "config.h"
#include "dsc.h"
#include "Commands.h"
#include "events.h"
#include "tft.h"
#include "pinMap.h"

// Windows //////////////////////////////////////////////////////////////////////
#define WIN_MENU 0
#define WIN_MAIN 1
#define WIN_DEMO 2
#define WIN_DEFAULT WIN_DEMO
/////////////////////////////////////////////////////////////////////////////////
#define IDEL_EVENT_FIFO_SIZE 8
#define IDEL_EVENT_FIFO_MASK 0x07
/////////////////////////////////////////////////////////////////////////////////
using namespace Menu;
TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
Button2 btnUp(BTN_UP); // Initialize the up button
Button2 btnDwn(BTN_DWN); // Initialize the down button
result doAlert(eventMask e, prompt &item);
/////////////////////////////////////////////////////////////////////////////////
char buff[128];
const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;
EEPROM_Config MyConfig;
byte myUsbSerialBaudrateIndex = 0;
TelescopeSensorMount CurrentSensorMountConfig;
int AutoPowerOffTimer = 0;
char* constMEM onlySign MEMMODE="+-";
char* constMEM onlyIntegers MEMMODE="0123456789";
char* constMEM onlyIntegersKeys[] MEMMODE={onlySign,onlyIntegers,onlyIntegers,onlyIntegers,onlyIntegers,onlyIntegers};
char* constMEM qwerty MEMMODE="+-_0123456789ABCDEFGHIJKLMNÑOPQRSTUVWXYZabcdefghijklmnñopqrstuvwxyz";
char* constMEM qwertyKeys[] MEMMODE={qwerty,qwerty,qwerty,qwerty,qwerty,qwerty,qwerty,qwerty};
char RAAzBuf[8] = "+10000";
char DecAltBuf[8] = "+10000";
bool isIdle = 0;
int wirelessStatus = 0;
WirelessStatus myWirelessStatus;
//////////////////////////////////////////////////////////////////////
// MENU SETUP ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
#ifdef USE_DEBUG
serialIn serial(Serial);
#else
chainStream<0> serial(NULL);//<-- this creates a NULL stream
#endif
//MENU_INPUTS(in, &serial);
//define serial output device
idx_t serialTops[MAX_DEPTH]={0};
serialOut outSerial(Serial,serialTops);
constMEM panel panels[] MEMMODE = {{0, 0, GFX_WIDTH / fontW, GFX_HEIGHT / fontH}};
navNode* nodes[sizeof(panels) / sizeof(panel)]; //navNodes to store navigation status
panelsList pList(panels, nodes, 1); //a list of panels and nodes
idx_t eSpiTops[MAX_DEPTH]={0};
TFT_eSPIOut eSpiOut(tft,colors,eSpiTops,pList,fontW,fontH+1);
//TFT_eSPIOut eSpiOut(tft,nightColors,eSpiTops,pList,fontW,fontH+1);
#ifdef USE_DEBUG
menuOut* constMEM outputs[] MEMMODE={&outSerial,&eSpiOut};//list of output devices
#else
menuOut* constMEM outputs[] MEMMODE={&eSpiOut};//list of output devices
#endif
outputsList out(outputs,sizeof(outputs)/sizeof(menuOut*));//outputs list controller
//config myOptions('*','-',defaultNavCodes,true);
//////////////////////////////////////////////////////////////////////
// EDIT MENU  ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// AutopowerOff //////////////////////////////////////////////
result Action_AutoPowerOff() {
  #ifdef USE_DEBUG
  Serial.printf("\n\nAutoPwrOff Off: %d\n", MyConfig.AutoPowerOff);
  #endif
  #ifndef USE_SAVE_AT_EXIT_MENU
  uploadEEPROM();
  #endif
  eventsPutFifo(EVENT_AUTOPOWEROFF_SET);
  return proceed;
}
TOGGLE(MyConfig.AutoPowerOff,setAutoPowerOff,"AutoPowerOff: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("Off",0, Action_AutoPowerOff, enterEvent)
  ,VALUE("10sec", 10, Action_AutoPowerOff, enterEvent)
  ,VALUE("30sec", 30, Action_AutoPowerOff, enterEvent)
  ,VALUE("1 min", 60, Action_AutoPowerOff, enterEvent)
  ,VALUE("5 min", 300, Action_AutoPowerOff, enterEvent)
  ,VALUE("10min", 600, Action_AutoPowerOff, enterEvent)
);

// Backlight /////////////////////////////////////////////////
result Action_Backlight() {
  #ifdef USE_DEBUG
  Serial.printf("\n\nledBacklight: %d\n", (int)MyConfig.Backlight);
  #endif
  #ifndef USE_SAVE_AT_EXIT_MENU
  uploadEEPROM();
  #endif
  return proceed;
}
TOGGLE(MyConfig.Backlight, setBacklight,"Backlight: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("100%", 100, Action_Backlight, enterEvent)
  ,VALUE("80%", 80, Action_Backlight, enterEvent)
  ,VALUE("60%", 60, Action_Backlight, enterEvent)
  ,VALUE("40%", 40, Action_Backlight, enterEvent)
  ,VALUE("20%", 20, Action_Backlight, enterEvent)
  ,VALUE("10%", 10, Action_Backlight, enterEvent)
  ,VALUE("5%", 5, Action_Backlight, enterEvent)
  ,VALUE("1%", 1, Action_Backlight, enterEvent)
);
// Choose Telescope Mount /////////////////////////////////// 
result Action_chooseTelescope() {
  #ifdef USE_DEBUG
  Serial.printf("\n\nChooseTelescope: %d\n", (int)MyConfig.CurrentMount);
  #endif
  #ifndef USE_SAVE_AT_EXIT_MENU
  uploadEEPROM();
  #endif
  eventsPutFifo(EVENT_MOUNT_SET);
  updateEncoders();  
  return proceed;
}
TOGGLE(MyConfig.CurrentMount,chooseTelescopeMenu,"Telescope:",doNothing,noEvent,noStyle
  ,VALUE("Mount 1",0,Action_chooseTelescope,enterEvent)
  ,VALUE("Mount 2",1,Action_chooseTelescope,enterEvent)
  ,VALUE("Mount 3",2,Action_chooseTelescope,enterEvent)
  ,VALUE("Mount 4",3,Action_chooseTelescope,enterEvent)
);
// customizing a prompt look! ///////////////////////////////
//by extending the prompt class
/*
class altPrompt:public prompt {
public:
  altPrompt(constMEM promptShadow& p):prompt(p) {}
  Used printTo(navRoot &root,bool sel,menuOut& out, idx_t idx,idx_t len,idx_t) override {
    return out.printRaw(F("special prompt!"),len);;
  }
};

MENU(subMenu,"Sub-Menu",doNothing,noEvent,noStyle
  ,altOP(altPrompt,"",doNothing,noEvent)
  ,OP("Op",doNothing,noEvent)
  ,EXIT("<Back")
);
*/
// Configure Telescope SubMenu ///////////////////////////////
result Action_chooseRAAzSensor()
{
  #ifdef USE_DEBUG
  Serial.printf("\n\nAction_chooseRAAzSensor\n");
  #endif
  MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_SensorType = CurrentSensorMountConfig.RA_Az_SensorType;
  #ifndef USE_SAVE_AT_EXIT_MENU
  uploadEEPROM();
  #endif
  eventsPutFifo(EVENT_AZ_ENC_SET);
  updateEncoders();
  return proceed;
}

TOGGLE(CurrentSensorMountConfig.RA_Az_SensorType,chooseRAAzSensorMenu,"RA/Az Enc:  ",doNothing,noEvent,noStyle
  //,VALUE("Quadrature",ST_QUADRATURE,Action_chooseRAAzSensor,noEvent)
  ,VALUE("AS5600",ST_AS5600,Action_chooseRAAzSensor,noEvent)
  ,VALUE("AS5048",ST_AS5048,Action_chooseRAAzSensor,noEvent)
  ,VALUE("MPU6050",ST_MPU6050,Action_chooseRAAzSensor,noEvent)
);

result Action_chooseDecAltSensor()
{
  #ifdef USE_DEBUG
  Serial.printf("\n\nAction_chooseDecAltSensor\n");
  #endif
  MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_SensorType = CurrentSensorMountConfig.Dec_Alt_SensorType;
  #ifndef USE_SAVE_AT_EXIT_MENU
  uploadEEPROM();
  #endif
  eventsPutFifo(EVENT_ALT_ENC_SET);
  updateEncoders();
  return proceed;
}

TOGGLE(CurrentSensorMountConfig.Dec_Alt_SensorType,chooseDecAltSensorMenu,"Rec/Alt Enc:",doNothing,noEvent,noStyle
  //,VALUE("Quadrature",ST_QUADRATURE,Action_chooseDecAltSensor,noEvent)
  ,VALUE("AS5600",ST_AS5600,Action_chooseDecAltSensor,noEvent)
  ,VALUE("AS5048",ST_AS5048,Action_chooseDecAltSensor,noEvent)
  ,VALUE("MPU6050",ST_MPU6050,Action_chooseDecAltSensor,noEvent)
);

result Action_subMenuConfig() {  
  CurrentSensorMountConfig = MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES];
  sprintf(RAAzBuf,"%c%05d", CurrentSensorMountConfig.RA_Az_Res == abs(CurrentSensorMountConfig.RA_Az_Res)?'+':'-', abs(CurrentSensorMountConfig.RA_Az_Res));
  sprintf(DecAltBuf,"%c%05d", CurrentSensorMountConfig.Dec_Alt_Res == abs(CurrentSensorMountConfig.Dec_Alt_Res)?'+':'-', abs(CurrentSensorMountConfig.Dec_Alt_Res));

  #ifdef USE_DEBUG
  //Serial.printf("\n");
  //Serial.printf("Loading mount config: %d\n", MyConfig.CurrentMount);
  //Serial.printf("RA Az Sensor: %d\n", CurrentSensorMountConfig.RA_Az_SensorType);
  //Serial.printf("Dec Alt Sensor: %d\n", CurrentSensorMountConfig.Dec_Alt_SensorType);
  //Serial.printf("RA Az res: %d (%s)\n", CurrentSensorMountConfig.RA_Az_Res, RAAzBuf);
  //Serial.printf("Dec Alt res: %d (%s)\n", CurrentSensorMountConfig.Dec_Alt_Res, DecAltBuf);
  #endif

  #ifdef USE_DEBUG
  Serial.printf("\nsubMenuConfig: %s:%d\t%s:%d\n\n", RAAzBuf, atol(RAAzBuf), DecAltBuf, atol(DecAltBuf));
  #endif
  #ifndef USE_SAVE_AT_EXIT_MENU
  uploadEEPROM();
  #endif
  
  updateEncoders();
  return proceed;
}
  
result Action_subMenuUploadConfig() {  

  if(strncmp(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Name, CurrentSensorMountConfig.Name, BT_NAME_SIZE))
  {
    strncpy(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Name, CurrentSensorMountConfig.Name, BT_NAME_SIZE);  
    eventsPutFifo(EVENT_MOUNT_NAME_SET);
  }
  if(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_Res != atol(RAAzBuf))
  {
    MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_Res = atol(RAAzBuf);
    eventsPutFifo(EVENT_AZ_RES_SET);
  }
  if(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_Res != atol(DecAltBuf))
  {
    MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_Res = atol(DecAltBuf);
    eventsPutFifo(EVENT_ALT_RES_SET);
  }

  #ifdef USE_DEBUG
  Serial.printf("\nsubMenuUploadConfig: Az:%s:%d\tAlt:%s:%d\n\n", RAAzBuf, atol(RAAzBuf), DecAltBuf, atol(DecAltBuf));
  #endif
  #ifndef USE_SAVE_AT_EXIT_MENU
  uploadEEPROM();
  #endif
  updateEncoders();
  return proceed;
}

MENU(subMenuConfig,"Configure telescope",Action_subMenuConfig,enterEvent,noStyle
  ,EDIT("Name: ",CurrentSensorMountConfig.Name,qwertyKeys,Action_subMenuUploadConfig,enterEvent,noStyle)
  ,SUBMENU(chooseRAAzSensorMenu)
  ,SUBMENU(chooseDecAltSensorMenu)
  ,EDIT("RA/Az Res:  ",RAAzBuf,onlyIntegersKeys,Action_subMenuUploadConfig,exitEvent,noStyle)
  ,EDIT("Dec/Alt Res:",DecAltBuf,onlyIntegersKeys,Action_subMenuUploadConfig,exitEvent,noStyle)
  ,EXIT("<Back")
);

// Communications /////////////////////////////////////////////////
TOGGLE(myWirelessStatus.BT_switched,chooseBluetoothMenu,"Bluetooth:",doNothing,noEvent,noStyle
  ,VALUE("ON",1,doNothing,noEvent)
  ,VALUE("OFF",0,doNothing,noEvent)
);
TOGGLE(myWirelessStatus.WiFi_switched,chooseWiFiMenu,"WiFi:",doNothing,noEvent,noStyle
  ,VALUE("ON",1,doNothing,noEvent)
  ,VALUE("OFF",0,doNothing,noEvent)
);
TOGGLE(myUsbSerialBaudrateIndex,chooseSerialMenu,"Serial:",doNothing,noEvent,noStyle
  ,VALUE("1200",DEF_BR_1200,doNothing,noEvent)
  ,VALUE("2400",DEF_BR_2400,doNothing,noEvent)
  ,VALUE("4800",DEF_BR_4800,doNothing,noEvent)
  ,VALUE("9600",DEF_BR_9600,doNothing,noEvent)
  ,VALUE("19200",DEF_BR_19200,doNothing,noEvent)
  ,VALUE("38400",DEF_BR_38400,doNothing,noEvent)
  ,VALUE("57600",DEF_BR_57600,doNothing,noEvent)
  ,VALUE("115200",DEF_BR_115200,doNothing,noEvent)
);
// WiFi Tx Power ////////////////////////////////////////////

result Action_chooseWiFiTxPower()
{
  if(esp_wifi_set_max_tx_power(MyConfig.WiFiTxPower) != ESP_OK)
  {
    #ifdef USE_DEBUG
    Serial.printf("\n\nAction_chooseWiFiTxPower: Fail\n");
    #endif
  }
  else
  {
    #ifdef USE_DEBUG
    Serial.printf("\n\nAction_chooseWiFiTxPower: Success\n");
    #endif
  }
  return proceed;
}

TOGGLE(MyConfig.WiFiTxPower,chooseWiFiTxPowerMenu,"WiFi pwr:",doNothing,noEvent,noStyle
  ,VALUE("Very high",WIFI_TX_PWR_VERYHIGH,Action_chooseWiFiTxPower,noEvent)
  ,VALUE("High",WIFI_TX_PWR_HIGH,Action_chooseWiFiTxPower,noEvent)
  ,VALUE("Middle",WIFI_TX_PWR_MIDDLE,Action_chooseWiFiTxPower,noEvent)
  ,VALUE("Low",WIFI_TX_PWR_LOW,Action_chooseWiFiTxPower,noEvent)
  ,VALUE("Very low",WIFI_TX_PWR_VERYLOW,Action_chooseWiFiTxPower,noEvent)
);
// Restore //////////////////////////////////////////////////
result Action_Restore()
{
  #ifdef USE_DEBUG
  Serial.printf("\n\nAction Restore..."); 
  #endif
  //loadPartialDefEEPROM();
  loadDefEEPROM();
  uploadEEPROM();
  //updateEncoders();
  eSpiOut.clear();
  eSpiOut.println("\nReset...\n");
  for(int i = 0; i < 20; i++)
  {
    eSpiOut.print("*");
    delay(200);
  }
  ESP.restart();
  #ifdef USE_DEBUG
  //Serial.printf("done\n");
  #endif
  return quit;
}
MENU(SubMenuRestore,"Restore",doNothing,noEvent,wrapStyle
  ,EXIT("No")
  ,OP("Yes",Action_Restore,enterEvent)
);

// Main menu ////////////////////////////////////////////////
MENU(mainMenu,"OpenDSC menu",doNothing,noEvent,wrapStyle
  ,SUBMENU(chooseTelescopeMenu)
  ,SUBMENU(subMenuConfig) 
  ,SUBMENU(chooseBluetoothMenu)
  ,SUBMENU(chooseWiFiMenu)
  ,SUBMENU(chooseWiFiTxPowerMenu)
  #ifndef USE_DEBUG
  ,SUBMENU(chooseSerialMenu)
  #endif
  ,SUBMENU(setBacklight)
  ,SUBMENU(setAutoPowerOff)  
  ,OP(ESP_DSC_BT_NAME,doNothing,noEvent)  
  ,OP(ESP_DSC_WIFI_NAME,doNothing,noEvent) 
  ,OP(ESP_DSC_WIFI_PASS,doNothing,noEvent) 
  ,OP(ESP_DSC_WIFI_IP,doNothing,noEvent) 
  ,OP(ESP_DSC_ABOUT,doNothing,noEvent)
  ,SUBMENU(SubMenuRestore)  
  //,EXIT("<Back")
);

//////////////////////////////////////////////////////////////////////
// MENU //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
NAVROOT(nav,mainMenu,MAX_DEPTH,serial,out);
//////////////////////////////////////////////////////////////////////
// IDLE HOOK /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//when menu is suspended
result idle(menuOut& o,idleEvent e) {//idleStart,idling,idleEnd

  //o.clear();
  switch(e) {
    case idleStart:    
    //o.println("\nsuspending menu!\n");   
    //o.clear();    
    eventsPutFifo(EVENT_IDLE_START);
    break;
    
    case idling:
    //o.println("\nsuspended...\n");
    isIdle = 1;
    //o.clear();   
    eventsPutFifo(EVENT_IDLE_DONE);
    break;
    
    case idleEnd:
    //o.println("\nresuming menu.\n");    
    isIdle = 0;
    //o.clear();
    eventsPutFifo(EVENT_IDLE_END);
    break;
  }
  
  return proceed;
/*  
      // Show the idle message once
      int xpos = tft.width() / 2; // Half the screen width
      tft.fillScreen(Black);
      
      tft.setTextSize(5);
      tft.setTextColor(Yellow,Black);
      tft.setTextWrap(false);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("IDLE", xpos, 50);
      int getFontHeight = tft.fontHeight();
      tft.setTextSize(2);
      tft.setTextColor(White,Black);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("Long press a button", xpos, 90);
      tft.drawString("to exit", xpos, 110);

      return proceed;
*/  
}

//////////////////////////////////////////////////////////////////////
// BUTTONS INIT //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void button_init()
{
    // Button UP ////////////////////////////////////
    btnUp.setChangedHandler([](Button2 & b) {eventsPutFifo(EVENT_BTN_UP_CHANGE);});
    btnUp.setPressedHandler([](Button2 & b)
    {
      unsigned int time = b.wasPressedFor();
      if (time >= 1000) eventsPutFifo(EVENT_BTN_UP_LONG_PRESSED);
      else eventsPutFifo(EVENT_BTN_UP_PRESSED);
    });
    btnUp.setReleasedHandler([](Button2 & b) {eventsPutFifo(EVENT_BTN_UP_RELEASED);});
    btnUp.setTapHandler([](Button2 & b) { eventsPutFifo(EVENT_BTN_UP_TAP);});
    btnUp.setDoubleClickHandler([](Button2 & b) {eventsPutFifo(EVENT_BTN_UP_DOUBLE_CLICK);});
    btnUp.setTripleClickHandler([](Button2 & b) {eventsPutFifo(EVENT_BTN_UP_TRIPLE_CLICK);});
    btnUp.setLongClickHandler([](Button2 & b) 
    {
        unsigned int time = b.wasPressedFor(); 
        if(time >= 1000) eventsPutFifo(EVENT_BTN_UP_VERY_LONG_CLICK);
        else eventsPutFifo(EVENT_BTN_UP_LONG_CLICK); 
    });    
    btnUp.setClickHandler([](Button2 & b) {eventsPutFifo(EVENT_BTN_UP_CLICK);});
    // Button DWN ////////////////////////////////////
    btnDwn.setChangedHandler([](Button2 & b) {eventsPutFifo(EVENT_BTN_DWN_CHANGE);});
    btnDwn.setPressedHandler([](Button2 & b)
    {
      unsigned int time = b.wasPressedFor();
      if (time >= 1000) eventsPutFifo(EVENT_BTN_DWN_LONG_PRESSED);
      else eventsPutFifo(EVENT_BTN_DWN_PRESSED);
    });
    btnDwn.setReleasedHandler([](Button2 & b) {eventsPutFifo(EVENT_BTN_DWN_RELEASED);});
    btnDwn.setTapHandler([](Button2 & b) {eventsPutFifo(EVENT_BTN_DWN_TAP);});
    btnDwn.setDoubleClickHandler([](Button2 & b) {eventsPutFifo(EVENT_BTN_DWN_DOUBLE_CLICK);});
    btnDwn.setTripleClickHandler([](Button2 & b) {eventsPutFifo(EVENT_BTN_DWN_TRIPLE_CLICK);});
    btnDwn.setLongClickHandler([](Button2 & b) 
    {
        unsigned int time = b.wasPressedFor();        
        if (time >= 1000) eventsPutFifo(EVENT_BTN_DWN_VERY_LONG_CLICK);       
        else eventsPutFifo(EVENT_BTN_DWN_LONG_CLICK);  
    });    
    btnDwn.setClickHandler([](Button2 & b) {eventsPutFifo(EVENT_BTN_DWN_CLICK);});
}

//////////////////////////////////////////////////////////////////////
// BUTTONS LOOP //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void button_loop()
{
    // Check for button presses
    btnUp.loop();
    btnDwn.loop();

    if(eventsReadCurrentEvent() == EVENT_BTN_UP_CHANGE || 
       eventsReadCurrentEvent() == EVENT_BTN_DWN_CHANGE)
    {
      AutoPowerOffTimer = MyConfig.AutoPowerOff; //Restart timeout screenoff
    }
    if(AutoPowerOffTimer || MyConfig.AutoPowerOff == 0) //Screen on..
    {
      if(eventsReadCurrentEvent() == EVENT_BTN_UP_CLICK) nav.doNav(downCmd);
      if(eventsReadCurrentEvent() == EVENT_BTN_UP_LONG_CLICK && !isIdle) nav.doNav(enterCmd);
      if(eventsReadCurrentEvent() == EVENT_BTN_DWN_CLICK) nav.doNav(upCmd);
      if(eventsReadCurrentEvent() == EVENT_BTN_DWN_VERY_LONG_CLICK) nav.doNav(escCmd); 
    }
}

/*
//! Long time delay, it is recommended to use shallow sleep, which can effectively reduce the current consumption
void espDelay(int ms)
{
    esp_sleep_enable_timer_wakeup(ms * 1000);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    esp_light_sleep_start();
}
*/

//////////////////////////////////////////////////////////////////////
// BACKLIGHT AUTOPOWEROFF ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void AutoPowerOffLoop()
{
  static time_t timer = 0;
  static int LastAutoPowerOffCtrl = -1;
  static bool forceOff = 0;
  static bool lightOn = 1;

  if(MyConfig.AutoPowerOff != LastAutoPowerOffCtrl)
  {
    LastAutoPowerOffCtrl = MyConfig.AutoPowerOff;
    AutoPowerOffTimer = MyConfig.AutoPowerOff;
  }    
  if(MyConfig.AutoPowerOff == 0)
  {   
    timer = InitTimer(0);
    AutoPowerOffTimer = -1;
  }
  else
  {
    if(AutoPowerOffTimer < 0) AutoPowerOffTimer = MyConfig.AutoPowerOff;
    else if(checkTimer(timer, 1000))
    {
      timer = InitTimer(0);
      if(AutoPowerOffTimer) AutoPowerOffTimer--;     
    }
  }
  if(isIdle)
  {
    if(eventsReadCurrentEvent() == EVENT_BTN_DWN_CLICK) forceOff = !forceOff;
  }
  else forceOff = 0;
  
  if(AutoPowerOffTimer && !forceOff)
  {
    ledcWrite(pwmLedChannelTFT, (int)MyConfig.Backlight*255/100);
    if(!lightOn)
    {
      lightOn = 1;
      eventsPutFifo(EVENT_AUTOPOWEROFF_ON); 
    }
  }
  else
  {
    ledcWrite(pwmLedChannelTFT, 0);
    if(lightOn)
    {
      lightOn = 0;
      eventsPutFifo(EVENT_AUTOPOWEROFF_OFF); 
    }
  }
}


void updateEncoders()
{
  Serial.printf("Update encoders in Mount %d\n", MyConfig.CurrentMount);
  dsc_SetAltSensor(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_SensorType);
  dsc_SetAzSensor(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_SensorType);
  dsc_SetAltRes(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_Res);
  dsc_SetAzRes(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_Res);  
  dsc_SetAlt(abs(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_Res)/2);
  dsc_SetAz(abs(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_Res)/2);
}

//////////////////////////////////////////////////////////////////////
// CHECK WIRELESS ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void checkWirelessLoop()
{
  static WirelessStatus lastStatus = {0,0,0};
  static time_t timer = 0;
  extern unsigned long int BAUDRATES[];
  
  if(myUsbSerialBaudrateIndex != MyConfig.USB)
  {
    Serial.printf("New seial baudrate: %d : %d\n", myUsbSerialBaudrateIndex, BAUDRATES[myUsbSerialBaudrateIndex&7]);
    MyConfig.USB = myUsbSerialBaudrateIndex;
    uploadEEPROM();
    connectUsbSerial(myUsbSerialBaudrateIndex);
    eventsPutFifo(EVENT_SERIAL_SET);    
  }
  
  if(myWirelessStatus.BT_switched && !MyConfig.Bluetooth)
  {    
    myWirelessStatus.str.BT_enabled = 1;
    MyConfig.Bluetooth = 1;
    uploadEEPROM();
    connectBluetooth();
  }
  if(!myWirelessStatus.BT_switched && MyConfig.Bluetooth)
  {
    myWirelessStatus.str.BT_enabled = 0;
    MyConfig.Bluetooth = 0;
    uploadEEPROM();
    disconnectBluetooth();
  }

  if(myWirelessStatus.WiFi_switched && !MyConfig.WiFi)
  {
    myWirelessStatus.str.WiFi_enabled = 0;
    MyConfig.WiFi = 1;
    uploadEEPROM();
    connectWIFI();
    esp_wifi_set_max_tx_power(MyConfig.WiFiTxPower);
  }
  if(!myWirelessStatus.WiFi_switched && MyConfig.WiFi)
  {
    myWirelessStatus.str.WiFi_enabled = 1;
    MyConfig.WiFi = 0;
    uploadEEPROM();
    disconnectWIFI();
  }

  //Check connection
  //wirelessStatus = isBluetoothEnabled();
  //wirelessStatus = wirelessStatus + (int)isBtEnabled() * 2;
  //wirelessStatus = wirelessStatus + (int)isWiFiEnabled() * 4;
  //wirelessStatus = wirelessStatus + (int)isWiFiConnected() * 8;

  myWirelessStatus.str.BT_enabled = isBluetoothEnabled();
  myWirelessStatus.str.BT_connected = isBtConnected();
  myWirelessStatus.str.WiFi_enabled = isWiFiEnabled();
  myWirelessStatus.str.WiFi_connected = isWiFiConnected();
  
  //myConnectEvent = 0;
  if(myWirelessStatus.reg != lastStatus.reg)
  {
    if(myWirelessStatus.str.BT_enabled && !lastStatus.str.BT_enabled) eventsPutFifo(EVENT_BT_ENABLED);
    if(!myWirelessStatus.str.BT_enabled && lastStatus.str.BT_enabled) eventsPutFifo(EVENT_BT_DISABLED);

    if(myWirelessStatus.str.BT_connected && !lastStatus.str.BT_connected) eventsPutFifo(EVENT_BT_CONNECTED);
    if(!myWirelessStatus.str.BT_connected && lastStatus.str.BT_connected) eventsPutFifo(EVENT_BT_DISCONNECTED);

    if(myWirelessStatus.str.WiFi_enabled && !lastStatus.str.WiFi_enabled) eventsPutFifo(EVENT_WIFI_ENABLED);
    if(!myWirelessStatus.str.WiFi_enabled && lastStatus.str.WiFi_enabled) eventsPutFifo(EVENT_WIFI_DISABLED);

    if(myWirelessStatus.str.WiFi_connected && !lastStatus.str.WiFi_connected) eventsPutFifo(EVENT_WIFI_CONNECTED);
    if(!myWirelessStatus.str.WiFi_connected && lastStatus.str.WiFi_connected) eventsPutFifo(EVENT_WIFI_DISCONNECTED);
    
    lastStatus.reg = myWirelessStatus.reg;
    eventsPutFifo(EVENT_WIRELESS_CHANGE);
  }

  //Enable/disable menus
  if(!isBtConnected() && !isWiFiConnected())
  {    
    //Disconnected
    mainMenu[0].enable();
    mainMenu[1].enable();
    mainMenu[2].enable();    
    mainMenu[3].enable(); 
    #ifdef USE_DEBUG
    mainMenu[12].enable();
    #else
    mainMenu[13].enable();
    #endif
  }
  if(isBtConnected() || isWiFiConnected())
  {
    //Connected
    mainMenu[0].disable();
    mainMenu[1].disable();
    mainMenu[2].disable();
    mainMenu[3].disable();
    #ifdef USE_DEBUG
    mainMenu[12].disable();
    #else
    mainMenu[13].enable();
    #endif
  }
  
  if(isWiFiEnabled()) mainMenu[4].enable();
  else mainMenu[4].disable();
}

//////////////////////////////////////////////////////////////////////
// SETUP /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void setup()
{
  extern unsigned long int BAUDRATES[];
  char buf[48];
  pinMode(ADC_EN, OUTPUT);
  //digitalWrite(ADC_EN, HIGH);
  digitalWrite(ADC_EN, LOW);
  EEPROM.begin(EEPROM_SIZE);   
  eventsInit();

/*
  if (TFT_BL > 0) { // TFT_BL has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
      pinMode(TFT_BL, OUTPUT); // Set backlight pin to output mode
      digitalWrite(TFT_BL, TFT_BACKLIGHT_ON); // Turn backlight on. TFT_BACKLIGHT_ON has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
  }
*/
  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin(TFT_BL, pwmLedChannelTFT);
  ledcWrite(pwmLedChannelTFT, 0);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);      
  tft.setSwapBytes(true);  
  tft.setTextSize(2);
  tft.setTextWrap(false);
   

  // End of tft config ////////////////////////////
  delay(500*SLOW_BOOT);
  tft.setTextColor(TFT_RED);    
  ledcWrite(pwmLedChannelTFT, 100);
  if(checkVersionEEPROM())
  {
    loadEEPROM();
    tft.drawString("EEPROM loaded", 5, 24*0);
  }
  else
  {
    loadDefEEPROM();
    tft.drawString("Def EEPROM loaded", 5, 24*0);
  }
  uploadEEPROM();
  delay(500*SLOW_BOOT);
  
  dsc_SetAltSensor(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_SensorType);
  dsc_SetAzSensor(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_SensorType);
  
  dsc_SetAltRes(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_Res);
  dsc_SetAzRes(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_Res);  
  
  dsc_SetAlt(abs(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_Res)/2);
  dsc_SetAz(abs(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_Res)/2);

  sprintf(buf, "Mount %d: %s", MyConfig.CurrentMount%MAX_TELESCOPES, MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Name);
  tft.drawString(buf, 5, 24*1);
  delay(500*SLOW_BOOT);
  sprintf(buf, "Az Res: %ld", MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_Res);
  tft.drawString(buf, 5, 24*2);
  delay(500*SLOW_BOOT);
  sprintf(buf, "Alt Res: %ld", MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_Res);
  tft.drawString(buf, 5, 24*3);
  delay(500*SLOW_BOOT);
  
  dsc_Init();
  dsc_Enable();

  delay(2000*SLOW_BOOT);
  tft.fillScreen(TFT_BLACK);
  //#ifdef USE_DEBUG
  tft.drawString("DEBUG ON", 5, 24*0);
  //#else
  tft.drawString("DEBUG OFF", 5, 24*0);
  //#endif
  delay(500*SLOW_BOOT);

  tft.drawString("Opening Serial...", 5, 24*1);
  myUsbSerialBaudrateIndex = MyConfig.USB;
  
  #ifdef USE_DEBUG
  Serial.begin(BAUDRATE_DEBUG);
  #else
  connectUsbSerial(MyConfig.USB);
  //Serial.printf("\nSerial: %d", BAUDRATES[myUsbSerialBaudrateIndex&7]);
  #endif  
  
  sprintf(buf, "done: %d", BAUDRATES[myUsbSerialBaudrateIndex]);
  tft.drawString(buf, 5, 24*2);
  delay(500*SLOW_BOOT);

  myWirelessStatus.reg = 0;
  // Bluetooth /////////////
  if(MyConfig.Bluetooth)
  {
    tft.drawString("Connect BT...", 5, 24*3);
    connectBluetooth();
    myWirelessStatus.BT_switched = 1;
    tft.drawString("done", 239-24*2, 24*3);
  }
  else
  {    
    myWirelessStatus.BT_switched = 0;
    tft.drawString("Disconnect BT", 5, 24*3);
  }  
  delay(500*SLOW_BOOT);

  // WiFi ////////////////
  if(MyConfig.WiFi)
  {
    tft.drawString("Connect WiFi...", 5, 24*4);
    connectWIFI();
    myWirelessStatus.WiFi_switched = 1;
    esp_wifi_set_max_tx_power(MyConfig.WiFiTxPower);    
    tft.drawString("done", 239-24*2, 24*4);
  }
  else
  {
    myWirelessStatus.WiFi_switched = 0;
    tft.drawString("Disconnect WiFi", 5, 24*4);
  }
  delay(1000*SLOW_BOOT);
  tft.fillScreen(TFT_BLACK);
  tft.drawString("Wellcome!", 5, 31*0);
  tft.drawString("openDSC", 5, 31*1);
  tft.drawString(ESP_DSC_VERSION, 5, 31*2);  
  tft.drawString("by xvedra, 2020", 5, 31*3);
  delay(2000*SLOW_BOOT);
  tft.fillScreen(TFT_BLACK);
  
  commandsInit();
  #ifdef USE_DEBUG
  Serial.printf("\nCurrent Config:\n");
  Serial.printf("Mount: %d\t%s\n", MyConfig.CurrentMount, MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Name);
  Serial.printf("Sensors: Dec_Alt:%d\tRA_Az:%d\n", MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_SensorType, MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_SensorType);
  Serial.printf("Res: Dec_Alt:%d\tRA_Az:%d\n", MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_Res, MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_Res);
  Serial.printf("Pos: Dec_Alt:%d\tRA_Az:%d\n\n", dsc_GetAlt(), dsc_GetAz());
  #endif

  // Lets go... ////////////////////////////////////  
  button_init();
  ledcWrite(pwmLedChannelTFT, 0);  
  #ifdef _OpenDSClogo
  tft.pushImage(52, 0,  135, 135, OpenDSClogo);
  #endif
  delay(10*SLOW_BOOT);
  ledcWrite(pwmLedChannelTFT, (int)MyConfig.Backlight*255/100);
  //espDelay(1000);    
  delay(2000*SLOW_BOOT);
  ledcWrite(pwmLedChannelTFT, 0);
  tft.fillScreen(TFT_BLACK);
  delay(100*SLOW_BOOT);
  ledcWrite(pwmLedChannelTFT, (int)MyConfig.Backlight*255/100);  
  tft.fillScreen(Black);
  nav.idleTask=idle;//point a function to be used when menu is suspended
  nav.idleOn(idle);
  isIdle = 1;   
}

//////////////////////////////////////////////////////////////////////
// LOOP //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void loop()
{
  eventsLoop();
  nav.poll();//this device only draws when needed
  commandsLoop();
  AutoPowerOffLoop();
  button_loop();  
  checkWirelessLoop();
  winLoop();    
}




