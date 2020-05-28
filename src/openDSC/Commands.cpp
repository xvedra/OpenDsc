
#include "Commands.h"
#include "eq.h"

// Globals ///////////////////////////////////////
WiFiServer server(TCP_PORT);
WiFiClient serverClients[MAX_SRV_CLIENTS];
IPAddress  apIP(10, 0, 0, 1);

//long ALT_Res = ALT_Res_DEF;  // resolution of encoders
//long AZ_Res = AZ_Res_DEF;
//long ALT_Pos = ALT_Res_DEF/2;  // initial position of encoders is 
//long AZ_Pos = AZ_Res_DEF/2;   // half their resolution
extern long AltRes;  // resolution of encoders
extern long AzRes;
extern long AltPos;  // initial position of encoders is 
extern long AzPos;   // half their resolution
#define ALT_Res AltRes  // resolution of encoders
#define AZ_Res AzRes  // resolution of encoders
#define ALT_Pos AltPos  // initial position of encoders is 
#define AZ_Pos AzPos   // half their resolution
int beenAligned = 0;
BluetoothSerial SerialBT;
static unsigned long int GetPosReqCtr = 0;
bool WIFIon = 0;
bool BTon = 0;
bool BTConnected = 0;
unsigned long int BAUDRATES[8] = {1200,2400,4800,9600,19200,38400,57600,115200};
// BT event callback /////////////////////////////
void BTcallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
  if(event == ESP_SPP_SRV_OPEN_EVT)
  {
    #ifdef USE_DEBUG
    Serial.println("BT Client Connected");
    #endif
    BTConnected = 1;
  }
 
  if(event == ESP_SPP_CLOSE_EVT )
  {
    #ifdef USE_DEBUG
    Serial.println("BT Client disconnected");
    #endif
    BTConnected = 0;
  }
}
// Init //////////////////////////////////////////
void commandsInit(void)
{ 
  #ifdef pinLED1
  byte ctr = 10;
  pinMode(pinLED1, OUTPUT);  
  while(ctr--)
  {
    digitalWrite(pinLED1, LOW);
    delay(100);
    digitalWrite(pinLED1, HIGH);
    delay(100);
  }
  #endif

  #ifdef USE_L200
  eqSetup();
  #endif
}

// Loop //////////////////////////////////////////
void commandsLoop(void)
{  
  static time_t timer1 = 0;
  #ifdef USE_L200
  static time_t timer2 = 0;
  #endif
  
  dsc_Loop(); 
  ALT_Pos = dsc_GetAlt();
  AZ_Pos = dsc_GetAz();
  
  #ifdef USE_L200
  if(checkTimer(timer2, 100))
  {
    eqLoop(ALT_Pos, AZ_Pos);
    timer2 = InitTimer(0);
  }
  #endif

  #ifndef USE_DEBUG
  SerialProcessCommands(&Serial);
  #endif
  
  if(BTon) SerialProcessCommands(&SerialBT);
  
  if(checkTimer(timer1, WIFI_LOOP_T_MS))
  {
    timer1 = InitTimer(0);
    if(WIFIon) WIFI_loop();    
  }
  
  #ifdef USE_SOLVE_SKYSAFARI_AUTOMATICALLY_RESOLUTION_BUG
  GetAutomatically(); //solves bug for Skysafari
  #endif
}

/*
// print encoder value with leading zeros
void printEncoderValue(long val)
{
  unsigned long aval; 

  if (val < 0)
    SerialBT.print("-");
  else
    SerialBT.print("+");

  aval = abs(val);

  if (aval < 10)
    SerialBT.print("0000");
  else if (aval < 100)
    SerialBT.print("000");
  else if (aval < 1000)
    SerialBT.print("00");
  else if (aval < 10000) 
    SerialBT.print("0");

  SerialBT.print(aval);  
}
*/

void printHexEncoderValue(Stream *serial, long val)
{
  byte low, high;

  high = val / 256;
  low = val - high*256;
  serial->write(low);
  serial->write(high);
  return;
}

void printTime(void)
{
  unsigned int Time, secs, decs, mins, hours, days;

  Time = millis();
  decs = (Time / 100) % 10;
  secs = (Time / 1000) % 60;
  mins = (Time / (60 * 1000)) % 60; 
  hours = (Time / (60 * 60 * 1000)) % 24; 
  days = (Time / (24 * 60 * 60 * 1000)); 
  Serial.print(days);
  Serial.print("/");
  Serial.print(hours);
  Serial.print(":");
  Serial.print(mins);
  Serial.print(":");
  Serial.print(secs);
  Serial.print("'");
  Serial.print(decs);
  Serial.print("> ");
}

/* string is of format #<tab>#<cr> */
void parseSetResolutionCmd(char *cmd)
{
  char *p;
  int altres, azres;

  for (p=cmd; *p && *p != '\t'; p++);

  /* hit tab */
  if (*p) {
    *p = '\0';

    azres = atoi(cmd);
    altres = atoi(p+1);

#ifdef USE_DEBUG
    Serial.print(azres);
    Serial.print(" ");
    Serial.print(altres);
    Serial.println();    
#endif

    ALT_Res = altres;
    AZ_Res = azres;
    dsc_SetAltAzRes(ALT_Res, AZ_Res);
  }
}

void parseEkSetResolutionCmd(Stream *serial)
{
  byte b1 = serial->read();
  byte b2 = serial->read();
  ALT_Res = (uint16_t)b2*256+b1;

  b1 = serial->read();
  b2 = serial->read();
  AZ_Res = (uint16_t)b2*256+b1;

  dsc_SetAltAzRes(ALT_Res, AZ_Res);
  
  #ifdef USE_DEBUG
  Serial.print(ALT_Res);
  Serial.print(" ");
  Serial.print(AZ_Res);
  Serial.println();    
  #endif  
}

unsigned long int ReadGetPosReqCtr()
{
  return(GetPosReqCtr);
}

void readLine(Stream *serial, char *p, int maxlen)
{
  int i=0;
  char *s = p;
  
  do
  {
    *s = serial->read();    
    if (!*s) return;    
    if (*s == '\r') break;     
    s++;
  } while (i++ < maxlen-1);
  
  *s = '\0';
}

void SerialProcessCommands(Stream *serial)
{
    char buff[CLIENT_BUFF_LEN];
    char *value;
    char val;
    int i, j;

    if(serial->available()) val = serial->read();
    else return;

    switch (val) {        
        case 'Q':
            // get encoder values
            GetPosReqCtr++;

            value = EncoderValue(AZ_Pos, true);
            sprintf(buff, "%s\t", value);
            value = EncoderValue(ALT_Pos, true);
            strcat(buff, value);
            serial->printf("%s\r\n", buff);    
            
            /*
            printEncoderValue(AZ_Pos);
            serial->print("\t");
            printEncoderValue(ALT_Pos);
            serial->print("\r\n");  
            */          
            
            break;
        case 'Z':
        case 'R':
        case 'I':
            // set resolution
            long az, alt;
            
            readLine(serial, buff, CLIENT_BUFF_LEN-1);
            parseSetResolutionCmd(buff);

            if(sscanf(buff, "%ld %ld", &az, &alt) != 2)
            {
              #ifdef USE_DEBUG
              Serial.printf("R/I: Fail arguments != 2\n");
              #endif
            }
            else
            {
              AZ_Res = az;
              ALT_Res = alt;           
              dsc_SetAltAzRes(ALT_Res, AZ_Res);  
              if(val = 'Z')serial->print("*");
              else serial->print("R");
              #ifdef USE_DEBUG
              Serial.printf("Z,R/I: Set resolution...done\n");
              #endif
            }
            /*
            memset(buff, '\0', CLIENT_BUFF_LEN);
            i = 0;
            j = 0;
            // Horrible hack around ESP8266 not having sscanf()
            if (serial->available()) {
                serial->read(); // read the space
            } else {
                #ifdef USE_DEBUG
                Serial.printf("short command!\n");
                #endif
                return;
            }

            while (i < 2) {
                while (serial->available() && serial->peek() != ' ') {
                    if (serial->peek() != ' ') {
                        buff[j] = serial->read();
                        j++;
                    }
                }
                if (i == 0) {
                    j = 0;
                    az_value = String(buff).toInt();
                    #ifdef USE_DEBUG
                    Serial.printf("AZ = %ld\n", az_value);
                    #endif
                    memset(buff, '\0', CLIENT_BUFF_LEN);
                    serial->read(); // read the space
                } else {
                    alt_value = String(buff).toInt();
                    #ifdef USE_DEBUG
                    Serial.printf("ALT = %ld\n", alt_value);
                    #endif
                }
                i++;
            }
            if (i != 2) {
                #ifdef USE_DEBUG
                Serial.printf("Unable to process: R %s\n", buff);
                #endif
                return;
            }
            AZ_Res = az_value;
            ALT_Res = alt_value;           
            dsc_SetAltAzRes(ALT_Res, AZ_Res);      
            */      
            break;
        case 'G':
        case 'H':               
        case 'r':  
            AZ_Res = dsc_GetAzRes();
            ALT_Res = dsc_GetAltRes();
            
            /*
            printEncoderValue(AZ_Res);
            serial->print("\t");
            printEncoderValue(ALT_Res);
            serial->print("\r\n");  
            */ 

            value = EncoderValue(AZ_Res, true);
            sprintf(buff, "%s\t", value);
            value = EncoderValue(ALT_Res, true);
            strcat(buff, value);
            serial->printf("%s\r\n", buff);
            #ifdef USE_DEBUG
            Serial.printf("Get Res> Az:%ld\tAlt:%ld\n", AZ_Res, ALT_Res);
            Serial.printf("Pos> Az:%ld\t%Alt:%ld\n", AZ_Pos, ALT_Pos);
            #endif
            break;         
        case 'T':
            //Toggle test mode on/off

            //Read Encoders
            /*
            printEncoderValue(AZ_Res);
            serial->print("\t");
            printEncoderValue(ALT_Res);
            serial->print("\t00000\r\n");
            */
            /*
            value = EncoderValue(AZ_Res, true);
            sprintf(buff, "%s\t", value);
            value = EncoderValue(ALT_Res, true);
            serial->printf("%s%s\t00000\r\n", buff, value);
            */           
            /*
            // get resolution
            long az, alt;
            char az_pos[2], alt_pos[2];

            dsc_GetAltAzRes(&ALT_Res, &AZ_Res);
            az = abs(AZ_Res);
            alt = abs(ALT_Res);

            az_pos[1] = alt_pos[1] = '\0';
            az_pos[0] = az == AZ_Res ? '+' : '-';
            alt_pos[0] = alt == ALT_Res ? '+' : '-';
            #ifdef USE_DEBUG
            Serial.printf("RES: %s%05ld\t%s%05ld\r\n", az_pos, az, alt_pos, alt); 
            #endif
            if(val == 'T') serial->printf("%s%05ld\t%s%05ld\t00000\r\n", az_pos, az, alt_pos, alt); 
            else serial->printf("%s%05ld\t%s%05ld\r\n", az_pos, az, alt_pos, alt); 
            */
            break; 
           
        case 'V':
        case 'v':
            // get version
            serial->printf("ESP-DSC v%s\r\n", ESP_DSC_VERSION);
            break;          
        case 'P':
            //Get status
            serial->printf("001\r\n");
            break;
        case 'A':
            beenAligned = 1;
            break;
        case 'a':
            if (beenAligned) serial->print("Y");
            else serial->print("N");
            break;
        case 'q':
            // error count
            serial->print("00000\r\n");
            break;     
        //USE_EKS......................
        case 'z': 
            //Set the encoder resolution
            byte b1, b2;
            b1 = serial->read();
            b2 = serial->read();            
            ALT_Res = (uint16_t)b2*256+b1;          
            b1 = serial->read();
            b2 = serial->read();
            AZ_Res = (uint16_t)b2*256+b1;
            serial->print("r");
            dsc_SetAltAzRes(ALT_Res, AZ_Res);
            break;           
        case 'h': 
            AZ_Res = dsc_GetAzRes();
            ALT_Res = dsc_GetAltRes();
            //Report the encoder resolution 
            printHexEncoderValue(serial, abs(ALT_Res));
            printHexEncoderValue(serial, abs(AZ_Res));
            break; 
        case 'y': 
            //Report the encoder positions
            printHexEncoderValue(serial, abs(ALT_Pos));
            printHexEncoderValue(serial, abs(AZ_Pos));
            break;
        case 'p': 
            //Report the number of encoder errors and reset the counter to zero
            serial->print("00");
            break;            
        default:
            #ifdef USE_DEBUG
            Serial.printf("BT.Unknown command: %c\r\n", val);
            #endif
            break;
    }
}


void WiFiProcessClient(uint8_t c)
{
    char buff[CLIENT_BUFF_LEN];
    char *value;
    char val = serverClients[c].read();
    long az_value, alt_value;
    int i, j;
    
    switch (val) 
    {
        // get encoder values
        case 'Q':
            value = EncoderValue(AZ_Pos, true);
            sprintf(buff, "%s\t", value);
            value = EncoderValue(ALT_Pos, true);
            serverClients[c].printf("%s%s\r\n", buff, value);
            break;
        case 'R':
            // set resolution
            memset(buff, '\0', CLIENT_BUFF_LEN);
            i = 0;
            j = 0;
            // Horrible hack around ESP8266 not having sscanf()
            if (serverClients[c].available()) {
                serverClients[c].read(); // read the space
            } else {
                #ifdef USE_DEBUG
                Serial.printf("short command!\n");
                #endif
                return;
            }

            while (i < 2) {
                while (serverClients[c].available() && serverClients[c].peek() != ' ') {
                    if (serverClients[c].peek() != ' ') {
                        buff[j] = serverClients[c].read();
                        j++;
                    }
                }
                if (i == 0) {
                    j = 0;
                    az_value = String(buff).toInt();
                    #ifdef USE_DEBUG
                    Serial.printf("AZ = %ld\n", az_value);
                    #endif
                    memset(buff, '\0', CLIENT_BUFF_LEN);
                    serverClients[c].read(); // read the space
                } else {
                    alt_value = String(buff).toInt();
                    #ifdef USE_DEBUG
                    Serial.printf("ALT = %ld\n", alt_value);
                    #endif
                }
                i++;
            }
            if (i != 2) {
                #ifdef USE_DEBUG
                Serial.printf("Unable to process: R %s\n", buff);
                #endif
                return;
            }
            AZ_Res = az_value;
            ALT_Res = alt_value;           
            dsc_SetAltAzRes(ALT_Res, AZ_Res);  

        case 'G':
        case 'H':
            // get resolution
            long az, alt;
            char az_pos[2], alt_pos[2];

            az = abs(AZ_Res);
            alt = abs(ALT_Res);

            az_pos[1] = alt_pos[1] = '\0';
            az_pos[0] = az == AZ_Res ? '+' : '-';
            alt_pos[0] = alt == ALT_Res ? '+' : '-';
            #ifdef USE_DEBUG
            Serial.printf("%s%05ld\t%s%05ld\r\n", az_pos, az, alt_pos, alt); 
            #endif
            serverClients[c].printf("%s%05ld\t%s%05ld\r\n", az_pos, az, alt_pos, alt); 
            break; 
        case 'V':
        case 'v':
            // get version
            serverClients[c].printf("ESP-DSC v%s\n", ESP_DSC_VERSION);
            break;         
            
        #ifdef USE_L200   
        case ':': //Meade L200
            i = 0;
            while (serverClients[c].available() && serverClients[c].peek() != ' ')
            {
              char input = serverClients[c].peek();
              if(input != ' ')
              {
                buff[i] = serverClients[c].read();                
                i++;
              }
              if(input == '#')
              {
                if(buff[0] == 'G' && buff[1] == 'R' && buff[2] == '#')
                {
                  #ifdef USE_DEBUG
                  //Serial.printf("L200> AR: %s\r\n", get_Ar()); 
                  #endif
                  serverClients[c].printf("%s\r\n", get_Ar());
                }
                if(buff[0] == 'G' && buff[1] == 'D' && buff[2] == '#')
                {
                  #ifdef USE_DEBUG
                  //Serial.printf("L200> DEC: %s\r\n", get_Dec());
                  #endif
                  serverClients[c].printf("%s\r\n", get_Dec());
                }
                break;
              }
            }
            break;
        #endif
           
        default:
            #ifdef USE_DEBUG
            Serial.printf("WIFI.Unknown command: %c\n", val);
            #endif
            break;
    }
}


// To solve a Skysafari bug....
void GetAutomatically()
{
  static int stt =0;

  switch(stt)
  {
    case 0:
    if(isBtConnected() || isWiFiConnected())
    {
      GetPosReqCtr = 0;
      dsc_Disable();
      dsc_SetAlt(dsc_GetAltRes());
      dsc_SetAz(dsc_GetAzRes());
      #ifdef USE_DEBUG
      Serial.printf("Connected: %ld %ld %ld %ld\n", dsc_GetAltRes(),dsc_GetAzRes(),dsc_GetAlt(),dsc_GetAz());
      #endif
      stt++;
    }
    break;
    case 1:
    if(isBtConnected() || isWiFiConnected())
    {
      if(GetPosReqCtr >= 12)
      {
        dsc_SetAlt(abs(dsc_GetAltRes()/2));
        dsc_SetAz(abs(dsc_GetAzRes()/2));
        dsc_Enable(); 
        stt++;       
        #ifdef USE_DEBUG
        Serial.printf("Done: %ld %ld %ld %ld\n", dsc_GetAltRes(),dsc_GetAzRes(),dsc_GetAlt(),dsc_GetAz());
        #endif
      }
    }
    else
    {
        dsc_SetAlt(abs(dsc_GetAltRes()/2));
        dsc_SetAz(abs(dsc_GetAzRes()/2));
        dsc_Enable(); 
        stt = 0;  
        #ifdef USE_DEBUG
        Serial.printf("Disconnected: %ld %ld %ld %ld\n", dsc_GetAltRes(),dsc_GetAzRes(),dsc_GetAlt(),dsc_GetAz());
        #endif
    }
    break;
    case 3:
    if(!isBtConnected() && !isWiFiConnected()) stt = 0;
    break;
  }
}

void connectUsbSerial(byte index)
{
  Serial.flush();
  Serial.begin(BAUDRATES[index&7]);
}

bool isBtConnected()
{
  return(BTConnected);
}

void connectBluetooth()
{
  if(BTon == 0)
  {
    #ifdef USE_DEBUG
    Serial.printf("\nConnecting BT...\n");  
    #endif 
    esp_bluedroid_init();
    esp_bluedroid_enable();
    //initBluetooth();
    delay(100);
    SerialBT.begin(BT_NAME);
    SerialBT.register_callback(BTcallback);  
    delay(1000);
    BTon = 1;
  }
}

void disconnectBluetooth()
{
  if(BTon)
  {
    #ifdef USE_DEBUG
    Serial.printf("\nDisconnecting BT...\n");
    #endif
    SerialBT.flush();
    esp_bluedroid_disable();
    esp_bluedroid_deinit();  
    delay(100);   
    BTon = 0;
  }
}

bool isBluetoothEnabled()
{
  return(BTon);
}

void connectWIFI()
{  
  if(WIFIon == 0)
  {
    #ifdef USE_DEBUG
    Serial.printf("\nConnecting WIFI...\n");
    #endif
    WiFi.mode(WIFI_AP);
    delay(500);
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
    delay(500);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    delay(100);
    IPAddress myIP = WiFi.softAPIP();
    #ifdef USE_DEBUG
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    #endif
    server.begin();
    server.setNoDelay(true);   
    WIFIon = 1;
  }
}
void disconnectWIFI()
{
  if(WIFIon)
  {
    #ifdef USE_DEBUG
    Serial.printf("\nDisconnecting WIFI...\n");
    #endif
    for (byte c = 0; c < MAX_SRV_CLIENTS; c++)
    {
      serverClients[c].flush();
      serverClients[c].stop();
    }
    server.stop();
    server.close();
    //WiFi.mode(WIFI_MODE_NULL);
    WiFi.mode(WIFI_OFF);
    delay(1000);
    WiFi.disconnect(true,true);
    delay(1000);
    WIFIon = 0;
  }
}

bool isWiFiEnabled()
{
  return(WIFIon);
}

bool isWiFiConnected()
{
  return(WiFi.softAPgetStationNum() != 0);
}

void WIFI_loop()
{ 
    //unsigned long now = millis();
    uint8_t c;
    long alt_value, az_value;
    char buffer[50], az_f[10], alt_f[10];
    
    // look for new clients
    if (server.hasClient()) {
        for (c = 0; c < MAX_SRV_CLIENTS; c++) {
            // find a free slot
            if (!serverClients[c] || !serverClients[c].connected()) {
                if (serverClients[c]) 
                    serverClients[c].stop();
                serverClients[c] = server.available();
                continue;
            }
        }
        
        // no free slots to reject
        WiFiClient serverClient = server.available();
        serverClient.stop();
    }

    // check clients for data
    for (c = 0; c < MAX_SRV_CLIENTS; c++) {
        if (serverClients[c] && serverClients[c].connected()) {
            if (serverClients[c].available()) {
                WiFiProcessClient(c);
            }
        }
    }
    //delay(25); // service WiFiClient
}


