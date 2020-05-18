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
void SerialProcessCommands(Stream *serial);
void WiFiProcessClient(uint8_t c);
void readLine(char *p, int maxlen);
void printEncoderValue(long val);
void printHexEncoderValue(long val);
void parseSetResolutionCmd(char *cmd);
void parseEkSetResolutionCmd();
unsigned long int ReadGetPosReqCtr();
void connectUsbSerial(byte index);
bool isBtConnected();
void GetAutomatically();
void connectBluetooth();
void disconnectBluetooth();
bool isBluetoothEnabled();
void connectWIFI();
void disconnectWIFI();
bool isWiFiEnabled();
bool isWiFiConnected();
void WIFI_loop();

