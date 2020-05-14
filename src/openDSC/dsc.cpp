
#include "dsc.h"
#include "lib/AS5600.h"

// Globals //////////////////////////////////////////////////////////////////////////
long AltRes = RES_ALT_DEF;  // resolution of encoders
long AzRes = RES_AZ_DEF;
long AltPos = abs(RES_ALT_DEF/2);  // Set tu RES for "Get automatically" Skysafari command
long AzPos = abs(RES_AZ_DEF/2);   // Set tu RES for "Get automatically" Skysafari command
int AltSensor = -1;
int AzSensor = -1;

AS5600 EncoderAZ(SDA1_PIN,SCL1_PIN);
AS5600 EncoderALT(SDA2_PIN,SCL2_PIN);

// Protos ///////////////////////////////////////////////////////////////////////////

// Functions ////////////////////////////////////////////////////////////////////////
void dsc_Init(void)
{
    switch(AltSensor)
    {
      case DSC_QUADRATURE:  
        #ifdef USE_DEBUG
        Serial.println("No ALT encoder defined: QUADRATURE\n"); 
        #endif
        AltSensor = 1;
        break;
      case DSC_AS5600:        
        EncoderALT.setup(RES_ALT_DEF, ALT_FILTER_LEVEL);
        break;
      case DSC_AS5048:      
        #ifdef USE_DEBUG
        Serial.println("No ALT encoder defined: AS5048\n"); 
        #endif
        AltSensor = 1;
        break;
      case DSC_MPU6050:     
        #ifdef USE_DEBUG
        Serial.println("No ALT encoder defined: MPU6050\n"); 
        #endif
        AltSensor = 1;
        break;
    }
    switch(AzSensor)
    {
      case DSC_QUADRATURE:  
        #ifdef USE_DEBUG
        Serial.println("No AZ encoder defined: QUADRATURE\n"); 
        #endif
        AzSensor = 1;
        break;
      case DSC_AS5600:        
        EncoderAZ.setup(RES_AZ_DEF, AZ_FILTER_LEVEL); 
        break;
      case DSC_AS5048:      
        #ifdef USE_DEBUG
        Serial.println("No AZ encoder defined: AS5048\n"); 
        #endif
        AzSensor = 1;
        break;
      case DSC_MPU6050:     
        #ifdef USE_DEBUG
        Serial.println("No AZ encoder defined: MPU6050\n"); 
        #endif
        AzSensor = 1;
        break;
    }
  //Init QUADRATURE:
  
  //Init AS5600:


  //Init AS5048:
  
  //Init MPU6050:

  //
}

void dsc_Loop(void)
{
  static time_t timer = 0;

  if(checkTimer(timer, DSC_TIME_LOOP_MS))
  {
    timer = InitTimer(0);
    
    EncoderALT.loop();
    EncoderAZ.loop();
    //AltPos = EncoderALT.read();
    //AzPos = EncoderAZ.read();   

    switch(AltSensor)
    {
      case DSC_QUADRATURE:  AltPos = abs(AltRes/2); break;
      case DSC_AS5600:      AltPos = EncoderALT.read(); break;
      case DSC_AS5048:      AltPos = abs(AltRes/2); break;
      case DSC_MPU6050:     AltPos = abs(AltRes/2); break;
    }
    switch(AzSensor)
    {
      case DSC_QUADRATURE:  AzPos = abs(AzRes/2); break;
      case DSC_AS5600:      AzPos = EncoderAZ.read();  break;
      case DSC_AS5048:      AzPos = abs(AzRes/2);; break;
      case DSC_MPU6050:     AzPos = abs(AzRes/2);; break;
    }
  }
}


// Get AltAz ///////////////////////////////////
void dsc_GetAltAz(long *alt, long *az)
{
  switch(AltSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      *alt = AltPos = EncoderALT.read(); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  } 
  switch(AzSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      *az = AzPos = EncoderAZ.read(); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  } 
}



// Set AltAz Resolution /////////////////////
void dsc_SetAltAzRes(long alt, long az)
{
  AltRes = alt;
  switch(AltSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      EncoderALT.setEncoderResolution(AltRes); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  }  
  
  AzRes = az;
  switch(AltSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      EncoderAZ.setEncoderResolution(AzRes); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  }   
}

// Set Alt Resolution ///////////////////////
void dsc_SetAltRes(long alt)
{
  AltRes = alt;
  switch(AltSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      EncoderALT.setEncoderResolution(AltRes); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  }   
}

// Set Az Resolution ////////////////////////
void dsc_SetAzRes(long az)
{
  AzRes = az;  
  switch(AzSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      EncoderAZ.setEncoderResolution(AzRes); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  }  
}

// Get AltAz Resolution /////////////////////
void dsc_GetAltAzRes(long *alt, long *az)
{
  switch(AltSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      EncoderALT.getEncoderResolution(&AltRes); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  } 
  switch(AzSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      EncoderAZ.getEncoderResolution(&AzRes); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  }   
  *alt = AltRes;  
  *az = AzRes;
}


// Get Alt Resolution ///////////////////////
long dsc_GetAltRes(void)
{
  switch(AltSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      EncoderALT.getEncoderResolution(&AltRes); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  }    
  return(AltRes);
}

// Get Az Resolution ////////////////////////
long dsc_GetAzRes(void)
{
  switch(AzSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      EncoderAZ.getEncoderResolution(&AzRes); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  }   
  return(AzRes);
}

// Get Alt ///////////////////////////////////
long dsc_GetAlt(void)
{  
  switch(AltSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      AltPos = EncoderALT.read(); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  } 
  return(AltPos);
}

void dsc_SetAlt(long count)
{
  AltPos = count;
  switch(AltSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      EncoderALT.write(count); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  }   
}

// Get Az ///////////////////////////////////
long dsc_GetAz(void)
{    
  switch(AzSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      AzPos = EncoderAZ.read(); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  }  
  return(AzPos);
}

void dsc_SetAz(long count)
{
  AzPos = count;
  switch(AzSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      EncoderAZ.write(count); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  }   
}

void dsc_Disable()
{
  switch(AltSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      EncoderALT.disable(); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  }   
  switch(AzSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      EncoderAZ.disable(); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  } 
}

void dsc_Enable()
{
  switch(AltSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      EncoderALT.enable(); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  }   
  switch(AzSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      EncoderAZ.enable(); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  }  
}

bool dsc_IsEnabled()
{
  bool AltEn = 0, AzEn = 0;
  switch(AltSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      AltEn = EncoderALT.isEnabled(); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  }   
  switch(AzSensor)
  {
    case DSC_QUADRATURE:  break;
    case DSC_AS5600:      AzEn = EncoderAZ.isEnabled(); break;
    case DSC_AS5048:      break;
    case DSC_MPU6050:     break;
  } 
  return(AltEn && AzEn);
}

void dsc_SetAltSensor(int sensor)
{
  if(sensor != AltSensor)
  {
    AltSensor = sensor;
    dsc_Init();  
    dsc_SetAlt(abs(AltRes/2));
    dsc_SetAz(abs(AzRes/2));
  }
}

void dsc_SetAzSensor(int sensor)
{
  if(sensor != AzSensor)
  {
    AzSensor = sensor;  
    dsc_Init();
    dsc_SetAlt(abs(AltRes/2));
    dsc_SetAz(abs(AzRes/2));
  }
}

int dsc_GetAltSensor()
{
  return(AltSensor);
}

int dsc_GetAzSensor()
{
  return(AzSensor);
}

