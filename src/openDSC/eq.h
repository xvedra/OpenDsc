
#include <Arduino.h>
#include "config.h"

//long pulses_enc1 = ALT_RESOLUTION; //Alt.               
//long pulses_enc2 = AZ_RESOLUTION; //Az.

#define pulses_enc1 abs(dsc_GetAltRes())
#define pulses_enc2  abs(dsc_GetAzRes())

// enter your latitude (example: North 40º33'20'')
int latHH = 28;    // this means 40º North
int latMM = 7;
int latSS = 7;

// enter Pole Star right ascention (AR: HH:MM:SS)
int poleAR_HH = 2;    // this means 2 hours, 52 minutes and 16 seconds
int poleAR_MM = 55;
int poleAR_SS = 41;

// enter Pole Star hour angle (H: HH:MM:SS)
int poleH_HH = 1;
int poleH_MM = 46;
int poleH_SS = 0;

int lastEncoded1 = 0;
long encoderValue1 = 0;
int lastEncoded2 = 0;
long encoderValue2 = 0;

unsigned long seg_sideral = 1003;
const double  pi = 3.14159265358979324;

char input[20];
char txAR[10] = "";
char txDEC[11] = "";
long TSL;
unsigned long t_ciclo_acumulado = 0, t_ciclo;
long Az_tel_s, Alt_tel_s;
long AR_tel_s, DEC_tel_s;
long AR_stell_s, DEC_stell_s;
double cos_phi, sin_phi;
double alt, azi;

void eqSetup(void);
void eqLoop(uint16_t AltEnc, uint16_t AzEnc);
void write_sensors(uint16_t enc1, uint16_t enc2);
void read_sensors();
char* get_Ar(void);
char* get_Dec(void);
void AZ_to_EQ();
long AltToSec(long enc);
long AzToSec(long enc);

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void eqSetup(void)
{
  cos_phi = cos((((latHH * 3600) + (latMM * 60) + latSS) / 3600.0) * pi / 180.0);
  sin_phi = sin((((latHH * 3600) + (latMM * 60) + latSS) / 3600.0) * pi / 180.0);
  
  TSL = poleAR_HH * 3600 + poleAR_MM * 60 + poleAR_SS + poleH_HH * 3600 + poleH_MM * 60 + poleH_SS;
  while (TSL >= 86400) TSL = TSL - 86400;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void eqLoop(uint16_t AltEnc, uint16_t AzEnc)
{
  encoderValue1 = AltEnc;
  encoderValue2 = AzEnc;
  
  t_ciclo = millis();
  if (t_ciclo_acumulado >= seg_sideral) {
    TSL++;
    t_ciclo_acumulado = t_ciclo_acumulado - seg_sideral;
    if (TSL >= 86400) {
      TSL = TSL - 86400;
    }
  }

  if((encoderValue1 != lastEncoded1) || (encoderValue2 != lastEncoded2))
  {
    lastEncoded1 = encoderValue1;
    lastEncoded2 = encoderValue2;

    read_sensors();
    AZ_to_EQ();
  }

  //if (Serial.available() > 0) communication();

  t_ciclo = millis() - t_ciclo;
  t_ciclo_acumulado = t_ciclo_acumulado + t_ciclo;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void write_sensors(uint16_t AltEnc, uint16_t AzEnc)
{
  encoderValue1 = AltEnc;
  encoderValue2 = AzEnc;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void read_sensors()
{
  long h_deg, h_min, h_seg, A_deg, A_min, A_seg;

  if (encoderValue2 >= pulses_enc2 || encoderValue2 <= -pulses_enc2) {
    encoderValue2 = 0;
  }
//  encoderValue1 = encoderValue1 % pulses_enc1;
//  encoderValue2 = encoderValue2 % pulses_enc2;

  //90º -> ArcSec
  int enc1 = encoderValue1 / 1500;
  long encoder1_temp = encoderValue1 - (enc1 * 1500);
  long map1 = enc1 * map(1500, 0, pulses_enc1, 0, 324000);

  //360º -> ArcSec
  int enc2 = encoderValue2 / 1500;
  long encoder2_temp = encoderValue2 - (enc2 * 1500);
  long map2 = enc2 * map(1500, 0, pulses_enc2, 0, 1296000);

  Alt_tel_s = map1 + map (encoder1_temp, 0, pulses_enc1, 0, 324000);
  Az_tel_s  = map2 + map (encoder2_temp, 0, pulses_enc2, 0, 1296000);

  if (Az_tel_s < 0) Az_tel_s = 1296000 + Az_tel_s;
  if (Az_tel_s >= 1296000) Az_tel_s = Az_tel_s - 1296000 ;
/*

  if(encoderValue1 >= pulses_enc1) encoderValue1 = encoderValue1 - pulses_enc1;
  if(encoderValue1 < 0) encoderValue1 = pulses_enc1 + encoderValue1;
  if(encoderValue1 < (pulses_enc1 / 2))
  {
    Alt_tel_s = (double)encoderValue1 * 324000 / pulses_enc1;
  }
  else
  {
    Alt_tel_s = 
  }
  
  if(encoderValue2 >= pulses_enc2) encoderValue2 = encoderValue2 - pulses_enc2;
  if(encoderValue2 < 0) encoderValue2 = pulses_enc2 + encoderValue2;
*/
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
char* get_Ar(void)
{
  return(txAR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
char* get_Dec(void)
{
  return(txDEC);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void AZ_to_EQ()
{
  double delta_tel, sin_h, cos_h, sin_A, cos_A, sin_DEC, cos_DEC;
  double H_telRAD, h_telRAD, A_telRAD;
  long H_tel;
  long arHH, arMM, arSS;
  long decDEG, decMM, decSS;
  char sDEC_tel;

  static long hhh = 0;

  A_telRAD = (Az_tel_s / 3600.0) * pi / 180.0;
  h_telRAD = (Alt_tel_s / 3600.0) * pi / 180.0;
  sin_h = sin(h_telRAD);
  cos_h = cos(h_telRAD);
  sin_A = sin(A_telRAD);
  cos_A = cos(A_telRAD);
  delta_tel = asin((sin_phi * sin_h) + (cos_phi * cos_h * cos_A));
  sin_DEC = sin(delta_tel);
  cos_DEC = cos(delta_tel);
  DEC_tel_s = long((delta_tel * 180.0 / pi) * 3600.0);

  while (DEC_tel_s >= 324000) {
    DEC_tel_s = DEC_tel_s - 324000;
  }
  while (DEC_tel_s <= -324000) {
    DEC_tel_s = DEC_tel_s + 324000;
  }

  H_telRAD = acos((sin_h - (sin_phi * sin_DEC)) / (cos_phi * cos_DEC));
  H_tel = long((H_telRAD * 180.0 / pi) * 240.0);

  if (sin_A >= 0) {
    H_tel = 86400 - H_tel;
  }
  AR_tel_s = TSL - H_tel;

  while (AR_tel_s >= 86400) {
    AR_tel_s = AR_tel_s - 86400;
  }
  while (AR_tel_s < 0) {
    AR_tel_s = AR_tel_s + 86400;
  }

  arHH = AR_tel_s / 3600;
  arMM = (AR_tel_s - arHH * 3600) / 60;
  arSS = (AR_tel_s - arHH * 3600) - arMM * 60;
  decDEG = abs(DEC_tel_s) / 3600;
  decMM = (abs(DEC_tel_s) - decDEG * 3600) / 60;
  decSS = (abs(DEC_tel_s) - decDEG * 3600) - decMM * 60;
  (DEC_tel_s < 0) ? sDEC_tel = 45 : sDEC_tel = 43;
/*
  hhh++;
  if(hhh == 24) hhh = 0;
  
  arHH = poleAR_HH;
  arMM = poleAR_MM;
  arSS = poleAR_SS;

  sDEC_tel = 43;
  decDEG = 89;
  decMM = 20;
  decSS = 35;
*/
  sprintf(txAR, "%02d:%02d:%02d#", int(arHH), int(arMM), int(arSS));
  sprintf(txDEC, "%c%02d%c%02d:%02d#", sDEC_tel, int(decDEG), 223, int(decMM), int(decSS));
}

long AltToSec(long enc)
{
  //long h_deg, h_min, h_seg, A_deg, A_min, A_seg;
  long sec;

  //90º -> ArcSec
  int enc1 = enc / 1500;
  long encoder1_temp = enc - (enc1 * 1500);
  long map1 = enc1 * map(1500, 0, pulses_enc1, 0, 324000);

  sec = map1 + map (encoder1_temp, 0, pulses_enc1, 0, 324000);

  return(sec);
}

long AzToSec(long enc)
{
  //long h_deg, h_min, h_seg, A_deg, A_min, A_seg;
  long sec;

  if (enc >= pulses_enc2 || enc <= -pulses_enc2) {
    enc = 0;
  }

  //360º -> ArcSec
  int enc2 = enc / 1500;
  long encoder2_temp = enc - (enc2 * 1500);
  long map2 = enc2 * map(1500, 0, pulses_enc2, 0, 1296000);

  sec  = map2 + map (encoder2_temp, 0, pulses_enc2, 0, 1296000);

  if (sec < 0) sec = 1296000 + sec;
  if (sec >= 1296000) sec = sec - 1296000 ;

  return(sec);
}
