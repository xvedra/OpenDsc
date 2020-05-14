//#include <Arduino.h>
#include <BluetoothSerial.h>  
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <esp_bt_main.h>

#include "dsc.h"
#include "def.h"
#include "config.h"
#include "utils.h"


// Defines ///////////////////////////////////////
//#define pinLED1         17
// Prototypes ////////////////////////////////////
void commandsInit(void);
void commandsLoop(void);
void BtProcessCommands();
void WifiProcessClient(uint8_t c);
void readLine(char *p, int maxlen);
void printEncoderValue(long val);
void printHexEncoderValue(long val);
void parseSetResolutionCmd(char *cmd);
void parseEkSetResolutionCmd();
unsigned long int ReadGetPosReqCtr();
bool isBtConnected();
void GetAutomatically();
void connectBluetooth();
void disconnectBluetooth();
bool isBluetoothSelected();
void connectWIFI();
void disconnectWIFI();
bool isWifiSelected();
bool isWifiConnected();
void WIFI_loop();

