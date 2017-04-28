/*
Houston Multiple MIDI Master Interface

@author Richard Hoar <richard.hoar@streeme.com>
@see https://github.com/chaffneue/houston
@see houston.ino for changelog

License
---

The MIT License (MIT)

Copyright (c) 2015 Richard Hoar <richard.hoar@streeme.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */
#ifndef Houston_h
  #define Houston_h
  #define calulateQuarterNoteTime(bpm) (floor((1/((float)bpm / 60) * 4000000)/4))
  #define calulateMidiClockTime(quarterNoteTime) (floor(quarterNoteTime/24));
  
  //channel buttons
  #define CHANNEL_1_BUTTON_PIN 23
  #define CHANNEL_2_BUTTON_PIN 24
  #define CHANNEL_3_BUTTON_PIN 25
  #define CHANNEL_4_BUTTON_PIN 26 
  #define CHANNEL_5_BUTTON_PIN 27
  
  //channel lamps
  #define CHANNEL_1_LAMP_PIN 6
  #define CHANNEL_2_LAMP_PIN 7
  #define CHANNEL_3_LAMP_PIN 8
  #define CHANNEL_4_LAMP_PIN 9
  #define CHANNEL_5_LAMP_PIN 10
  
  //global buttons
  #define STOP_ALL_BUTTON_PIN 29
  #define PLAY_ALL_BUTTON_PIN 28
  #define TAP_BUTTON_PIN 3

  //global lamps
  #define TEMPO_LAMP_PIN 30
  #define PLAY_ALL_LAMP_PIN 12
  #define STOP_ALL_LAMP_PIN 11

  //Virtual Serial
  #define VIRTUAL_SERIAL_TX_PIN 46
  #define VIRTUAL_SERIAL_RX_PIN 13

  //I2C
  #define I2C_SDA_PIN 20
  #define I2C_SCL_PIN 21
  
  //Encoder
  #define ENCODER_CLK_PIN 2
  #define ENCODER_DT_PIN 4
  #define ENCODER_BUTTON_PIN 5
  #define ENCODER_DEBOUNCE_TIME 30000
  
  //startup settings
  #define DEFAULT_TEMPO 135.0
  #define DEFAULT_COUNT_IN 1
  #define POLL_TIME_TRANSPORT 15000
  #define DOWNBEAT_LAMP_FLASH_OFF 10000
  #define CHANNEL_PENDING_LAMP_ON 250000
  #define CHANNEL_PENDING_LAMP_OFF 90000
  #define MATRIX_ROW_UPDATE_INTERVAL 2500
  #define DEFAULT_BUTTON_MAX_BRIGHTNESS 100
  #define DEFAULT_BUTTON_MIN_BRIGHTNESS 1
  
  //globals
  Scheduler scheduler;
  
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial,  midi1);
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, midi2);
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, midi3);
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, midi4);
  MIDI_CREATE_INSTANCE(AltSoftSerial, altSerial, midi5);

  //callback declarations
  void dequeueChannel(int channel);
  void startPerformance(int channel, midi::MidiInterface<HardwareSerial> &midiInterface);
  void stopChannel(int channel, midi::MidiInterface<HardwareSerial> &midiInterface);
  void playChannel(int channel, midi::MidiInterface<HardwareSerial> &midiInterface); 
  void enqueueChannel(int channel, midi::MidiInterface<HardwareSerial> &midiInterface);
  void startPerformance(int channel, midi::MidiInterface<HardwareSerial> &midiInterface);
  void stopChannel(int channel, midi::MidiInterface<AltSoftSerial> &midiInterface);
  void playChannel(int channel, midi::MidiInterface<AltSoftSerial> &midiInterface); 
  void enqueueChannel(int channel, midi::MidiInterface<AltSoftSerial> &midiInterface);
  void channel1InteractionCallback();
  void channel1InteractionDebounceCallback();
  void channel1PendingCallback();
  void channel1PendingFlashCompleteCallback();
  void channel2InteractionCallback();
  void channel2InteractionDebounceCallback();
  void channel2PendingCallback();
  void channel2PendingFlashCompleteCallback();
  void channel3InteractionCallback();
  void channel3InteractionDebounceCallback();
  void channel3PendingCallback();
  void channel3PendingFlashCompleteCallback();
  void channel4InteractionCallback();
  void channel4InteractionDebounceCallback();
  void channel4PendingCallback();
  void channel4PendingFlashCompleteCallback();
  void channel5InteractionCallback();
  void channel5InteractionDebounceCallback();
  void channel5PendingCallback();
  void channel5PendingFlashCompleteCallback();
  void comfortDownbeatCallback();
  void downbeatFlashCompleteCallback();
  void encoderDebounceCallback();
  void tapDebounceCallback();
  void updateTempo();
  void stopAllButtonInteractionCallback();
  void stopAllButtonInteractionDebounceCallback();
  void playAllButtonInteractionCallback();
  void playAllButtonInteractionDebounceCallback();
  void midiClockCallback();
  void dirtyEventWatcherCallback();
  void oledRasterCallback();

  Input<STOP_ALL_BUTTON_PIN> stopPin;
  Input<PLAY_ALL_BUTTON_PIN> playPin;
  Input<CHANNEL_1_BUTTON_PIN> channel1ButtonPin;
  Input<CHANNEL_2_BUTTON_PIN> channel2ButtonPin;
  Input<CHANNEL_3_BUTTON_PIN> channel3ButtonPin;
  Input<CHANNEL_4_BUTTON_PIN> channel4ButtonPin;
  Input<CHANNEL_5_BUTTON_PIN> channel5ButtonPin;

  Output<TEMPO_LAMP_PIN> tempoRedPin;

  Output<I2C_SDA_PIN> i2c_sda;
  Output<I2C_SCL_PIN> i2c_scl;

  Input<ENCODER_CLK_PIN> encoderClkPin;
  Input<ENCODER_DT_PIN> encoderDtPin;
  Input<ENCODER_BUTTON_PIN> encoderSwPin;
  Input<TAP_BUTTON_PIN> tapPin;

  double tempo = DEFAULT_TEMPO;
  unsigned int countIn = DEFAULT_COUNT_IN;

  int performanceStarted = 0;
  int channelCountIn[] = {-1,-1,-1,-1,-1};
  int channelCountInLength = 5;
  int channelBars[] = {-1,-1,-1,-1,-1};
  int channelBarsLength = 5;
  int matrixColumn = 0;
  int matrixRow = 0;
  int playHead = 0;
  int currentGridLocation = 1;
  int quarterNotes = 1;
  int clocks = 1;
  int buttonMaxBrightness = DEFAULT_BUTTON_MAX_BRIGHTNESS;
  int buttonMinBrightness = DEFAULT_BUTTON_MIN_BRIGHTNESS;
  int lastDirtyRectangle;
  
  unsigned long quarterNoteTime = calulateQuarterNoteTime(tempo);
  unsigned long midiClockTime = calulateMidiClockTime(quarterNoteTime);
  const uint8_t encInterruptPin = digitalPinToInterrupt(ENCODER_CLK_PIN);
  const uint8_t tapInterruptPin = digitalPinToInterrupt(TAP_BUTTON_PIN);

  int s;

  /** Register the IO pins 
   */
  void setupPinIo() {
    analogWrite(CHANNEL_1_LAMP_PIN, buttonMinBrightness);
    analogWrite(CHANNEL_2_LAMP_PIN, buttonMinBrightness);
    analogWrite(CHANNEL_3_LAMP_PIN, buttonMinBrightness);
    analogWrite(CHANNEL_4_LAMP_PIN, buttonMinBrightness);
    analogWrite(CHANNEL_5_LAMP_PIN, buttonMinBrightness);
    analogWrite(STOP_ALL_LAMP_PIN, buttonMaxBrightness);
    analogWrite(PLAY_ALL_LAMP_PIN, buttonMinBrightness);
    i2c_sda = true;
    i2c_scl = true;
  }

  /** Start the MIDI interfaces
   */
  void initMidi() {
    midi1.begin(1);
    midi2.begin(1);
    midi3.begin(1);
    midi4.begin(1);
    midi5.begin(1);
  }
#endif
