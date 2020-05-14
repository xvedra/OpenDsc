/*
 * Teensy 3.1 Digital Setting Circles
 * Copyright (C) 2014-2015 Aaron Turner
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "utils.h"

/*
 * printf to the HW serial port, useful for debugging.  Note 128char limit!
 */
void 
serial_printf(const char *fmt, ...) 
{
    char tmp[128]; // resulting string limited to 128 chars
    va_list args;
    va_start(args, fmt);
    vsnprintf(tmp, 128, fmt, args);
    va_end(args);
    Serial.print(tmp);
}

/*
 * printf to HW serial port, if we have debugging enabled
 */
void
dbg_serial_printf(const char *fmt, ...) 
{
    #ifdef DEBUG
    char tmp[128]; // resulting string limited to 128 chars
    va_list args;
    va_start(args, fmt);
    vsnprintf(tmp, 128, fmt, args);
    va_end(args);
    Serial.print("DEBUG: ");
    Serial.print(tmp);
    #else
    return;
    #endif
}

/*
 * Float to ascii
 * Since the sprintf() of the Arduino doesn't support floating point
 * converstion, #include <stdlib.h> for itoa() and then use this function
 * to do the conversion manually
 */
char 
*ftoa(char *a, double f, int precision)
{
    long p[] = { 0,10,100,1000,10000,100000,1000000,10000000,100000000 };

    char *ret = a;
    long heiltal = (long)f;
    itoa(heiltal, a, 10);
    while (*a != '\0') a++;
    *a++ = '.';
    long desimal = abs((long)((f - heiltal) * p[precision]));
    itoa(desimal, a, 10);
    return ret;
}

/*
 * Converts a 'long' into a encoder value string
 */
char *
EncoderValue(long value, bool printsign) {
    static char buff[BUFF_LEN];
    char sign;

    if (printsign) {
        if (value < 0) {
            sign = '-';
        } else {
            sign = '+';
        }
    }

    if (printsign) {
        sprintf(buff, "%c%05lu", sign, abs(value));
    } else {
        sprintf(buff, "%05lu", abs(value));
    }
    return buff;
}

char * 
HexEncoderValue(long value) {
  char low, high;
  static char buff[3];

  high = value / 256;
  low = value - high * 256;
  sprintf(buff, "%02x%02x", low, high);

  return buff;
}

bool checkTimer(time_t timer, unsigned long msec)
{
  return((unsigned long)(millis() - timer) >= msec);  
}

time_t InitTimer(time_t load)
{
  return(millis() + load);
}
