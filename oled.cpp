#include "oled.h"
#include "Arduino.h"
extern "C" {
  #include "Twi.h"
}

// Initialize the viewport rectangles and common interators
unsigned char intExtViewport[intExtViewportLength];
unsigned char tapCounterViewport[tapCounterViewportLength]; 
unsigned char tempoViewport[tempoViewportLength];
unsigned char barCounterViewport[barCounterViewportLength];
unsigned char latencyViewport[latencyViewportLength];
unsigned char rasterViewport[tempoViewportLength]; // double buffer is sized to the largest viewport

int rasterIndex, rasterLength, dirtyIndex, currentDirtyIndex, row, column;
    
/** Initializer
 */
void OLED::begin() {
  unsigned char blankingData[17] = {DATA_MODE, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  unsigned char dividerData[17] = {DATA_MODE, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44};
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
      twi_writeTo(DISPLAY_ADDRESS, blankingData, 17, 0, 0);  
      delayMicroseconds(300);
  }

  // Draw the divider
  drawRect(44, 10, 76, 1);
  for(i=0; i < 3; i++) {        
      twi_writeTo(DISPLAY_ADDRESS, dividerData, 17, 0, 0);  
      delayMicroseconds(300);
  }
  
  // Draw the Bar Label
  drawRect(51, 10, 14, 7);
  for(i=0; i < 3; i++) { 
    dataBuffer[0] = DATA_MODE;
    for(j=0; j < 16; j++) {
      dataBuffer[j+1] = barLabel[(i * 16) + j];  
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
      dataBuffer[j+1] = timeAdjustLabel[(i * 16) + j];             
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
 *  @param: tempoViewportOffset - the offest in pixels/2 to draw the font
 *  @param: tempoFontOffset - the location to copy from in the font map in pixels/2
 *  @param: characterWidth - the width of character in pixels/2 to copy from the font's pixel map
 */
void OLED::pushToSystemBitmap(unsigned char *viewport, int systemViewportWidth, int systemViewportOffset, int systemFontOffset, int characterWidth){
    for(row = 0; row < systemViewportHeight; row++) {
     for(column = 0; column < characterWidth; column++) {
       viewport[(row * systemViewportWidth) + column + systemViewportOffset] = systemFont[(row * systemFontWidth) + column + systemFontOffset];
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
       tempoViewport[(row * tempoViewportWidth)+ column + tempoViewportOffset] = tempoFont[(row * tempoFontWidth) + column + tempoFontOffset];
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

  pushToDirtyList(DIRTY_TEMPO);
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
 *  @return: diryt id or 0 if clean
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

/**Limit the renderer to draw the current viewport once 
 * @param length: the size of the viewport array to draw
 */
void OLED::setRasterLength(int length) {
  rasterLength = length;
}

/**Take a snapshot of the current viewport so it doesn't change halfway through rendering
 * @param dirtyid: the dirtyId to stage
 */
void OLED::setRasterViewport(int dirtyId) {
    rasterIndex = 0;
    
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
        memcpy(&rasterViewport, &tempoViewport, tempoViewportLength);
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

/** Draw the viewport
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
  
  twi_writeTo(DISPLAY_ADDRESS, dataBuffer, i + 1, 0, 0);

  return true;
}

