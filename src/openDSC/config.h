// Use:
// Normally defined:
#define USE_SOLVE_SKYSAFARI_AUTOMATICALLY_RESOLUTION_BUG
#define USE_SAVE_AT_EXIT_MENU //Upload EEPRON only whet exit from MainMenu

// Normally undefined:
//#define USE_L200 /* only for awner development propossal */
//#define USE_DEBUG

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

// I2C //////////////////////////////////////////
//AZ pinout:
#define SCL1_PIN  13
#define SDA1_PIN  15
//Alt pinout:
#define SCL2_PIN  32
#define SDA2_PIN  33
