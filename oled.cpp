#include "oled.h"
#include "Arduino.h"
extern "C" {
  #include "Twi.h"
}



//int rasterStartByte = 0
//int rasterEndByte = 2288
//int 
unsigned char grayH= 0xF0;
unsigned char grayL= 0x0F;
    
/** Initializer
 */
void OLED::begin() {
  //Initialize the I2C interface
  twi_init();
  twi_setFrequency(I2C_FREQUENCY);
  twi_setAddress(DISPLAY_ADDRESS);
 
  //Initialize the SSD1327
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
}

/** Send a command to the display
 */
void OLED::sendCommand(unsigned char command) {
  unsigned char commandBuffer[] = {COMMAND_MODE, command};
  
  twi_writeTo(DISPLAY_ADDRESS, commandBuffer, 2, 0, 0);

}

/**
 * Send data to the display 
 */
void OLED::sendData(unsigned char data) {
  unsigned char dataBuffer[] = {DATA_MODE, random(0,255), random(0,255), random(0,255), random(0,255), random(0,255), random(0,255), random(0,255), random(0,255), random(0,255), random(0,255), random(0,255), random(0,255), random(0,255), random(0,255), random(0,255)};

  twi_writeTo(DISPLAY_ADDRESS, dataBuffer, 16, 0, 0);
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
  sendCommand(ROW_OFFSET + top); // Start 
  sendCommand(ROW_OFFSET + top + height - 1); // End 
   
  // Column Address
  sendCommand(SET_COLUMN); // Set Column Address - Each Column has 2 pixels(segments) and a 16 pixel column offset
  sendCommand(COLUMN_OFFSET + (left/2)); // Start 
  sendCommand(COLUMN_OFFSET + ((left + width)/2) - 1); // End
}

int currentIndex = 0;
int rectTop = 25;
int rectLeft = 10;
int rectWidth = 56;
int rectHeight = 16;
int bufferSize = 16;

int viewport[448];

/**
 * Initialize the OLED with Houston's dashboard
 */
void OLED::initialize() {
  int i, j;
  const unsigned char dataBuffer[17] = {DATA_MODE, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  
  for(i=0; i < 1000; i++) {        
      twi_writeTo(DISPLAY_ADDRESS, dataBuffer, 17, 0, 0);  
      delayMicroseconds(200);
  }

  sendCommand(0xAF); // Switch on display
}

void OLED::pushToTempoBitmap(int number, int numberPosition){
    int row, column;
    int font_width = 62;
    int character_width = 6;
    int viewport_width = 28;
    int viewport_height = 16;
    int viewport_offset = number * character_width;
    int tempo_char_offset = (numberPosition * character_width);
    if (numberPosition == 3) {
      tempo_char_offset  = tempo_char_offset + 4;
    }

    for(row = 0; row < viewport_height; row++) {
     for(column = 0; column < character_width; column++) {
       viewport[(row*viewport_width)+column+tempo_char_offset] = tempoFont[(row*font_width)+column+viewport_offset];
     }
   }  
}

void OLED::pushToTempoBitmap(){
    int row, column;
    int font_width = 62;
    int character_width = 2;
    int viewport_width = 28;
    int viewport_height = 16;
    int viewport_offset = 60;
    int tempo_char_offset = 19;

    for(row = 0; row < viewport_height; row++) {
     for(column = 0; column < character_width; column++) {
       viewport[(row*viewport_width)+column+tempo_char_offset] = tempoFont[(row*font_width)+column+viewport_offset];
     }
   }  
}

void OLED::pushToTempoBitmap(char none[]){
    int row, column;
    int font_width = 62;
    int character_width = 2;
    int viewport_width = 28;
    int viewport_height = 16;
    int viewport_offset = 60;
    int tempo_char_offset = 19;

    for(row = 0; row < viewport_height; row++) {
     for(column = 0; column < character_width; column++) {
       viewport[(row*viewport_width)+column+tempo_char_offset] = 0x00;
     }
   }  
}



void OLED::setTempo(double tempo) {
  int integralTempo = floor((tempo*10));
  
  pushToTempoBitmap(integralTempo % 10, 3);
  integralTempo /= 10;
  pushToTempoBitmap(integralTempo % 10, 2);
  integralTempo /= 10;
  pushToTempoBitmap(integralTempo % 10, 1);
  integralTempo /= 10;
  pushToTempoBitmap();
  if(tempo >= 100) {
    pushToTempoBitmap(integralTempo % 10, 0);
  } else {
    pushToTempoBitmap("none", 0);
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
     dataBuffer[i+1] = viewport[i+currentIndex];
  }

  currentIndex = currentIndex + 16;
  
  twi_writeTo(DISPLAY_ADDRESS, dataBuffer, 17, 0, 0);
}

