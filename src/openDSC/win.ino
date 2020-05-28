#include "def.h"

static time_t TimerDrawData = 0;

void winLoop()
{
  static byte stt = 0; 

  //if(eventsReadCurrentEvent() != EVENT_NULL) Serial.printf("IdleLoop> stt:%d e:%d\n", stt, eventsReadCurrentEvent());

  if(eventsReadCurrentEvent() == EVENT_IDLE_START)
  {
    #ifdef USE_SAVE_AT_EXIT_MENU    
    uploadEEPROM(); //First exit from menu: save!
    #endif
  }

  switch(stt)
  {
    case 0: 
      if(eventsReadCurrentEvent() == EVENT_IDLE_START) stt++;
      break;
    case 1:
      if(eventsReadCurrentEvent() == EVENT_IDLE_DONE) stt++;
      break;
    case 2:
      winMain(SECTION_START);    
      stt++;      
      break;
    case 3:
      if(isIdle) winMain(SECTION_RUN);
      if(eventsReadCurrentEvent() == EVENT_BTN_DWN_LONG_CLICK)
      {
        winMain(SECTION_END);
        stt++;
      }
      if(eventsReadCurrentEvent() == EVENT_IDLE_END)
      {
        winMain(SECTION_END);        
        stt = 0;
      }
      break;
    case 4:
      winDemo(SECTION_START); 
      stt++;
      break;
    case 5:
      if(isIdle) winDemo(SECTION_RUN);
      if(eventsReadCurrentEvent() == EVENT_BTN_DWN_LONG_CLICK)
      {
        stt = 2;
        winDemo(SECTION_END);
      }
      if(eventsReadCurrentEvent() == EVENT_IDLE_END)
      {
        winDemo(SECTION_END);
        stt = 0;
      }
      break;
  }
}
