#include "oled.h"
#include "Arduino.h"
extern "C" {
  #include <avr/pgmspace.h>
  #include "Twi.h"
}

// Initialize the viewport rectangles and common interators
unsigned char intExtViewport[intExtViewportLength];
unsigned char tapCounterViewport[tapCounterViewportLength]; 
unsigned char tempoViewport[tempoViewportLength];
unsigned char tempoPrecisionViewport[tempoPrecisionViewportLength];
unsigned char barCounterViewport[barCounterViewportLength];
unsigned char latencyViewport[latencyViewportLength];
unsigned char rasterViewport[tempoViewportLength]; // double buffer is sized to the largest viewport

int rasterIndex, rasterLength, dirtyIndex, currentDirtyIndex, row, column;

const unsigned char tempoFont[1088] PROGMEM = {
  0x00, 0x09, 0xEF, 0xFE, 0xA0, 0x00, 0x00, 0x02, 0xAF, 0xFF, 0xD0, 0x00, 
  0x06, 0xBD, 0xFF, 0xFC, 0x70, 0x00, 0x06, 0xAD, 0xDF, 0xFD, 0xA2, 0x00, 
  0x00, 0x00, 0x07, 0xFF, 0xFF, 0x70, 0x00, 0xDF, 0xFF, 0xFF, 0xFF, 0xF0, 
  0x00, 0x00, 0x09, 0xCD, 0xFF, 0x70, 0xAF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0x00, 0x2B, 0xEF, 0xFF, 0xC7, 0x00, 0x00, 0x2A, 0xEF, 0xFE, 0xA2, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDF, 0xFF, 0xFF, 
  0xFE, 0x20, 0x04, 0xCF, 0xFF, 0xFF, 0xD0, 0x00, 0xCF, 0xFF, 0xFF, 0xFF, 
  0xFC, 0x00, 0x7F, 0xFF, 0xFF, 0xFF, 0xFE, 0x20, 0x00, 0x00, 0x0F, 0xFF, 
  0xFF, 0x70, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x07, 0xFF, 0xFF, 
  0xFF, 0x70, 0xAF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x06, 0xFF, 0xFF, 0xFF, 
  0xFF, 0x90, 0x06, 0xFF, 0xFF, 0xFF, 0xFF, 0x60, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x0B, 0xFF, 0xFE, 0xEF, 0xFF, 0xD0, 0x0C, 0xFF, 
  0xFF, 0xFF, 0xD0, 0x00, 0xCF, 0xFF, 0xFF, 0xFF, 0xFF, 0x90, 0x0F, 0xFF, 
  0xDE, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0xAF, 0xFF, 0xFF, 0x70, 0x00, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x9F, 0xFF, 0xFF, 0xDD, 0x60, 0xAF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0x2F, 0xFF, 0xFA, 0xAF, 0xFF, 0xF4, 0x2F, 0xFF, 
  0xFD, 0xEF, 0xFF, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x4F, 0xFF, 0xC0, 0x0C, 0xFF, 0xF6, 0x0A, 0xFF, 0xBF, 0xFF, 0xD0, 0x00, 
  0x7F, 0xA4, 0x02, 0xCF, 0xFF, 0xD0, 0x0A, 0x80, 0x00, 0x6F, 0xFF, 0xF0, 
  0x00, 0x04, 0xFF, 0xCF, 0xFF, 0x70, 0x06, 0xFF, 0xF0, 0x00, 0x00, 0x00, 
  0x07, 0xFF, 0xFF, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xF9, 
  0x9F, 0xFF, 0x60, 0x06, 0xFF, 0xFA, 0xAF, 0xFF, 0x90, 0x0A, 0xFF, 0xF9, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAF, 0xFF, 0x40, 0x04, 
  0xFF, 0xFB, 0x06, 0xA2, 0x0F, 0xFF, 0xD0, 0x00, 0x02, 0x00, 0x00, 0x0F, 
  0xFF, 0xF0, 0x00, 0x00, 0x00, 0x0D, 0xFF, 0xF0, 0x00, 0x0C, 0xFF, 0x6F, 
  0xFF, 0x70, 0x07, 0xFF, 0xD0, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xF2, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0xFF, 0xF0, 0xAF, 0xFF, 0x20, 0x00, 
  0xFF, 0xFA, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xDF, 0xFF, 0x00, 0x00, 0xFF, 0xFD, 0x00, 0x00, 
  0x0F, 0xFF, 0xD0, 0x00, 0x00, 0x00, 0x00, 0x0D, 0xFF, 0xF0, 0x00, 0x00, 
  0x00, 0x7F, 0xFF, 0xC0, 0x00, 0x7F, 0xFD, 0x0F, 0xFF, 0x70, 0x0A, 0xFF, 
  0xC0, 0x00, 0x00, 0x00, 0x8F, 0xFF, 0x76, 0xAA, 0x80, 0x00, 0x00, 0x00, 
  0x00, 0x8F, 0xFF, 0x90, 0x7F, 0xFF, 0xC2, 0x0A, 0xFF, 0xF4, 0xFF, 0xFD, 
  0x00, 0x00, 0xDF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0xFF, 0xFD, 0x00, 0x00, 0xDF, 0xFF, 0x00, 0x00, 0x0F, 0xFF, 0xD0, 0x00, 
  0x00, 0x00, 0x00, 0x2F, 0xFF, 0xD0, 0x00, 0x6D, 0xDF, 0xFF, 0xFE, 0x20, 
  0x00, 0xFF, 0xF6, 0x0F, 0xFF, 0x70, 0x0A, 0xFF, 0xFF, 0xFD, 0xA2, 0x00, 
  0xCF, 0xFF, 0xDF, 0xFF, 0xFF, 0x60, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 
  0x0D, 0xFF, 0xFF, 0xEF, 0xFF, 0x90, 0xFF, 0xFF, 0x00, 0x00, 0xEF, 0xFF, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFD, 0x00, 0x00, 
  0xDF, 0xFF, 0x00, 0x00, 0x0F, 0xFF, 0xD0, 0x00, 0x00, 0x00, 0x00, 0xBF, 
  0xFF, 0x90, 0x00, 0x7F, 0xFF, 0xFF, 0xF4, 0x00, 0x0A, 0xFF, 0xC0, 0x0F, 
  0xFF, 0x70, 0x0C, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0xEF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xF4, 0x00, 0x00, 0x09, 0xFF, 0xFA, 0x00, 0x02, 0xCF, 0xFF, 0xFF, 
  0xFC, 0x00, 0xCF, 0xFF, 0xC2, 0x0A, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xFF, 0xFD, 0x00, 0x00, 0xDF, 0xFF, 0x00, 0x00, 
  0x0F, 0xFF, 0xD0, 0x00, 0x00, 0x00, 0x09, 0xFF, 0xFD, 0x00, 0x00, 0x7F, 
  0xFF, 0xFF, 0xFF, 0x90, 0x2F, 0xFF, 0x20, 0x0F, 0xFF, 0x70, 0x0B, 0xDD, 
  0xDF, 0xFF, 0xFF, 0xF4, 0xFF, 0xFF, 0xC2, 0x2C, 0xFF, 0xFC, 0x00, 0x00, 
  0x0F, 0xFF, 0xF2, 0x00, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x4F, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0xFF, 0xFD, 0x00, 0x00, 0xDF, 0xFF, 0x00, 0x00, 0x0F, 0xFF, 0xD0, 0x00, 
  0x00, 0x00, 0x9F, 0xFF, 0xF4, 0x00, 0x00, 0x00, 0x00, 0x8E, 0xFF, 0xF7, 
  0xCF, 0xFF, 0xDD, 0xDF, 0xFF, 0xED, 0x00, 0x00, 0x00, 0x4E, 0xFF, 0xFA, 
  0xFF, 0xFE, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x9F, 0xFF, 0xA0, 0x00, 
  0x8F, 0xFF, 0xC4, 0x9F, 0xFF, 0xFA, 0x06, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDF, 0xFF, 0x00, 0x00, 
  0xFF, 0xFD, 0x00, 0x00, 0x0F, 0xFF, 0xD0, 0x00, 0x00, 0x09, 0xFF, 0xFF, 
  0x60, 0x00, 0x00, 0x00, 0x00, 0x04, 0xFF, 0xFC, 0xDF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0x00, 0x00, 0x00, 0x04, 0xFF, 0xFD, 0xFF, 0xFD, 0x00, 0x00, 
  0xDF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x20, 0x00, 0xEF, 0xFF, 0x20, 0x02, 
  0xFF, 0xFF, 0x00, 0x07, 0xAA, 0x76, 0xFF, 0xFA, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xBF, 0xFF, 0x40, 0x04, 0xFF, 0xFB, 0x00, 0x00, 
  0x0F, 0xFF, 0xD0, 0x00, 0x00, 0xCF, 0xFF, 0xF6, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x02, 0xFF, 0xFD, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 
  0x00, 0x02, 0xFF, 0xFC, 0xCF, 0xFF, 0x20, 0x00, 0xEF, 0xFF, 0x00, 0x09, 
  0xFF, 0xFA, 0x00, 0x00, 0xFF, 0xFD, 0x00, 0x00, 0xDF, 0xFF, 0x00, 0x00, 
  0x00, 0x2E, 0xFF, 0xF2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x4F, 0xFF, 0xC0, 0x0C, 0xFF, 0xF6, 0x00, 0x00, 0x0F, 0xFF, 0xD0, 0x00, 
  0x2C, 0xFF, 0xFD, 0x20, 0x00, 0x00, 0x0B, 0x60, 0x00, 0x2C, 0xFF, 0xFB, 
  0x00, 0x00, 0x00, 0x0F, 0xFF, 0x70, 0x07, 0x00, 0x00, 0x2C, 0xFF, 0xF9, 
  0x8F, 0xFF, 0xA0, 0x07, 0xFF, 0xFB, 0x00, 0x2F, 0xFF, 0xF2, 0x00, 0x00, 
  0xFF, 0xFF, 0x20, 0x02, 0xFF, 0xFE, 0x00, 0x00, 0x07, 0xEF, 0xFF, 0xA0, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFA, 0x0C, 0xFF, 0xFE, 0xEF, 
  0xFF, 0xC0, 0x00, 0x00, 0x0F, 0xFF, 0xD0, 0x00, 0xEF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xF7, 0x6F, 0xFF, 0xDD, 0xFF, 0xFF, 0xF4, 0x00, 0x00, 0x00, 0x0F, 
  0xFF, 0x70, 0x6F, 0xFD, 0xDD, 0xFF, 0xFF, 0xE0, 0x0D, 0xFF, 0xFE, 0xDF, 
  0xFF, 0xF4, 0x00, 0xAF, 0xFF, 0xA0, 0x00, 0x00, 0xAF, 0xFF, 0xFA, 0xAF, 
  0xFF, 0xF9, 0x09, 0xDD, 0xFF, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0xFF, 0xFF, 0x02, 0xEF, 0xFF, 0xFF, 0xFE, 0x20, 0x00, 0x00, 
  0x0F, 0xFF, 0xD0, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0xAF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0x60, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0x70, 0xAF, 0xFF, 
  0xFF, 0xFF, 0xFD, 0x20, 0x02, 0xEF, 0xFF, 0xFF, 0xFF, 0x60, 0x02, 0xFF, 
  0xFF, 0x20, 0x00, 0x00, 0x0C, 0xFF, 0xFF, 0xFF, 0xFF, 0xB0, 0x0A, 0xFF, 
  0xFF, 0xFF, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 
  0x00, 0x0A, 0xEF, 0xFE, 0xA0, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xD0, 0x00, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0x08, 0xCD, 0xFF, 0xDC, 0x92, 0x00, 
  0x00, 0x00, 0x00, 0x0F, 0xFF, 0x70, 0x6B, 0xDF, 0xFF, 0xFC, 0x80, 0x00, 
  0x00, 0x09, 0xDF, 0xFF, 0xB2, 0x00, 0x0A, 0xFF, 0xFA, 0x00, 0x00, 0x00, 
  0x00, 0x7C, 0xFF, 0xFF, 0xC7, 0x00, 0x0A, 0xFF, 0xFC, 0xA2, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFA
};

const unsigned char timeAdjustLabel[380] PROGMEM = {
  0xFF, 0xFF, 0xF7, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x0E, 0xF0, 0x00, 0x00, 0x06, 0xF0, 0x8F, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0xA0, 0x00, 0x00, 0x06, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x02, 0x60, 0x00, 0xF7, 0x00, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x2F, 0xF6, 0x00, 0x00, 0x07, 0xF0, 0x28, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x07, 0xF0, 0x00, 0x00, 0x4F, 0x20, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x02, 0xF4, 0x00, 0xF7, 0x00, 0xF6, 0x6F, 0xEF, 0xBE, 0xFA, 
  0x00, 0x4F, 0xF7, 0x00, 0x00, 0x9F, 0xEB, 0x00, 0x6F, 0xFE, 0xF0, 0x6F, 
  0x0F, 0x60, 0x6F, 0x0C, 0xFE, 0x0F, 0xFF, 0x90, 0x00, 0xBB, 0x6F, 0xEF, 
  0xAF, 0xFA, 0x09, 0xFF, 0xC0, 0xBC, 0x00, 0xF7, 0x00, 0xF7, 0x7F, 0x87, 
  0xFA, 0x7F, 0x20, 0xDA, 0x9F, 0x00, 0x00, 0xDC, 0xBE, 0x00, 0xFB, 0x0B, 
  0xF0, 0x7F, 0x0F, 0x70, 0x7F, 0x4F, 0x24, 0x07, 0xF0, 0x00, 0x00, 0xF7, 
  0x7F, 0x89, 0xF8, 0x7F, 0x2F, 0x80, 0x40, 0x7F, 0x00, 0xF7, 0x00, 0xF7, 
  0x7F, 0x00, 0xF7, 0x0F, 0x70, 0xF4, 0x2F, 0x20, 0x00, 0xF8, 0x7F, 0x44, 
  0xF2, 0x07, 0xF0, 0x7F, 0x0F, 0x70, 0x7F, 0x0F, 0xD6, 0x07, 0xF0, 0x00, 
  0x00, 0xF2, 0x7F, 0x07, 0xF0, 0x0F, 0x7E, 0xFC, 0x20, 0x2F, 0x00, 0xF7, 
  0x00, 0xF7, 0x7F, 0x00, 0xF7, 0x0F, 0x77, 0xFF, 0xFF, 0x60, 0x08, 0xFF, 
  0xFF, 0xA6, 0xF2, 0x07, 0xF0, 0x7F, 0x0F, 0x70, 0x7F, 0x02, 0xCF, 0x27, 
  0xF0, 0x00, 0x00, 0xF0, 0x7F, 0x07, 0xF0, 0x0F, 0x70, 0x9E, 0xF0, 0x0F, 
  0x00, 0xF7, 0x00, 0xF7, 0x7F, 0x00, 0xF7, 0x0F, 0x72, 0xF8, 0x04, 0x00, 
  0x0C, 0xD0, 0x0C, 0xD0, 0xFB, 0x0B, 0xF0, 0x7F, 0x0F, 0xA0, 0xBF, 0x06, 
  0x4F, 0x46, 0xF2, 0x00, 0x00, 0xF4, 0x7F, 0x07, 0xF0, 0x0F, 0x77, 0x08, 
  0xF0, 0x4F, 0x00, 0xF6, 0x00, 0xF6, 0x6F, 0x00, 0xF6, 0x0F, 0x60, 0x7F, 
  0xFD, 0x00, 0x0F, 0x90, 0x07, 0xF0, 0x7F, 0xFC, 0xF0, 0x7F, 0x08, 0xFF, 
  0xCF, 0x4F, 0xFC, 0x00, 0xEF, 0x70, 0x00, 0xE8, 0x6F, 0x06, 0xF0, 0x0F, 
  0x6F, 0xFF, 0x90, 0x8E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAF, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAC, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0xCA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x0A, 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2F, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xF2
};

const unsigned char systemFont[456] PROGMEM= {
  0x00, 0x4F, 0xF6, 0x00, 0x00, 0x4D, 0xF0, 0x00, 0x02, 0xDF, 0xFA, 0x00, 
  0x02, 0xEF, 0xF8, 0x00, 0x00, 0x00, 0xCF, 0x00, 0x00, 0xCF, 0xFF, 0x40, 
  0x00, 0x08, 0xFB, 0x00, 0x00, 0xFF, 0xFF, 0xE0, 0x00, 0x9F, 0xFA, 0x00, 
  0x00, 0x6F, 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xDA, 0xAE, 
  0x00, 0x00, 0x9C, 0xF0, 0x00, 0x00, 0xA2, 0x9F, 0x40, 0x00, 0xA2, 0xBF, 
  0x00, 0x00, 0x08, 0xFF, 0x00, 0x00, 0xD9, 0x00, 0x00, 0x00, 0x7F, 0x70, 
  0x00, 0x00, 0x00, 0x0F, 0xA0, 0x02, 0xF6, 0x6F, 0x20, 0x00, 0xF9, 0xAF, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 
  0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xF4, 0x4F, 0x00, 0x00, 0x07, 
  0xF0, 0x00, 0x00, 0x00, 0x0F, 0x70, 0x00, 0x00, 0x9F, 0x00, 0x00, 0x4F, 
  0xAF, 0x00, 0x00, 0xF7, 0x00, 0x00, 0x00, 0xEA, 0x00, 0x00, 0x00, 0x00, 
  0x7F, 0x20, 0x02, 0xF4, 0x4F, 0x20, 0x07, 0xF2, 0x2F, 0x40, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 
  0xFF, 0xFF, 0x00, 0x07, 0xF0, 0x0F, 0x70, 0x00, 0x07, 0xF0, 0x00, 0x00, 
  0x00, 0x8F, 0x20, 0x00, 0x02, 0xDD, 0x00, 0x00, 0xEB, 0x6F, 0x00, 0x00, 
  0xFF, 0xF8, 0x00, 0x02, 0xFF, 0xFA, 0x00, 0x00, 0x00, 0xDC, 0x00, 0x00, 
  0xBF, 0xF9, 0x00, 0x07, 0xF0, 0x0F, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0xF0, 0x00, 0xF0, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 
  0x07, 0xF0, 0x0F, 0x70, 0x00, 0x07, 0xF0, 0x00, 0x00, 0x02, 0xFB, 0x00, 
  0x00, 0x9F, 0xF8, 0x00, 0x0A, 0xD0, 0x6F, 0x00, 0x00, 0x02, 0xAF, 0x40, 
  0x07, 0xF7, 0x7F, 0x40, 0x00, 0x04, 0xF4, 0x00, 0x00, 0xEC, 0xDE, 0x00, 
  0x02, 0xF8, 0x8F, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xF0, 
  0xF0, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xF6, 0x6F, 
  0x00, 0x00, 0x07, 0xF0, 0x00, 0x00, 0x0D, 0xD0, 0x00, 0x00, 0x00, 0x8F, 
  0x40, 0x0F, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x0F, 0x70, 0x07, 0xF0, 0x0F, 
  0x70, 0x00, 0x0C, 0xD0, 0x00, 0x07, 0xF0, 0x2F, 0x70, 0x00, 0x9F, 0xFF, 
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x00, 
  0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xDA, 0xAE, 0x00, 0x00, 0x07, 
  0xF0, 0x00, 0x00, 0xCE, 0x20, 0x00, 0x02, 0x60, 0x7F, 0x60, 0x00, 0x00, 
  0x6F, 0x00, 0x02, 0x60, 0xAF, 0x20, 0x00, 0xF9, 0x7F, 0x20, 0x00, 0x2F, 
  0x80, 0x00, 0x04, 0xF7, 0x7F, 0x40, 0x00, 0x06, 0xDC, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 
  0xFF, 0xFF, 0x6B, 0x00, 0x6F, 0xF6, 0x00, 0x00, 0x06, 0xF0, 0x00, 0x09, 
  0xFF, 0xFF, 0x90, 0x07, 0xFF, 0xF9, 0x00, 0x00, 0x00, 0x4F, 0x00, 0x08, 
  0xFF, 0xF7, 0x00, 0x00, 0x7F, 0xF8, 0x00, 0x00, 0xAF, 0x00, 0x00, 0x00, 
  0xAF, 0xF9, 0x00, 0x00, 0xDF, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x9F
};
  
const unsigned char barLabel[49] PROGMEM = {
  0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0xF7, 0x09, 0xF0, 0x00, 0x00, 
  0x00, 0x00, 0xF7, 0x2A, 0xF0, 0xCF, 0xB0, 0x6F, 0xFB, 0xFF, 0xFF, 0x80, 
  0x44, 0xF2, 0x7F, 0xA2, 0xF7, 0x09, 0xF0, 0xAF, 0xF7, 0x7F, 0x00, 0xF7, 
  0x09, 0xF4, 0xF4, 0xF7, 0x7F, 0x00, 0xFF, 0xFE, 0x70, 0xDF, 0xF6, 0x6F,
  0x00
};
    
/** Initializer
 */
void OLED::begin() {
  unsigned char dataBuffer[17];
  int i, j;
  
  //Initialize the I2C interface
  twi_init();
  twi_setAddress(DISPLAY_ADDRESS);

  //Initialize the SSD1327
  delay(100);
  sendCommand(0xFD); // Unlock OLED driver IC MCU interface from entering command. i.e: Accept commands
  sendCommand(0x12);
  sendCommand(0xAE); // Set display off
  delay(1);
  sendCommand(0xA8); // set multiplex ratio
  sendCommand(0x5F); // 96
  sendCommand(0xA1); // set display start line
  delay(1);
  sendCommand(0x00);
  sendCommand(0xA2); // set display offset
  sendCommand(0x60);
  delay(1);
  sendCommand(0xA0); // set remap
  sendCommand(0x46);
  sendCommand(0xAB); // set vdd internal
  delay(1);
  sendCommand(0x01); //
  sendCommand(0x81); // set contrast
  sendCommand(0x53); // 100 nit
  delay(1);
  sendCommand(0xB1); // Set Phase Length
  sendCommand(0X51); //
  sendCommand(0xB3); // Set Display Clock Divide Ratio/Oscillator Frequency
  delay(1);
  sendCommand(0x01);
  sendCommand(0xB9); //
  sendCommand(0xBC); // set pre_charge voltage/VCOMH
  delay(1);
  sendCommand(0x08); // (0x08);
  sendCommand(0xBE); // set VCOMH
  sendCommand(0X07); // (0x07);
  delay(1);
  sendCommand(0xB6); // Set second pre-charge period
  sendCommand(0x01); //
  sendCommand(0xD5); // enable second precharge and enternal vsl
  sendCommand(0X62); // (0x62);
  delay(1);
  sendCommand(0xA4); // Set Normal Display Mode
  sendCommand(0x2E); // Deactivate Scroll
  delay(100);
  sendCommand(0xA0); // remap to horizontal mode
  sendCommand(0x42);

  // Zero out all the pixels
  drawRect(0, 0, 96, 96);
  for(i=0; i < 288; i++) { 
    dataBuffer[0] = DATA_MODE;
    for(j=0; j < 16; j++) {
      dataBuffer[j+1] = COLOR_BLACK;           
    }
    
    twi_writeTo(DISPLAY_ADDRESS, dataBuffer, j+1, 0, 0);  
    delayMicroseconds(300);
  }
  
  // Draw the divider
  drawRect(44, 10, 76, 1);
  for(i=0; i < 3; i++) {        
    dataBuffer[0] = DATA_MODE;
    for(j=0; j < 16; j++) {
      dataBuffer[j+1] = COLOR_DIVIDER;           
    }
    
    twi_writeTo(DISPLAY_ADDRESS, dataBuffer, j+1, 0, 0);  
    delayMicroseconds(300);
  }
  
  // Draw the Bar Label
  drawRect(51, 10, 14, 7);
  for(i=0; i < 3; i++) { 
    dataBuffer[0] = DATA_MODE;
    for(j=0; j < 16; j++) {
      dataBuffer[j+1] = pgm_read_byte(&barLabel[(i * 16) + j]);  
      if((i * 16) + j == 49) break;           
    }
    
    twi_writeTo(DISPLAY_ADDRESS, dataBuffer, j+1, 0, 0);  
    delayMicroseconds(300);
  }

  // Draw the Time Adjust label
  drawRect(63, 10, 76, 10);
  for(i=0; i < 24; i++) { 
    dataBuffer[0] = DATA_MODE;
    for(j=0; j < 16; j++) {
      dataBuffer[j+1] = pgm_read_byte(&timeAdjustLabel[(i * 16) + j]);             
    }
    
    twi_writeTo(DISPLAY_ADDRESS, dataBuffer, j+1, 0, 0);  
    delayMicroseconds(300);
    if((i * 16) + j > 380) break;
  }

  sendCommand(0xAF); // Switch on display
}

/** Send a command to the display
 */
void OLED::sendCommand(unsigned char command) {
  unsigned char commandBuffer[] = {COMMAND_MODE, command};
  twi_writeTo(DISPLAY_ADDRESS, commandBuffer, 2, 0, 0);
}

/** Draw the dirty rectangle to update
 *  Note: the left and width values round to the nearest 2 pixels
 *  @param: top - origin of the rectangle from the top in pixels 
 *  @param: left - origin of the rectangle from the left in pixels
 *  @param: width - bottom corner width from origin in pixels
 *  @param: height - bottom corner height from the origin in pixels
 */
void OLED::drawRect(int top, int left, int width, int height) {
  // Row Address
  sendCommand(SET_ROW); // Set Row Address - 1 pixel per row 
  sendCommand(DRIVER_ROW_OFFSET + top); // Start 
  sendCommand(DRIVER_ROW_OFFSET + top + height - 1); // End
   
  // Column Address
  sendCommand(SET_COLUMN); // Set Column Address - Each Column has 2 pixels(segments) and a 16 pixel column offset
  sendCommand(DRIVER_COLUMN_OFFSET + (left/2)); // Start 
  sendCommand(DRIVER_COLUMN_OFFSET + ((left + width)/2) - 1); // End
}

/** Mark the data as dirty
 *  @param: dirtyId - the Id to en-dirty-en
 */
void OLED::pushToDirtyList(int dirtyId) {
  if (dirtyIndex == DIRTY_LIST_LENGTH) {
    dirtyIndex = 0;
  }
  dirtyViewportList[dirtyIndex] = dirtyId;
  dirtyIndex++;
}

/** Write the pixel map for dynamic system numbers and symbols
 *  @param: viewport - the viewport to write
 *  @param: systemViewportWidth - the system viewport width
 *  @param: systemViewportOffset - the offest in pixels/2 to draw the font
 *  @param: systemFontOffset - the location to copy from in the font map in pixels/2
 *  @param: characterWidth - the width of character in pixels/2 to copy from the font's pixel map
 */
void OLED::pushToSystemBitmap(unsigned char *viewport, int systemViewportWidth, int systemViewportOffset, int systemFontOffset, int characterWidth){
    for(row = 0; row < systemViewportHeight; row++) {
     for(column = 0; column < characterWidth; column++) {
       viewport[(row * systemViewportWidth) + column + systemViewportOffset] = pgm_read_byte(&systemFont[(row * systemFontWidth) + column + systemFontOffset]);
     }
   }
}

/** Write the pixel map for the larger tempo font
 *  @param: tempoViewportOffset - the offest in pixels/2 to draw the font
 *  @param: tempoFontOffset - the location to copy from in the font map in pixels/2
 *  @param: characterWidth - the width of character in pixels/2 to copy from the font's pixel map
 */
void OLED::pushToTempoBitmap(int tempoViewportOffset, int tempoFontOffset, int characterWidth){
    for(row = 0; row < tempoViewportHeight; row++) {
     for(column = 0; column < characterWidth; column++) {
       tempoViewport[(row * tempoViewportWidth)+ column + tempoViewportOffset] = pgm_read_byte(&tempoFont[(row * tempoFontWidth) + column + tempoFontOffset]);
     }
   }  
}

/** Update the mode to internal/external
 *  @param: state - 0 or 1 
 */
void OLED::setIntExt(int state){
  if(state = 1) {
    memcpy(intExtViewport, intLabel, intExtViewportLength);
  } else {
    memcpy(intExtViewport, extLabel, intExtViewportLength);
  }

  pushToDirtyList(DIRTY_INT_EXT);
}

/** Give feedback that the tap button is pressed and that 
 *  we're listening for a new tempo - count 1 bar with a user tapping a quarter note
 *  @param: state - a number between 0-3
 */
void OLED::setTapState(int state){
  switch(state){
    case 0:
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 0, 40, 4);
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 5, 40, 4);
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 10, 40, 4);
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 15, 40, 4);
      break;
    case 1:
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 0, 52, 4);
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 5, 48, 4);
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 10, 48, 4);
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 15, 48, 4);
      break;
    case 2:
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 0, 52, 4);
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 5, 52, 4);
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 10, 48, 4);
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 15, 48, 4);
      break;
    case 3:
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 0, 52, 4);
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 5, 52, 4);
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 10, 52, 4);
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 15, 48, 4);
      break;
    case 4:
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 0, 52, 4);
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 5, 52, 4);
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 10, 52, 4);
      pushToSystemBitmap(tapCounterViewport, tapCounterViewportWidth, 15, 52, 4);
      break;
  }

  pushToDirtyList(DIRTY_TAP);
}

/** Extract the digits from the tempo value and update the tempo bitmap
 *  The text should always be left justified
 *  @param: tempo - the tempo float value eg 120.0 
 */
void OLED::setTempo(double tempo) {
  int integralTempo = floor((tempo*10));
  memset(tempoViewport, COLOR_BLACK, tempoViewportLength);
  
  if(tempo >= 100) {
    pushToTempoBitmap(22, getTempoCharOffset(integralTempo), 6); //fractional
    pushToTempoBitmap(19, 66, 2); // add decimal point
    integralTempo /= 10;
    pushToTempoBitmap(12, getTempoCharOffset(integralTempo), 6); //ones
    integralTempo /= 10;
    pushToTempoBitmap(6, getTempoCharOffset(integralTempo), 6); //tens
    integralTempo /= 10;
    pushToTempoBitmap(0, getTempoCharOffset(integralTempo), 6); //hundreds
  } else {
    pushToTempoBitmap(16, getTempoCharOffset(integralTempo), 6); //fractional
    pushToTempoBitmap(13, 66, 2); // add decimal point
    integralTempo /= 10;
    pushToTempoBitmap(6, getTempoCharOffset(integralTempo), 6); //ones
    integralTempo /= 10;
    pushToTempoBitmap(0, getTempoCharOffset(integralTempo), 6); //tens
  }

  pushToDirtyList(DIRTY_TEMPO);
}

/** Display the level of temop precision 3/4 digits 
 */
void OLED::setTempoPrecision(int state) {
  int divLength, i;
  memset(tempoPrecisionViewport, COLOR_BLACK, tempoPrecisionViewportLength);

  switch(state) {
    // Tempo greater than 100 - 4 digit precision
    case 1:
      divLength = 29;
      break;
    // Tempo less than 100 - 2 digit precision
    case 2:
      divLength = 13;
      break;
    // Tempo less than 100 - 3 digit precision
    case 3:
      divLength = 23;
      break;
    // Tempo greater than 100 - 3 digit precision 
    default:
      divLength = 19;
      break;
  }

  for(i=0; i < divLength; i++) {
    tempoPrecisionViewport[i] = COLOR_DIVIDER;           
  }

  pushToDirtyList(DIRTY_TEMPO_PRECISION);
}

void OLED::setBarCounter(int playhead, int currentBeat){
  pushToSystemBitmap(barCounterViewport, barCounterViewportWidth, 0, 0, 8);
  pushToSystemBitmap(barCounterViewport, barCounterViewportWidth, 10, 0, 8);
  
  pushToDirtyList(DIRTY_BAR_COUNTER);
}

void OLED::setLatency(int channelLatency[]) {
  pushToDirtyList(DIRTY_LATENCY);
}

/** Find a dirty Id or return 0 if nothing's dirty
 *  @return: dirty id or 0 if clean
 */
int OLED::getDirtyId() {
  int i;
  for(i=0; i < DIRTY_LIST_LENGTH; i++) {
    if (dirtyViewportList[i] != 0){
      int dirtyId = dirtyViewportList[i];
      dirtyViewportList[i] = 0;
      return dirtyId;
    }
  }

  return 0;
}

/**Take a snapshot of the current viewport so it doesn't change halfway through rendering
 * @param dirtyid: the dirtyId to stage
 */
void OLED::setRasterViewport(int dirtyId) {
    rasterIndex = 0;

    // Zero the viewport surface
    memset(rasterViewport, 0x00, tempoViewportLength);

    // Copy the dirty surface to the viewport surface
    switch(dirtyId){
      case DIRTY_INT_EXT:
        rasterLength = intExtViewportLength;
        memcpy(rasterViewport, intExtViewport, intExtViewportLength);
        break;
      case DIRTY_TAP:
        rasterLength = tapCounterViewportLength;
        memcpy(rasterViewport, tapCounterViewport, tapCounterViewportLength);
        break;
      case DIRTY_TEMPO:
        rasterLength = tempoViewportLength;
        memcpy(rasterViewport, tempoViewport, tempoViewportLength);
        break;
      case DIRTY_TEMPO_PRECISION:
        rasterLength = tempoPrecisionViewportLength;
        memcpy(rasterViewport, tempoPrecisionViewport, tempoPrecisionViewportLength);
        break;
      case DIRTY_BAR_COUNTER:
        rasterLength = barCounterViewportWidth;
        memcpy(rasterViewport, barCounterViewport, barCounterViewportWidth);
        break;
      case DIRTY_LATENCY:
        rasterLength = latencyViewportWidth;
        memcpy(rasterViewport, latencyViewport, latencyViewportWidth);
        break;
    }
}

/** Draw the viewport - plane sizes must be a multiple of 16
 */
bool OLED::raster(){
  if(rasterIndex >= rasterLength){
    return false;
  }
  
  int i;
  unsigned char dataBuffer[bufferSize + 1];
  dataBuffer[0] = DATA_MODE;
  
  for (i = 0; i < bufferSize; i++) {
     dataBuffer[i+1] = rasterViewport[rasterIndex + i];
  }

  rasterIndex = rasterIndex + i;
  
  twi_writeTo(DISPLAY_ADDRESS, dataBuffer, bufferSize + 1, 0, 0);

  return true;
}

