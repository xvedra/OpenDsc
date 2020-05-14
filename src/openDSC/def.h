#include "config.h"

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

#define ESP_DSC_VERSION "1.0.0"
#define ESP_DSC_ABOUT "V:1.0.0 by Xvedra"
#define ESP_DSC_BT_NAME "Bluetooth: openDSC"
#define ESP_DSC_WIFI_NAME "WIFI: openDSC"
#define ESP_DSC_WIFI_PASS " pass: 12345678"
#define ESP_DSC_WIFI_IP " IP: 10.0.0.1:4030"



#define WIFI_LOOP_T_MS 25
#define BUFF_LEN 10
#define CLIENT_BUFF_LEN 36
