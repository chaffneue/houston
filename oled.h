/*
i2c based OLED Display driver for Houston
This is an adaptation of SeeedGrayOLED and TWI with specific changes
to suit Houston's scheduler - this library should work with many SSD1327 based 
OLED displays

@author Richard Hoar <richard.hoar@streeme.com>
@see https://github.com/chaffneue/houston
@see houston.ino for changelog

Derivation Notice - SeedGrayOLED License included
---
 * SSD1327 Gray OLED Driver Library
 *
 * Copyright (c) 2011 seeed technology inc.
 * Author        :   Visweswara R
 * Create Time   :   Dec 2011 - May 2017
 * Change Log    :
 * 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
---
  twi.c - TWI/I2C library for Wiring & Arduino
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 2012 by Todd Krein (todd@krein.org) to implement repeated starts
*/
#ifndef OLED_h
  #define OLED_h
  #if !defined(PROGMEM)
    #define PROGMEM
    #define pgm_read_byte(STR) STR
  #endif

  #define getTempoCharOffset(integralTempo) ((integralTempo % 10) * 6)

  // Driver Setup Configuration and Commands 
  #define DISPLAY_ADDRESS 0x3c
  #define COMMAND_MODE 0x80
  #define DATA_MODE 0x40
  #define SET_ROW 0x75
  #define SET_COLUMN 0x15
  #define COLOR_BLACK 0x00
  #define COLOR_DIVIDER 0x44
  #define DRIVER_COLUMN_OFFSET 8 // Offset left by 8 for the 96x96 display
  #define DRIVER_ROW_OFFSET 0    // Starts at 0 for the 96x96 display

  // This class uses a dirty rectangle strategy to write to the fairly slow 
  // screen hardware in the OLED. Passing these values to setDirty will tell
  // the driver to get fresh values.
  #define DIRTY_INT_EXT 1
  #define DIRTY_TAP 2
  #define DIRTY_TEMPO 3
  #define DIRTY_BAR_COUNTER 4
  #define DIRTY_LATENCY 5
  #define DIRTY_TEMPO_PRECISION 6
  #define DIRTY_LIST_LENGTH 99

  const unsigned char extLabel[49]  = {
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x07, 0x00, 0xF7, 0x00, 0x00, 0x00, 0x00, 
    0x7F, 0x00, 0xF7, 0x00, 0x0D, 0xC2, 0xF9, 0xFF, 0xF7, 0xFF, 0xFD, 0x02, 
    0xFD, 0xC0, 0x7F, 0x00, 0xF7, 0x00, 0x00, 0xBF, 0x40, 0x7F, 0x00, 0xF7, 
    0x00, 0x04, 0xFC, 0xD0, 0x7F, 0x20, 0xFF, 0xFF, 0x0E, 0xB0, 0xFA, 0x0E,
    0xF6
  };
  
  const unsigned char intLabel[49]  = {
    0xF6, 0x00, 0x00, 0x00, 0x07, 0x20, 0x00, 0xF7, 0x00, 0x00, 0x00, 0x0F, 
    0x70, 0x00, 0xF7, 0x0F, 0xDF, 0xB0, 0xDF, 0xF8, 0x00, 0xF7, 0x0F, 0x86, 
    0xF4, 0x0F, 0x70, 0x00, 0xF7, 0x0F, 0x70, 0xF7, 0x0F, 0x70, 0x00, 0xF7, 
    0x0F, 0x70, 0xF7, 0x0F, 0x60, 0x00, 0xF6, 0x0F, 0x60, 0xF6, 0x0C, 0xF8,
    0x00
  };

  const int tempoFontWidth = 68;
  const int tempoFontHeight = 16;
  const int systemFontWidth = 57;
  const int systemFontHeight = 8;
  const int systemViewportHeight = 8;
  const int bufferSize = 16;

  //Dynamic Viewports
  // Int/Ext - 14 x 7 pixels
  const int intExtViewportWidth = 7; //widths are divided in half on this driver
  const int intExtViewportHeight = 7;
  const int intExtViewportLength = 49;

  // Tap Counter - 40 x 8 pixels
  const int tapCounterViewportWidth = 20;
  const int tapCounterViewportHeight = 8;
  const int tapCounterViewportLength = 160;  
  
  // Tempo - 56 x 16 pixels
  const int tempoViewportWidth = 28; 
  const int tempoViewportHeight = 16;
  const int tempoViewportLength = 448;

  // Tempo Precision Indicator - 64 x 1 pixels
  const int tempoPrecisionViewportWidth = 28;
  const int tempoPrecisionViewportHeight = 1;
  const int tempoPrecisionViewportLength = 28;

  // Bar Counter - 26 x 8 pixels
  const int barCounterViewportWidth = 16;
  const int barCounterViewportHeight = 8;
  const int barCounterViewportLength = 128;
 
  // Latency - 76 x 8 pixels
  const int latencyViewportWidth = 38;
  const int latencyViewportHeight = 8;
  const int latencyViewportLength = 304;

  class OLED {
    public:
      void begin();
      void setIntExt(int state);
      void setTapState(int state);
      void setTempo(double tempo);
      void setTempoPrecision(int state);
      void setBarCounter(int playhead, int currentBeat);
      void setLatency(int channelLatency[]);
      int getDirtyId();
      void drawRect(int top, int left, int width, int height);
      void setRasterViewport(int dirtyId);
      bool raster();

    private:
      void sendCommand(unsigned char command);
      char sc;
      int dirtyViewportList[DIRTY_LIST_LENGTH];
      void pushToDirtyList(int dirtyId);
      void pushToTempoBitmap(int tempoFontOffset, int tempoViewportOffset, int character_width);
      void pushToSystemBitmap(unsigned char *viewport, int systemViewportWidth, int systemViewportOffset, int systemFontOffset, int characterWidth);
  };
#endif
