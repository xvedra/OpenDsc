#include <Arduino.h>
#include "def.h"
#include "config.h"
#include "utils.h"

// Defines ///////////////////////////////////////
//#define ENC_ALT_DEF RES_ALT_DEF  // resolution of encoders
//#define ENC_AZ_DEF  RES_AZ_DEF

#define DSC_TIME_LOOP_MS  20

#define DSC_QUADRATURE 0
#define DSC_AS5600     1
#define DSC_AS5048     2
#define DSC_MPU6050    3

void dsc_Init(void);
void dsc_Loop(void);

void dsc_GetAltAz(long *alt, long *az);
long dsc_GetAlt(void);
long dsc_GetAz(void);

void dsc_SetAltAzRes(long alt, long az);
void dsc_SetAltRes(long alt);
void dsc_SetAzRes(long az);

void dsc_GetAltAzRes(long *alt, long *az);
long dsc_GetAltRes(void);
long dsc_GetAzRes(void);

void dsc_SetAlt(long count);
void dsc_SetAz(long count);

void dsc_Disable();
void dsc_Enable();
bool dsc_IsEnabled();

void dsc_SetAltSensor(int sensor);
void dsc_SetAzSensor(int sensor);

int dsc_GetAltSensor();
int dsc_GetAzSensor();

long int formatData(long int in, long int res);

