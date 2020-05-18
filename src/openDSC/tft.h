#include <TFT_eSPI.h>
#include <SPI.h>
#include <menu.h>
#include <Button2.h>
#include <menuIO/serialIO.h>
#include <menuIO/TFT_eSPIOut.h>

#include "pinMap.h"

// bitmaps
#include "img/OpenDSClogo.h"
#include "img/BTonoff.h"
#include "img/VBatt.h"
#include "img/okfail1.h"
#include "img/WIFIonoff.h"

#ifndef _TFT_H
#define _TFT_H

#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN   0x10
#endif

#define GFX_WIDTH 240
#define GFX_HEIGHT 135
#define fontW 12
#define fontH 18
#define MAX_DEPTH 5

// define menu colors --------------------------------------------------------
//  {{disabled normal,disabled selected},{enabled normal,enabled selected, enabled editing}}
//monochromatic color table
#define Black RGB565(0,0,0)
#define Red  RGB565(255,0,0)
#define Green RGB565(0,255,0)
#define Blue RGB565(0,0,255)
#define Gray RGB565(128,128,128)
#define LighterRed RGB565(255,150,150)
#define LighterGreen RGB565(150,255,150)
#define LighterBlue RGB565(150,150,255)
#define DarkerRed RGB565(150,0,0)
#define DarkerGreen RGB565(0,150,0)
#define DarkerBlue RGB565(0,0,150)
#define Cyan RGB565(0,255,255)
#define Magenta RGB565(255,0,255)
#define Yellow RGB565(255,255,0)
#define White RGB565(255,255,255)

// Menu colors ///////////////////////////////////
const colorDef<uint16_t> colors[] MEMMODE={
  {
    {
      (uint16_t)Black,
      (uint16_t)Black
    },
    {
      (uint16_t)Black,
      (uint16_t)DarkerBlue,
      (uint16_t)DarkerBlue
    }
  },//bgColor
  {
    {
      (uint16_t)Gray,
      (uint16_t)Gray
    },
    {
      (uint16_t)White,
      (uint16_t)White,
      (uint16_t)White
    }
  },//fgColor
  {
    {
      (uint16_t)White,
      (uint16_t)Black
    },
    {
      (uint16_t)Yellow,
      (uint16_t)Yellow,
      (uint16_t)Red
    }
  },//valColor
  {
    {
      (uint16_t)White,
      (uint16_t)Black
    },
    {
      (uint16_t)White,
      (uint16_t)Yellow,
      (uint16_t)Yellow
    }
  },//unitColor
  {
    {
      (uint16_t)White,
      (uint16_t)Gray
    },
    {
      (uint16_t)Black,
      (uint16_t)Blue,
      (uint16_t)White
    }
  },//cursorColor
  {
    {
      (uint16_t)White,
      (uint16_t)Yellow
    },
    {
      (uint16_t)DarkerRed,
      (uint16_t)White,
      (uint16_t)White
    }
  },//titleColor
};


#endif
