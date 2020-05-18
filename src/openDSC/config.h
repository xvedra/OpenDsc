// Use:
// Normally defined:
#define USE_SOLVE_SKYSAFARI_AUTOMATICALLY_RESOLUTION_BUG
#define USE_SAVE_AT_EXIT_MENU //Upload EEPRON only whet exit from MainMenu

// Normally undefined:
//#define USE_L200 /* only for awner development propossal */
//#define USE_DEBUG //Comment to enable communications over USB
#define SLOW_BOOT 1 //1:slow 0:fast
// USB Serial ///////////////////////////////////
#define BAUDRATE_DEBUG_INDEX 7

// Bluetooth ////////////////////////////////////
#define BT_NAME "openDSC"

// WiFi ////////////////////////////////////////
#define AP_MODE
#define TCP_PORT 4030
#ifdef AP_MODE
  // Set these values if you wish to create a new WiFi network
  #define WIFI_SSID "openDSC"
  #define WIFI_PASSWORD "12345678" // change this to something more secure
#else
  // Set these values if you wish to connect to an existing WiFi network
  #define WIFI_SSID "MyWiFiNetwork"
  #define WIFI_PASSWORD "*********"
  #error "Not developed yet... please wait. Thank you"
#endif

#define DEF_USB_SERIAL_BAUDRATE DEF_BR_115200
#define DEF_CURRENT_MOUNT 0
//Default setting for Mount1:
#define DEF_NAME1 "GSO 10"
#define DEF_ALT_ENC1 ST_AS5600
#define DEF_AZ_ENC1 ST_AS5600
#define DEF_ALT_RES1 28672 //4096 * 182 / 26 /* Use negative to reverse */
#define DEF_AZ_RES1 -28672 //4096 * 182 / 26 /* Use negative to reverse */
//Default setting for Mount2:
#define DEF_NAME2 "ORION 10"
#define DEF_ALT_ENC2 ST_AS5600
#define DEF_AZ_ENC2 ST_AS5600
#define DEF_ALT_RES2 -14336 //4096 * 182 / 26 /* Use negative to reverse */
#define DEF_AZ_RES2 -14336 //4096 * 182 / 26 /* Use negative to reverse */
//Default setting for Mount3:
#define DEF_NAME3 "GSO 8"
#define DEF_ALT_ENC3 ST_AS5600
#define DEF_AZ_ENC3 ST_AS5600
#define DEF_ALT_RES3 4096 //4096 * 182 / 26 /* Use negative to reverse */
#define DEF_AZ_RES3 4096 //4096 * 182 / 26 /* Use negative to reverse */
//Default setting for Mount4:
#define DEF_NAME4 "ORION 8"
#define DEF_ALT_ENC4 ST_AS5600
#define DEF_AZ_ENC4 ST_AS5600
#define DEF_ALT_RES4 65535 //4096 * 182 / 26 /* Use negative to reverse */
#define DEF_AZ_RES4  65535 //4096 * 182 / 26 /* Use negative to reverse */
// I2C //////////////////////////////////////////
//AZ pinout:
#define SCL1_PIN  13
#define SDA1_PIN  15
//Alt pinout:
#define SCL2_PIN  32
#define SDA2_PIN  33
