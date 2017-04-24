#include "oled.h"
#include "Arduino.h"
extern "C" {
  #include "Twi.h"
}

// Initialize the viewport rectangles and common interators 
int tempoViewport[tempoViewportLength];
int latencyViewport[latencyViewportLength];
int currentIndex, row, column;
    
/** Initializer
 */
void OLED::begin() {
  const unsigned char blankingData[17] = {DATA_MODE, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  const unsigned char dividerData[17] = {DATA_MODE, 0x66,0x66,0x66,0x66,0x66,0x66, 0x66, 0x66,0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66};
  int i=0;
  
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
  sendCommand(0x81); // set contrasr
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
  delay(1);
  sendCommand(0X62); // (0x62);
  sendCommand(0xA4); // Set Normal Display Mode
  sendCommand(0x2E); // Deactivate Scroll
  delay(100);
  sendCommand(0xA0); // remap to horizontal mode
  sendCommand(0x42);

  // Zero out all the pixels
  for(i=0; i < 288; i++) {        
      twi_writeTo(DISPLAY_ADDRESS, blankingData, 17, 0, 0);  
      delayMicroseconds(300);
  }
  
  sendCommand(0xAF); // Switch on display

  delay(1);

  // Draw the divider
  drawRect(46, 10, 76, 1);
  for(i=0; i < 3; i++) {        
      twi_writeTo(DISPLAY_ADDRESS, dividerData, 17, 0, 0);  
      delayMicroseconds(300);
  }
  
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
void OLED::drawRect(unsigned int top, unsigned int left, unsigned int width, unsigned int height) {
  // Row Address
  sendCommand(SET_ROW); // Set Row Address - 1 pixel per row 
  sendCommand(DRIVER_ROW_OFFSET + top); // Start 
  sendCommand(DRIVER_ROW_OFFSET + top + height - 1); // End
   
  // Column Address
  sendCommand(SET_COLUMN); // Set Column Address - Each Column has 2 pixels(segments) and a 16 pixel column offset
  sendCommand(DRIVER_COLUMN_OFFSET + (left/2)); // Start 
  sendCommand(DRIVER_COLUMN_OFFSET + ((left + width)/2) - 1); // End
}

/** Write the pixel map for the larger tempo font
 *  @param: tempoViewportOffset - the offest in pixels/2 to draw the font
 *  @param: tempoFontOffset - the location to copy from in the font map in pixels/2
 *  @param: characterWidth - the width of character in pixels/2 to copy from the font's pixel map
 */
void OLED::pushToTempoBitmap(int tempoViewportOffset, int tempoFontOffset, int characterWidth){
    for(row = 0; row < tempoViewportHeight; row++) {
     for(column = 0; column < characterWidth; column++) {
       tempoViewport[(row * tempoViewportWidth)+ column + tempoViewportOffset] = tempoFont[(row * tempoFontWidth) + column + tempoFontOffset];
     }
   }  
}

/** Extract the digits from the tempo value and update the tempo bitmap
 *  The text should always be left justified
 *  @param: tempo - the tempo float value eg 120.0 
 */
void OLED::setTempo(double tempo) {
  int integralTempo = floor((tempo*10));
  if(tempo >= 100) {
    pushToTempoBitmap(22, getTempoCharOffset(integralTempo), 6); //fractional
    pushToTempoBitmap(18, 60, 4); // clear decimal point location
    pushToTempoBitmap(19, 66, 2); // add decimal point
    integralTempo /= 10;
    pushToTempoBitmap(12, getTempoCharOffset(integralTempo), 6); //ones
    integralTempo /= 10;
    pushToTempoBitmap(6, getTempoCharOffset(integralTempo), 6); //tens
    integralTempo /= 10;
    pushToTempoBitmap(0, getTempoCharOffset(integralTempo), 6); //hundreds
  } else {
    pushToTempoBitmap(22, 60, 6); //clear the pixels to the right
    pushToTempoBitmap(16, getTempoCharOffset(integralTempo), 6); //fractional
    pushToTempoBitmap(12, 60, 4); // clear decimal point location
    pushToTempoBitmap(13, 66, 2); // add decimal point
    integralTempo /= 10;
    pushToTempoBitmap(6, getTempoCharOffset(integralTempo), 6); //ones
    integralTempo /= 10;
    pushToTempoBitmap(0, getTempoCharOffset(integralTempo), 6); //tens
  }
}

  
void OLED::raster(){
  if(currentIndex == 448){
    //return;
    currentIndex=0;
  }
  
  unsigned char dataBuffer[bufferSize + 1];
  dataBuffer[0] = DATA_MODE;
  int i;
  for (i = 0; i < 16; i++) {
     dataBuffer[i+1] = tempoViewport[i+currentIndex];
  }

  currentIndex = currentIndex + 16;
  
  twi_writeTo(DISPLAY_ADDRESS, dataBuffer, 17, 0, 0);
}

