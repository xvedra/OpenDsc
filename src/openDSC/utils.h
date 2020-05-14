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
#include "def.h"
#include "config.h"

#ifndef UTILS_H
#define UTILS_H

void serial_printf(const char *fmt, ... );
void dbg_serial_printf(const char *fmt, ... );
char *ftoa(char *a, double f, int precision);
char *HexEncoderValue(long value);
char *EncoderValue(long value, bool lead);
bool checkTimer(time_t timer, unsigned long msec);
time_t InitTimer(time_t load);


#define IS_WORD_END(x) (((x) == ' ') || ((x) == '\r') || ((x) == '\t') || ((x) == '\n'))
#define GETWORD_BUFFSIZE 128 
#define MAX_GETWORD_SIZE 127

#endif
