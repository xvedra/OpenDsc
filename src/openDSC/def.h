#include "config.h"

#ifndef _DEF_H
#define _DEF_H

#define MAX_SRV_CLIENTS 4
#define MAX_SRV_CLIENTS 4
#define MAX_AP_RETRIES 60
#define SERIAL_SPEED 115200
#define SERIAL_PRINT_DELAY 1000
#define BUFF_LEN 10
#define CLIENT_BUFF_LEN 36

#define RES_ALT_DEF 28672 //4096 * 182 / 26 /* Use negative to reverse */
#define RES_AZ_DEF -28672 //4096 * 182 / 26 /* Use negative to reverse */
#define DEF_MAX_RES 65535
#define DEF_MIN_RES -65535
#define AZ_FILTER_LEVEL 10
#define ALT_FILTER_LEVEL 10



#define ESP_DSC_VERSION "2.0.2"
#define ESP_DSC_ABOUT "V2.0.2 by Xvedra"
#define ESP_DSC_BT_NAME "Bluetooth: openDSC"
#define ESP_DSC_WIFI_NAME "WIFI: openDSC"
#define ESP_DSC_WIFI_PASS " pass: 12345678"
#define ESP_DSC_WIFI_IP " IP: 10.0.0.1:4030"

#define DEF_BR_1200 0
#define DEF_BR_2400 1
#define DEF_BR_4800 2
#define DEF_BR_9600 3
#define DEF_BR_19200  4
#define DEF_BR_38400  5
#define DEF_BR_57600  6
#define DEF_BR_115200 7


#define WIFI_LOOP_T_MS 25
#define BUFF_LEN 10
#define CLIENT_BUFF_LEN 36


#define MAX_TELESCOPES 4
#define BT_NAME_SIZE 8
#define EEPROM_ADD 0
#define DEF_BACKLIGHT 80
#define DEF_AUTOPOWEROFF 0

#define EEPROM_SIZE (sizeof(MyConfig))
#define MAX_RES DEF_MAX_RES
#define MIN_RES DEF_MIN_RES
#define DEF_AZ_RES RES_AZ_DEF
#define DEF_ALT_RES RES_ALT_DEF


#define DEF_WIFI 0
#define DEF_BT 1

#define DEF_WIFI_TX_POWER WIFI_TX_PWR_LOW

#define SECTION_NULL  0
#define SECTION_START 1
#define SECTION_RUN   2
#define SECTION_IDLE  3
#define SECTION_END   4
#define SECTION_ERROR -1

// Sensor Types ///////////////////////////
#define ST_QUADRATURE 0 //Not supported by TTGO. Under development
#define ST_AS5600     1
#define ST_AS5048     2
#define ST_MPU6050    3
#define ST_MAX_SENSOR ST_MPU6050

// TelescopeSensorMount //////////////////////
#define EEPROM_VERSION 2

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
  byte WiFi;
  byte WiFiTxPower;
  byte Bluetooth;
  byte USB;
  byte CurrentMount;
  TelescopeSensorMount Tele[MAX_TELESCOPES];
  byte Backlight;
  int AutoPowerOff;
} EEPROM_Config;

// Wireless statur register
typedef struct
{
  union
  {
    byte reg;  
    struct
    {
      byte BT_enabled : 1;
      byte BT_connected : 1;
      byte BT_fail : 1;
      byte WiFi_enabled : 1;
      byte WiFi_connected : 1;
      byte WiFi_fail : 1;
    }str;
  };
  byte BT_switched; //non bit to use in menu
  byte WiFi_switched; //non bit to use in menu
} WirelessStatus;

//WIFI TX PWR:
#define WIFI_TX_PWR_VERYHIGH  78
#define WIFI_TX_PWR_HIGH  66
#define WIFI_TX_PWR_MIDDLE  52
#define WIFI_TX_PWR_LOW 28
#define WIFI_TX_PWR_VERYLOW 8

#endif
