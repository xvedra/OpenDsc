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

#include <TFT_eSPI.h>
#include <SPI.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <Wire.h>
#include <Button2.h>
#include "def.h"
#include "config.h"
#include "dsc.h"
#include "Commands.h"


// bitmaps
#include "img/OpenDSClogo.h"
#include "img/BTonoff.h"
#include "img/VBatt.h"
#include "img/okfail1.h"
#include "img/WIFIonoff.h"

#include <menu.h>
#include <menuIO/serialIO.h>
#include <menuIO/TFT_eSPIOut.h>

#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN   0x10
#endif

#define TFT_MOSI            19
#define TFT_SCLK            18
#define TFT_CS              5
#define TFT_DC              16
#define TFT_RST             23

#define TFT_BL          4   // Display backlight control pin
#define ADC_EN          14  //ADC_EN is the ADC detection enable port
#define ADC_PIN         34
#define BUTTON_1        35
#define BUTTON_2        0
#define BTN_UP 35 // Pinnumber for button for up/previous and select / enter actions (don't change this if you want to use the onboard buttons)
#define BTN_DWN 0 // Pinnumber for button for down/next and back / exit actions (don't change this if you want to use the onboard buttons)

#define LED_CTRL        0

#define MAX_TELESCOPES 4
#define BT_NAME_SIZE 8
#define EEPROM_ADD 0
#define DEF_BACKLIGHT 80
#define DEF_AUTOPOWEROFF 0

#define GFX_WIDTH 240
#define GFX_HEIGHT 135
#define fontW 12
#define fontH 18
#define MAX_DEPTH 5

//////////////////////////////////////////////
#define EEPROM_SIZE (sizeof(MyConfig))
#define MAX_RES DEF_MAX_RES
#define MIN_RES DEF_MIN_RES
#define DEF_AZ_RES RES_AZ_DEF
#define DEF_ALT_RES RES_ALT_DEF

#define GRAPH_MAX_POINTS 60
#define battery_max 4.2
#define battery_min 2.5
#define battery_alert 2.8
#define DRAW_TEXT_COLOR1 DarkerRed
#define DRAW_TEXT_COLOR2 Red
#define DRAW_TEXT_COLOR3 TFT_DARKGREY //TFT_BROWN
#define DRAW_TEXT_COLOR4 TFT_ORANGE
#define DRAW_GRAPH_COLOR TFT_BROWN//TFT_VIOLET
#define DRAW_FULL_BATT_COLOR DarkerGreen
#define DRAW_MID_BATT_COLOR DarkerBlue
#define DRAW_LOW_BATT_COLOR DarkerRed

#define IDEL_EVENT_FIFO_SIZE 8
#define IDEL_EVENT_FIFO_MASK 0x07

//#define WIRELESS_BT 0
//#define WIRELESS_WIFI 1
//#define DEF_WIRELESS WIRELESS_BT//WIRELESS_WIFI
#define DEF_WIFI 0
#define DEF_BT 1

/*
 * //See def.h
#define SCL1_PIN  13
#define SDA1_PIN  15
#define SCL2_PIN  32
#define SDA2_PIN  33
*/
// Sensor Types ///////////////////////////
#define ST_QUADRATURE 0 //Not supported by TTGO. Under development
#define ST_AS5600     1
#define ST_AS5048     2
#define ST_MPU6050    3
#define ST_MAX_SENSOR ST_MPU6050

// define menu colors --------------------------------------------------------
//  {{disabled normal,disabled selected},{enabled normal,enabled selected, enabled editing}}
//monochromatic color table
#define Black RGB565(0,0,0)
#define Red  RGB565(255,0,0)
#define Green RGB565(0,255,0)
#define Blue RGB565(0,0,255)
#define Gray RGB565(128,128,128)
#define LighterRed RGB565(255,150,150)
#define LighterGreen RGB565(150,255,150)
#define LighterBlue RGB565(150,150,255)
#define DarkerRed RGB565(150,0,0)
#define DarkerGreen RGB565(0,150,0)
#define DarkerBlue RGB565(0,0,150)
#define Cyan RGB565(0,255,255)
#define Magenta RGB565(255,0,255)
#define Yellow RGB565(255,255,0)
#define White RGB565(255,255,255)

// TelescopeSensorMount //////////////////////
#define EEPROM_VERSION 1
// Remember upload EEPROM_VERSION every change in this structure!!!
typedef struct
{
  char Name[BT_NAME_SIZE+1] = "My Mount";
  byte RA_Az_SensorType = 0;
  byte Dec_Alt_SensorType = 0;
  long int RA_Az_Res = 4096;
  long int Dec_Alt_Res = 4096;  
} TelescopeSensorMount;
// Config ////////////////////////////////////
// Remember upload EEPROM_VERSION every change in this structure!!!
typedef struct
{
  byte Version;
  byte Wifi;
  byte Bluetooth;
  byte USB;
  byte CurrentMount;
  TelescopeSensorMount Tele[MAX_TELESCOPES];
  byte Backlight;
  int AutoPowerOff;
} EEPROM_Config;


const colorDef<uint16_t> colors[] MEMMODE={
  {
    {
      (uint16_t)Black,
      (uint16_t)Black
    },
    {
      (uint16_t)Black,
      (uint16_t)DarkerBlue,
      (uint16_t)DarkerBlue
    }
  },//bgColor
  {
    {
      (uint16_t)Gray,
      (uint16_t)Gray
    },
    {
      (uint16_t)White,
      (uint16_t)White,
      (uint16_t)White
    }
  },//fgColor
  {
    {
      (uint16_t)White,
      (uint16_t)Black
    },
    {
      (uint16_t)Yellow,
      (uint16_t)Yellow,
      (uint16_t)Red
    }
  },//valColor
  {
    {
      (uint16_t)White,
      (uint16_t)Black
    },
    {
      (uint16_t)White,
      (uint16_t)Yellow,
      (uint16_t)Yellow
    }
  },//unitColor
  {
    {
      (uint16_t)White,
      (uint16_t)Gray
    },
    {
      (uint16_t)Black,
      (uint16_t)Blue,
      (uint16_t)White
    }
  },//cursorColor
  {
    {
      (uint16_t)White,
      (uint16_t)Yellow
    },
    {
      (uint16_t)DarkerRed,
      (uint16_t)White,
      (uint16_t)White
    }
  },//titleColor
};


const colorDef<uint16_t> nightColors[] MEMMODE={
  {
    {
      (uint16_t)Black,
      (uint16_t)Black
    },
    {
      (uint16_t)Black,
      (uint16_t)DarkerBlue,
      (uint16_t)DarkerBlue
    }
  },//bgColor
  {
    {
      (uint16_t)Gray,
      (uint16_t)Gray
    },
    {
      (uint16_t)Red,//White,
      (uint16_t)Green,//White,
      (uint16_t)Red//White
    }
  },//fgColor
  {
    {
      (uint16_t)White,
      (uint16_t)Black
    },
    {
      (uint16_t)Yellow,
      (uint16_t)Yellow,
      (uint16_t)Green//Red
    }
  },//valColor
  {
    {
      (uint16_t)White,
      (uint16_t)Black
    },
    {
      (uint16_t)White,
      (uint16_t)Yellow,
      (uint16_t)Yellow
    }
  },//unitColor
  {
    {
      (uint16_t)White,
      (uint16_t)Gray
    },
    {
      (uint16_t)Black,
      (uint16_t)Blue,
      (uint16_t)White
    }
  },//cursorColor
  {
    {
      (uint16_t)White,
      (uint16_t)Yellow
    },
    {
      (uint16_t)DarkerRed,
      (uint16_t)White,
      (uint16_t)White
    }
  },//titleColor
};
////////////////////////////////////////////////////////////
using namespace Menu;
TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);
Button2 btnUp(BTN_UP); // Initialize the up button
Button2 btnDwn(BTN_DWN); // Initialize the down button
result doAlert(eventMask e, prompt &item);
//int ledBacklight = 80;
char buff[128];
int vref = 1100;
const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;
//int ledCtrl=LOW; // Initial value for external connected ledCtrl
//int test=55;
EEPROM_Config MyConfig;
byte myUsbSerialBaudrateIndex = 0;
TelescopeSensorMount CurrentSensorMountConfig;
bool nightMode = 0;
int AutoPowerOffTimer = 0;
char* constMEM onlySign MEMMODE="+-";
char* constMEM onlyIntegers MEMMODE="0123456789";
char* constMEM onlyIntegersKeys[] MEMMODE={onlySign,onlyIntegers,onlyIntegers,onlyIntegers,onlyIntegers,onlyIntegers};
char* constMEM qwerty MEMMODE="+-_0123456789ABCDEFGHIJKLMNÑOPQRSTUVWXYZabcdefghijklmnñopqrstuvwxyz";
char* constMEM qwertyKeys[] MEMMODE={qwerty,qwerty,qwerty,qwerty,qwerty,qwerty,qwerty,qwerty};
char RAAzBuf[8] = "+10000";
char DecAltBuf[8] = "+10000";
bool isIdle = 0;
bool myUpButtEvent = 0;
bool myDwnButtEvent = 0;
byte myConnectEvent = 0;
bool myBtEnable = 0;
bool myWifiEnable = 0;
volatile int myIdleEventsFifoBuffer[IDEL_EVENT_FIFO_SIZE] = {0,0,0,0,0,0,0,0};
int myIdleEventsFifoReadIndex = 0;
volatile int myIdleEventsFifoWriteIndex = 0;
////////////////////////////////////////////////////////////
void loadEEPROM();
void uploadEEPROM();
void loadDefEEPROM();
bool checkVersionEEPROM();
void validateEEPROM();

// myIdleDraw1:
void prepareDrawData1();
void drawData1();
void restoreDrawData1();

// myIdleDraw2:
void prepareDrawData2();
void drawData2();
void restoreDrawData2();

// myIdleDraw3:
void prepareDrawData3();
void drawData3();
void restoreDrawData3();

void updateEncoders();
void myIdleLoop();
result Action_NightMode_On();
result Action_NightMode_Off();

// Night mode //////////////////////////////////////////////
TOGGLE(nightMode, setNightMode,"Night mode: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("On",HIGH,Action_NightMode_On,enterEvent)
  ,VALUE("Off",LOW,Action_NightMode_Off,enterEvent)
);
// AutopowerOff //////////////////////////////////////////////
result Action_AutoPowerOff_On() {
  #ifdef USE_DEBUG
  Serial.printf("\n\nAutoPowerOff On: %d sec\n", MyConfig.AutoPowerOff);
  #endif
  #ifndef USE_SAVE_AT_EXIT_MENU
  uploadEEPROM();
  #endif
  return proceed;
}
result Action_AutoPowerOff_Off() {
  #ifdef USE_DEBUG
  Serial.printf("\n\nAutoPwrOff Off: %d\n", MyConfig.AutoPowerOff);
  #endif
  #ifndef USE_SAVE_AT_EXIT_MENU
  uploadEEPROM();
  #endif
  return proceed;
}
TOGGLE(MyConfig.AutoPowerOff,setAutoPowerOff,"AutoPowerOff: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("Off",0, Action_AutoPowerOff_Off, enterEvent)
  ,VALUE("10sec", 10, Action_AutoPowerOff_On, enterEvent)
  ,VALUE("30sec", 30, Action_AutoPowerOff_On, enterEvent)
  ,VALUE("1 min", 60, Action_AutoPowerOff_On, enterEvent)
  ,VALUE("5 min", 300, Action_AutoPowerOff_On, enterEvent)
  ,VALUE("10min", 600, Action_AutoPowerOff_On, enterEvent)
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
  strncpy(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Name, CurrentSensorMountConfig.Name, BT_NAME_SIZE);  
  MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_Res = atol(RAAzBuf);
  MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_Res = atol(DecAltBuf);

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
TOGGLE(myBtEnable,chooseBluetoothMenu,"Bluetooth:",doNothing,noEvent,noStyle
  ,VALUE("ON",1,doNothing,noEvent)
  ,VALUE("OFF",0,doNothing,noEvent)
);
TOGGLE(myWifiEnable,chooseWifiMenu,"WiFi:",doNothing,noEvent,noStyle
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


// Restore //////////////////////////////////////////////////
result Action_Restore()
{
  #ifdef USE_DEBUG
  Serial.printf("\n\nAction Restore..."); 
  #endif
  loadDefEEPROM();
  uploadEEPROM();
  updateEncoders();
  #ifdef USE_DEBUG
  Serial.printf("done\n");
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
  ,SUBMENU(chooseWifiMenu)
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

////////////////////////////////////////////////////////////
serialIn serial(Serial);
//MENU_INPUTS(in,&serial);its single, no need to `chainStream`
//define serial output device
idx_t serialTops[MAX_DEPTH]={0};
serialOut outSerial(Serial,serialTops);
constMEM panel panels[] MEMMODE = {{0, 0, GFX_WIDTH / fontW, GFX_HEIGHT / fontH}};
navNode* nodes[sizeof(panels) / sizeof(panel)]; //navNodes to store navigation status
panelsList pList(panels, nodes, 1); //a list of panels and nodes
idx_t eSpiTops[MAX_DEPTH]={0};
TFT_eSPIOut eSpiOut(tft,colors,eSpiTops,pList,fontW,fontH+1);
//TFT_eSPIOut eSpiOut(tft,nightColors,eSpiTops,pList,fontW,fontH+1);
menuOut* constMEM outputs[] MEMMODE={&outSerial,&eSpiOut};//list of output devices
outputsList out(outputs,sizeof(outputs)/sizeof(menuOut*));//outputs list controller
NAVROOT(nav,mainMenu,MAX_DEPTH,serial,out);
////////////////////////////////////////////////////////////
result Action_NightMode_On() {
  #ifdef USE_DEBUG
  Serial.printf("\n\nNightMode On\n");
  #endif
  return proceed;
}
result Action_NightMode_Off() {
  #ifdef USE_DEBUG
  Serial.printf("\n\nNightMode Off\n");
  #endif
  return proceed;
}
////////////////////////////////////////////////////////////
//when menu is suspended
result idle(menuOut& o,idleEvent e) {//idleStart,idling,idleEnd

  //static int lastEvent = -1;

  //if(e == lastEvent) return proceed; //Avoid repetitions
  //else lastEvent = e;
  
  //o.clear();
  switch(e) {
    case idleStart:    
    //o.println("\nsuspending menu!\n");   
    //o.clear();    
    myIdleEventsFifoBuffer[myIdleEventsFifoWriteIndex] = e;
    myIdleEventsFifoWriteIndex = (myIdleEventsFifoWriteIndex+1)&IDEL_EVENT_FIFO_MASK;
    break;
    
    case idling:
    //o.println("\nsuspended...\n");
    isIdle = 1;
    //o.clear();   
    myIdleEventsFifoBuffer[myIdleEventsFifoWriteIndex] = e;
    myIdleEventsFifoWriteIndex = (myIdleEventsFifoWriteIndex+1)&IDEL_EVENT_FIFO_MASK;
    break;
    
    case idleEnd:
    //o.println("\nresuming menu.\n");    
    isIdle = 0;
    //o.clear();
    myIdleEventsFifoBuffer[myIdleEventsFifoWriteIndex] = e;
    myIdleEventsFifoWriteIndex = (myIdleEventsFifoWriteIndex+1)&IDEL_EVENT_FIFO_MASK;
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

//config myOptions('*','-',defaultNavCodes,true);

void button_init()
{
    btnUp.setLongClickHandler([](Button2 & b) {
        // Select
        unsigned int time = b.wasPressedFor();
        myUpButtEvent = 2;        
        if (time >= 1000) {
          if(AutoPowerOffTimer || MyConfig.AutoPowerOff == 0) nav.doNav(enterCmd);
        }
        AutoPowerOffTimer = MyConfig.AutoPowerOff;
    });
    
    btnUp.setClickHandler([](Button2 & b) {        
       // Up
       if(AutoPowerOffTimer || MyConfig.AutoPowerOff == 0) nav.doNav(downCmd); // It's called downCmd because it decreases the index of an array. Visually that would mean the selector goes upwards.
       myUpButtEvent = 1;
       AutoPowerOffTimer = MyConfig.AutoPowerOff;
    });

    btnDwn.setLongClickHandler([](Button2 & b) {
        // Exit
        unsigned int time = b.wasPressedFor();
        
        if (time >= 1000) {
          if(AutoPowerOffTimer || MyConfig.AutoPowerOff == 0) nav.doNav(escCmd);
        }
        myDwnButtEvent = 2;
        AutoPowerOffTimer = MyConfig.AutoPowerOff;
    });
    
    btnDwn.setClickHandler([](Button2 & b) {
        myDwnButtEvent = 1;
        // Down
        if(AutoPowerOffTimer || MyConfig.AutoPowerOff == 0) nav.doNav(upCmd); // It's called upCmd because it increases the index of an array. Visually that would mean the selector goes downwards.
        AutoPowerOffTimer = MyConfig.AutoPowerOff;
    });
}

void button_loop()
{
    // Check for button presses
    btnUp.loop();
    btnDwn.loop();
}

//! Long time delay, it is recommended to use shallow sleep, which can effectively reduce the current consumption
void espDelay(int ms)
{
    esp_sleep_enable_timer_wakeup(ms * 1000);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    esp_light_sleep_start();
}

void loadEEPROM()
{
  byte* p;
  #ifdef USE_DEBUG
  Serial.print("Loading EEPROM...\n");
  #endif
  p = (byte*)&MyConfig;
  for(int i = 0; i < EEPROM_SIZE; i++) p[i] = EEPROM.read(i);
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
}
void loadDefEEPROM()
{
  #ifdef USE_DEBUG
  Serial.print("Loading default config...\n");
  #endif
  MyConfig.Version = EEPROM_VERSION;
  MyConfig.CurrentMount = DEF_CURRENT_MOUNT;
  MyConfig.Wifi = DEF_WIFI;
  MyConfig.Bluetooth = DEF_BT;
  MyConfig.USB = DEF_USB_SERIAL_BAUDRATE;  

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

void AutoPowerOffLoop()
{
  static time_t timer = 0;
  static int LastAutoPowerOffCtrl = -1;
  static bool forceOff = 0;

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
    if(myUpButtEvent == 1)
    {
      myUpButtEvent = 0;
      forceOff = !forceOff;      
    }
  }
  else
  {
    forceOff = 0;
    myUpButtEvent = 0;
    myDwnButtEvent = 0;
  }
  
  if(AutoPowerOffTimer && !forceOff) ledcWrite(pwmLedChannelTFT, (int)MyConfig.Backlight*255/100);
  else ledcWrite(pwmLedChannelTFT, 0);
}

void updateEncoders()
{
  dsc_SetAltSensor(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_SensorType);
  dsc_SetAzSensor(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_SensorType);
  dsc_SetAltRes(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_Res);
  dsc_SetAzRes(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_Res);  
  dsc_SetAlt(abs(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_Res)/2);
  dsc_SetAz(abs(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_Res)/2);
}

  
void myConnectedLoop()
{
  static int lastConnectedEvent = 0;
  int newConnectedEvent = 0;
  static time_t timer = 0;
  
  if(myUsbSerialBaudrateIndex != MyConfig.USB)
  {
    MyConfig.USB = myUsbSerialBaudrateIndex;
    uploadEEPROM();
    connectUsbSerial(myUsbSerialBaudrateIndex);
  }
  
  if(myBtEnable && !MyConfig.Bluetooth)
  {
    MyConfig.Bluetooth = 1;
    uploadEEPROM();
    connectBluetooth();
  }
  if(!myBtEnable && MyConfig.Bluetooth)
  {
    MyConfig.Bluetooth = 0;
    uploadEEPROM();
    disconnectBluetooth();
  }

  if(myWifiEnable && !MyConfig.Wifi)
  {
    MyConfig.Wifi = 1;
    uploadEEPROM();
    connectWIFI();
  }
  if(!myWifiEnable && MyConfig.Wifi)
  {
    MyConfig.Wifi = 0;
    uploadEEPROM();
    disconnectWIFI();
  }

  //Check connection
  newConnectedEvent = isBluetoothSelected();
  newConnectedEvent = newConnectedEvent + (int)isBtConnected() * 2;
  newConnectedEvent = newConnectedEvent + (int)isWifiSelected() * 4;
  newConnectedEvent = newConnectedEvent + (int)isWifiConnected() * 8;

  myConnectEvent = 0;
  if(newConnectedEvent != lastConnectedEvent)
  {
    lastConnectedEvent = newConnectedEvent;
    myConnectEvent = 1;
  }

  //Enable/disable menus
  if(!isBtConnected() && !isWifiConnected())
  {    
    //Disconnected
    mainMenu[0].enable();
    mainMenu[1].enable();
    mainMenu[2].enable();    
    mainMenu[3].enable(); 
    #ifdef USE_DEBUG
    mainMenu[11].enable();
    #else
    mainMenu[12].enable();
    #endif
  }
  if(isBtConnected() || isWifiConnected())
  {
    //Connected
    mainMenu[0].disable();
    mainMenu[1].disable();
    mainMenu[2].disable();
    mainMenu[3].disable();
    #ifdef USE_DEBUG
    mainMenu[11].disable();
    #else
    mainMenu[12].enable();
    #endif
  }
}

void setup()
{
  extern unsigned long int BAUDRATES[];
  char buf[48];
  pinMode(ADC_EN, OUTPUT);
  //digitalWrite(ADC_EN, HIGH);
  digitalWrite(ADC_EN, LOW);
  EEPROM.begin(EEPROM_SIZE);   

/*
  if (TFT_BL > 0) { // TFT_BL has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
      pinMode(TFT_BL, OUTPUT); // Set backlight pin to output mode
      digitalWrite(TFT_BL, TFT_BACKLIGHT_ON); // Turn backlight on. TFT_BACKLIGHT_ON has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
  }
*/
  #ifdef USE_DEBUG
  Serial.print("Configuring PWM for TFT backlight...\n");
  #endif
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
  delay(500);
  tft.setTextColor(TFT_RED);    
  ledcWrite(pwmLedChannelTFT, 100);
  if(checkVersionEEPROM())
  {
    #ifdef USE_DEBUG
    Serial.printf("Config: %d\n", EEPROM_VERSION);
    #endif    
    loadEEPROM();
    tft.drawString("EEPROM loaded", 5, 24*0);
  }
  else
  {
    #ifdef USE_DEBUG
    Serial.printf("New Config to %d\n", EEPROM_VERSION);
    #endif
    loadDefEEPROM();
    tft.drawString("Def EEPROM loaded", 5, 24*0);
  }
  uploadEEPROM();
  delay(500);
  
  dsc_SetAltSensor(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_SensorType);
  dsc_SetAzSensor(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_SensorType);
  
  dsc_SetAltRes(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_Res);
  dsc_SetAzRes(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_Res);  
  
  dsc_SetAlt(abs(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_Res)/2);
  dsc_SetAz(abs(MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_Res)/2);

  sprintf(buf, "Mount %d: %s", MyConfig.CurrentMount%MAX_TELESCOPES, MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Name);
  tft.drawString(buf, 5, 24*1);
  delay(500);
  sprintf(buf, "Az Res: %ld", MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].RA_Az_Res);
  tft.drawString(buf, 5, 24*2);
  delay(500);
  sprintf(buf, "Alt Res: %ld", MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Dec_Alt_Res);
  tft.drawString(buf, 5, 24*3);
  delay(500);
  
  dsc_Init();
  dsc_Enable();

  delay(2000);
  tft.fillScreen(TFT_BLACK);
  #ifdef USE_DEBUG
  tft.drawString("DEBUG ON", 5, 24*0);
  #else
  tft.drawString("DEBUG OFF", 5, 24*0);
  #endif
  delay(500);

  tft.drawString("Opening Serial...", 5, 24*1);
  myUsbSerialBaudrateIndex = MyConfig.USB;
  #ifdef USE_DEBUG
  Serial.begin(BAUDRATE_DEBUG);
  #else
  connectUsbSerial(MyConfig.USB);
  #endif
  sprintf(buf, "done: %d", BAUDRATES[myUsbSerialBaudrateIndex]);
  tft.drawString(buf, 5, 24*2);
  delay(500);
  
  // Bluetooth /////////////
  if(MyConfig.Bluetooth)
  {
    tft.drawString("Connect BT...", 5, 24*3);
    connectBluetooth();
    myBtEnable = 1;
    tft.drawString("done", 239-24*2, 24*3);
  }
  else
  {    
    //disconnectBluetooth();
    myBtEnable = 0;
    tft.drawString("Disconnect BT", 5, 24*3);
  }  
  delay(500);

  // WiFi ////////////////
  if(MyConfig.Wifi)
  {
    tft.drawString("Connect WiFi...", 5, 24*4);
    connectWIFI();
    myWifiEnable = 1;
    tft.drawString("done", 239-24*2, 24*4);
  }
  else
  {
    //disconnectWIFI();
    myWifiEnable = 0;
    tft.drawString("Disconnect WiFi", 5, 24*4);
  }
  delay(1000);
  tft.fillScreen(TFT_BLACK);
  tft.drawString("Wellcome!", 5, 31*0);
  tft.drawString("openDSC", 5, 31*1);
  tft.drawString(ESP_DSC_VERSION, 5, 31*2);  
  tft.drawString("by xvedra, 2020", 5, 31*3);
  delay(2000);
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
  delay(10);
  ledcWrite(pwmLedChannelTFT, (int)MyConfig.Backlight*255/100);
  //espDelay(1000);    
  delay(2000);
  ledcWrite(pwmLedChannelTFT, 0);
  tft.fillScreen(TFT_BLACK);
  delay(100);
  ledcWrite(pwmLedChannelTFT, (int)MyConfig.Backlight*255/100);  
  tft.fillScreen(Black);
  nav.idleTask=idle;//point a function to be used when menu is suspended
  nav.idleOn(idle);
  isIdle = 1;
}

void loop()
{
  
  nav.poll();//this device only draws when needed
  commandsLoop();
  AutoPowerOffLoop();
  button_loop();  
  myConnectedLoop();
  myIdleLoop();  
}

static int lastAlt = -1;
static int lastAz = -1;
static float lastV = -1.0;
static int lastCap = -1;
static time_t TimerDrawData = 0;

void prepareHead()
{       
    lastCap = -1;
    tft.setCursor(0, 0);
    tft.setTextDatum(MC_DATUM);    
    tft.setTextColor(DRAW_TEXT_COLOR2);
    tft.setTextSize(3);
    sprintf(buff, "%s", MyConfig.Tele[MyConfig.CurrentMount%MAX_TELESCOPES].Name);
    tft.drawString(buff, 0, 15);

    if(isBtConnected()) tft.pushImage(239-24-32-5*2, 0,  27, 32, &BTonoff[_BTonoff_on]); 
    else tft.pushImage(239-24-32-5*2, 0,  27, 32, &BTonoff[_BTonoff_off]);

    if(isWifiConnected()) tft.pushImage(239-27-5, 0,  32, 32, &WIFIonoff[_WIFIonoff_on]); 
    else tft.pushImage(239-27-5, 0,  32, 32, &WIFIonoff[_WIFIonoff_off]);
}

void drawHead()
{
  if(myConnectEvent != 0)
  {
    if(isBtConnected()) tft.pushImage(239-24-32-5*2, 0,  27, 32, &BTonoff[_BTonoff_on]); 
    else tft.pushImage(239-24-32-5*2, 0,  27, 32, &BTonoff[_BTonoff_off]);

    if(isWifiConnected()) tft.pushImage(239-27-5, 0,  32, 32, &WIFIonoff[_WIFIonoff_on]); 
    else tft.pushImage(239-27-5, 0,  32, 32, &WIFIonoff[_WIFIonoff_off]);
  }

  if((millis() % 500) == 0) //1Hz
  {
    uint16_t v = analogRead(ADC_PIN);
    float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    int output = 8 * ((battery_voltage - battery_min) / (battery_max - battery_min));
    if(output > 8) output = 8;
    if(output < 0) output = 0;
    
    if(output != lastCap)
    {
      lastCap = output;      

      #ifdef _VBatt
      if(output < 2 )    tft.pushImage(239-22-16-32-5*4, 0,  16, 32, &VBatt[_VBatt_VERYLOW]);    
      else if(output < 4)tft.pushImage(239-22-16-32-5*4, 0,  16, 32, &VBatt[_VBatt_LOW]); 
      else if(output < 6)tft.pushImage(239-22-16-32-5*4, 0,  16, 32, &VBatt[_VBatt_MIDDLE]); 
      else          tft.pushImage(239-22-16-32-5*4, 0,  16, 32, &VBatt[_VBatt_HIGH]); 
      #else
  
      #endif
    }
  }
}

void prepareDrawData1()
{
    #ifdef USE_DEBUG
    Serial.println("prepareDrawData...\n");
    #endif
    lastAlt = -1;
    lastAz = -1;
    lastV = -1.0;    
    TimerDrawData = InitTimer(1000);
    tft.fillScreen(TFT_BLACK);        
    tft.setCursor(0, 0);
    tft.setTextDatum(MC_DATUM);    
    tft.setTextColor(DRAW_TEXT_COLOR2);
    tft.setTextSize(3);

    prepareHead();

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
    tft.setTextColor(DRAW_TEXT_COLOR1);
    tft.drawString("RA/AZ:", 5, 25+32);
    tft.drawString("DEC/ALT:", 5, 25+32*2);
    tft.drawString("BAT:", 5, 25+32*3); 
    tft.setTextSize(3);
    
}
void restoreDrawData1()
{
  tft.setTextSize(2);
}


void drawData1()
{  
  static signed int data[GRAPH_MAX_POINTS];
  int i;
  static int index = 0;
  static unsigned long int LastGetPorReq = 0;
  static unsigned int currentTextColor = 0;   
  
  if(TimerDrawData == 0)
  {
    for(i = 0; i < GRAPH_MAX_POINTS; i++) data[i] = 0;
    index = 0;
    TimerDrawData = InitTimer(0);
  }

  if(myDwnButtEvent)
  {
    
    //here user code:
  }

  drawHead();

  if(checkTimer(TimerDrawData, 1000)) TimerDrawData = InitTimer(0);
  else return;

  //Request rate:
  data[index] = ReadGetPosReqCtr() - LastGetPorReq;
  if(data[index] < 0) data[index] = 0;
    
  
  // Zone A
  if(dsc_GetAz() != lastAz)
  {
    lastAz = dsc_GetAz();
    tft.setTextColor(DRAW_TEXT_COLOR2, TFT_BLACK);
    sprintf(buff, "%05ld", abs(dsc_GetAz()));    
    //tft.fillRect(100, 10+32-2, 105, 21, TFT_BLACK); //fillRect
    tft.drawString(buff, 160, 20 + 32 * 1);
  }

  // Zone B
  if(dsc_GetAlt() != lastAlt)
  {
    lastAlt = dsc_GetAlt();    
    tft.setTextColor(DRAW_TEXT_COLOR2, TFT_BLACK);
    sprintf(buff, "%05ld", abs(dsc_GetAlt()));
    //tft.fillRect(100, 10+32*2-2, 105, 21, TFT_BLACK);
    tft.drawString(buff, 160, 20 + 32 * 2);  
  }

  // Zone C
  uint16_t v = analogRead(ADC_PIN);
  float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
  if(battery_voltage != lastV)
  {
    if(battery_voltage < battery_alert)
    {
      //1Hz:
      if(millis() & 0x400) tft.setTextColor(DRAW_TEXT_COLOR4);
      else tft.setTextColor(DRAW_TEXT_COLOR3);
    }
    else tft.setTextColor(DRAW_TEXT_COLOR3, TFT_BLACK);
    sprintf(buff, "%01.2fV", battery_voltage);
    tft.drawString(buff, 160, 20 + 32 * 3);  
  }

/*
  // Zone E: Graph
  tft.fillRect(0, 134-50, 240, 50, TFT_BLACK);
  //tft.drawRect(0, 134-50, 240, 50, TFT_RED);//debug
  if(myEvent != 0)
  {
    tft.setTextColor(DRAW_TEXT_COLOR2);
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
      tft.drawRect(i*4, 100 + (32 - val), 2, val, DRAW_GRAPH_COLOR);
    }
  }  
*/ 

  index--;
  if(index < 0) index = GRAPH_MAX_POINTS - 1;  
  LastGetPorReq = ReadGetPosReqCtr();  
}

void prepareDrawData2()
{
  
}
void drawData2()
{
  
}
void restoreDrawData2()
{
  
}

void prepareDrawData3()
{
  
}
void drawData3()
{
  
}
void restoreDrawData3()
{
  
}

void myIdleLoop()
{
  static int stt = 0;
  static int currentEvent = -1;

  if(myIdleEventsFifoWriteIndex != myIdleEventsFifoReadIndex)
  {
    currentEvent = myIdleEventsFifoBuffer[myIdleEventsFifoReadIndex];
    #ifdef USE_DEBUG
    //Serial.printf("myIdleLoop: e=%d at %d\n", currentEvent, myIdleEventsFifoReadIndex);
    #endif
    myIdleEventsFifoReadIndex = (myIdleEventsFifoReadIndex+1)&IDEL_EVENT_FIFO_MASK;    
  }
  
  switch(stt)
  {
    case 0:
    // Prepare DrawData1
    if(currentEvent == idleStart)
    {
      #ifdef USE_SAVE_AT_EXIT_MENU    
      uploadEEPROM(); //First exit from menu: save!
      #endif
      prepareDrawData1();    
      stt++;
    }
    if(currentEvent == idling) stt++;
    break;
    case 1:
    // DrawData1
    if(currentEvent == idling)
    {
      drawData1();
    }
    else if(currentEvent == idleEnd)
    {
      stt = 0;
      restoreDrawData1();
    }
    break;
    case 2:
    // Prepare DrawData2
    break;
    case 3:
    // DrawData2
    break;
    case 4:
    // Prepare DrawData3
    break;
    case 5:
    // DrawData3
    break;
  }

  myDwnButtEvent = 0;
}

