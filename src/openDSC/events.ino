#include "events.h"

// Fifo buffer /////////////////////////////
static eventsFifoData eventsFifo;
static byte eventsCurrentEvent = EVENT_NULL;

void eventsInit()
{
  eventsInitFifo();
}

void eventsLoop()
{
  if(eventsAvailableFifo())
  {    
    eventsGetFifo(&eventsCurrentEvent);
    //Serial.printf("New current event: %d\n", eventsCurrentEvent);
  }
  else eventsClearCurrentEvent();
}

bool eventsIsCurrentEvent()
{
  return(eventsCurrentEvent != EVENT_NULL);
}

byte eventsReadCurrentEvent()
{
  return(eventsCurrentEvent);
}

void eventsClearCurrentEvent()
{
  eventsCurrentEvent = EVENT_NULL;
}

void eventsInitFifo()
{
  eventsFifo.Rd = 0;
  eventsFifo.Wr = 0;
  eventsFifo.n = 0;
}

bool eventsIsNewFifo()
{
  return(eventsFifo.Wr != eventsFifo.Rd);
}

byte eventsAvailableFifo()
{
    return(eventsFifo.n);
}

bool eventsIsFullFifo()
{
  return(eventsFifo.n == EVENTS_FIFO_SIZE);
}

bool eventsIsEmptyFifo()
{
  return(eventsFifo.n == 0);
}

bool eventsPutFifo(byte data)
{
  if(eventsFifo.n < EVENTS_FIFO_SIZE)
  {
    //Serial.printf("Put event %d at %d\n", data, eventsFifo.Wr);
    eventsFifo.buf[eventsFifo.Wr] = data;
    eventsFifo.Wr = (eventsFifo.Wr + 1) % EVENTS_FIFO_SIZE;
    eventsFifo.n = eventsFifo.n + 1;
    return 0;
  }
  //Serial.println("Put event fail");
  return 1;
}

bool eventsGetFifo(byte *data)
{
  if(eventsFifo.n > 0)
  {
    //Serial.printf("Get event %d from %d\n", eventsFifo.buf[eventsFifo.Rd], eventsFifo.Rd);
    if(data != NULL) *data = eventsFifo.buf[eventsFifo.Rd];    
    //eventsFifo.buf[eventsFifo.Rd] = EVENT_NULL;
    eventsFifo.Rd = (eventsFifo.Rd + 1) % EVENTS_FIFO_SIZE;
    eventsFifo.n = eventsFifo.n - 1;
    return 0;
  }
  return 1;
}

