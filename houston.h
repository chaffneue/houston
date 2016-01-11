#ifndef Houston_h
  #define Houston_h
  #define calulateQuarterNoteTime(bpm) (floor((1/((float)bpm / 60) * 4000000)/4))
  #define calulateMidiClockTime(quarterNoteTime) (floor(quarterNoteTime/24));

  //lcd pins
  #define LCD_RS 6
  #define LCD_RW 7
  #define LCD_ENABLE 8
  #define LCD_D0 20
  #define LCD_D1 21  
  #define LCD_D2 22
  #define LCD_D3 23
  #define LCD_D4 24
  #define LCD_D5 25  
  #define LCD_D6 26
  #define LCD_D7 27

  //tlc5940 pins
  #define TLC_SIN 51
  #define TLC_BLANK 52
  #define TLC_XLAT 11
  #define TLC_GSCLK 9
  #define TLC_SCLK 12
  
  //tempo counter
  #define TEMPO_LAMP_RED_PIN 46
  #define TEMPO_LAMP_GREEN_PIN 47
  #define TEMPO_LAMP_BLUE_PIN 48
  
  //channel buttons
  #define CHANNEL_1_BUTTON_PIN 32
  #define CHANNEL_2_BUTTON_PIN 33
  #define CHANNEL_3_BUTTON_PIN 34
  #define CHANNEL_4_BUTTON_PIN 35
  
  //channel lamps
  #define CHANNEL_1_LAMP_PIN 36
  #define CHANNEL_2_LAMP_PIN 37
  #define CHANNEL_3_LAMP_PIN 38
  #define CHANNEL_4_LAMP_PIN 39
  
  //global buttons
  #define TEMPO_UP_BUTTON_PIN 40
  #define TEMPO_DOWN_BUTTON_PIN 41
  #define COUNT_IN_UP_BUTTON_PIN 42
  #define COUNT_IN_DOWN_BUTTON_PIN 43
  #define STOP_ALL_BUTTON_PIN 45
  #define PLAY_ALL_BUTTON_PIN 44
  
  //startup settings
  #define DEFAULT_TEMPO 120
  #define DEFAULT_COUNT_IN 4
  #define POLL_TIME_TEMPO 150000
  #define POLL_TIME_COUNT_IN 30000
  #define POLL_TIME_TRANSPORT 30000
  #define DOWNBEAT_LAMP_FLASH_OFF 10000
  #define CHANNEL_PENDING_LAMP_ON 250000
  #define CHANNEL_PENDING_LAMP_OFF 90000
  #define MATRIX_ROW_UPDATE_INTERVAL 2500
  
  //globals
  Scheduler scheduler;
  
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial,  midi1);
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, midi2);
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, midi3);
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, midi4);
  
  LiquidCrystal lcd(LCD_RS, LCD_RW, LCD_ENABLE, LCD_D0, LCD_D1, LCD_D2, LCD_D3, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

  //callback declarations
  void dequeueChannel(int channel);
  void startPerformance(int channel, midi::MidiInterface<HardwareSerial> &midiInterface);
  void stopChannel(int channel, midi::MidiInterface<HardwareSerial> &midiInterface);
  void enqueueChannel(int channel, midi::MidiInterface<HardwareSerial> &midiInterface);
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
  void comfortDownbeatCallback();
  void downbeatFlashCompleteCallback();
  void updateTempo();
  void tempoUpInteractionCallback();
  void tempoDownInteractionCallback();
  void countInUpInteractionCallback();
  void countInDownInteractionCallback();
  void countInUpDebounceCallback();
  void countInDownDebounceCallback();
  void stopAllButtonInteractionCallback();
  void stopAllButtonInteractionDebounceCallback();
  void playAllButtonInteractionCallback();
  void playAllButtonInteractionDebounceCallback();
  void midiClockCallback();
  void updateMatrixCallback();

  const int tempoUpPin = TEMPO_UP_BUTTON_PIN;
  const int tempoDownPin = TEMPO_DOWN_BUTTON_PIN;
  const int tempoRedPin = TEMPO_LAMP_RED_PIN;
  const int tempoGreenPin = TEMPO_LAMP_GREEN_PIN;
  const int tempoBluePin = TEMPO_LAMP_BLUE_PIN;
  const int countInUpPin = COUNT_IN_UP_BUTTON_PIN;
  const int countInDownPin = COUNT_IN_DOWN_BUTTON_PIN;
  const int stopPin = STOP_ALL_BUTTON_PIN;
  const int playPin = PLAY_ALL_BUTTON_PIN;
  const int rowOutputs[] = {16, 17, 18, 19, 28, 29, 30, 31};
  const int redOutputs[] = {0, 1, 2, 3, 4, 5, 6, 7};
  const int greenOutputs[] = {27, 26, 25, 24, 23, 22, 21, 20};
  const int blueOutputs[] = {8, 9, 10, 11, 12, 13, 14, 15};
  
  // Matrix Color Palette {R,G,B}
  const int darkBlue[] = {0, 5, 30};
  const int lightBlue[] = {0, 30, 30};
  const int green[] = {0, 1200, 0}; 
  const int red[] = {1800, 0, 0};
  const int white[] = {1000, 300, 100};
  const int yellow[] = {2400, 300, 0};
  const int purple[] = {1500, 0, 1000};

  unsigned int tempo = DEFAULT_TEMPO;
  unsigned int countIn = DEFAULT_COUNT_IN;

  int performanceStarted = 0;
  int channelCountIn[] = {-1,-1,-1,-1};
  int channelCountInLength = 4;
  int channelButtonPins[] = {CHANNEL_1_BUTTON_PIN, CHANNEL_2_BUTTON_PIN, CHANNEL_3_BUTTON_PIN, CHANNEL_4_BUTTON_PIN};
  int channelButtonPinsLength = 4;
  int channelIndicatorPins[] = {CHANNEL_1_LAMP_PIN, CHANNEL_2_LAMP_PIN, CHANNEL_3_LAMP_PIN, CHANNEL_4_LAMP_PIN};
  int channelIndicatorPinsLength = 4;
  int channelBars[] = {-1,-1,-1,-1};
  int channelBarsLength = 4;
  int matrixColumn = 0;
  int matrixRow = 0;
  int playHead = 0;
  int currentGridLocation = 1;
  
  unsigned long clocks = 1;
  unsigned long quarterNotes = 1;
  unsigned long bars = 1;
  unsigned long vis = 0;
  unsigned long quarterNoteTime = calulateQuarterNoteTime(tempo);
  unsigned long midiClockTime = calulateMidiClockTime(quarterNoteTime);

  void printTempo() {
    lcd.setCursor(5, 1);
    lcd.print("   "); 
    lcd.setCursor(5, 1);
    lcd.print(tempo);  
  }
  
  void printCountIn() {
    lcd.setCursor(14, 1);
    lcd.print("  ");
    lcd.setCursor(14, 1);
    lcd.print(countIn);
  }

  void setupPinIo() {
    pinMode(LCD_RS, OUTPUT);
    pinMode(LCD_RW, OUTPUT);
    pinMode(LCD_ENABLE, OUTPUT);
    pinMode(LCD_D0, OUTPUT);
    pinMode(LCD_D1, OUTPUT);  
    pinMode(LCD_D2, OUTPUT);
    pinMode(LCD_D3, OUTPUT);
    pinMode(LCD_D4, OUTPUT);
    pinMode(LCD_D5, OUTPUT);  
    pinMode(LCD_D6, OUTPUT);
    pinMode(LCD_D7, OUTPUT);

    pinMode(TLC_SIN, OUTPUT);
    pinMode(TLC_BLANK, OUTPUT);
    pinMode(TLC_XLAT, OUTPUT);
    pinMode(TLC_GSCLK, OUTPUT);
    pinMode(TLC_SCLK, OUTPUT);
      
    pinMode(TEMPO_LAMP_RED_PIN, OUTPUT);
    pinMode(TEMPO_LAMP_GREEN_PIN, OUTPUT);
    pinMode(TEMPO_LAMP_BLUE_PIN, OUTPUT);
      
    pinMode(CHANNEL_1_BUTTON_PIN, INPUT_PULLUP);
    pinMode(CHANNEL_2_BUTTON_PIN, INPUT_PULLUP);
    pinMode(CHANNEL_3_BUTTON_PIN, INPUT_PULLUP);
    pinMode(CHANNEL_4_BUTTON_PIN, INPUT_PULLUP);
      
    pinMode(CHANNEL_1_LAMP_PIN, OUTPUT);
    pinMode(CHANNEL_2_LAMP_PIN, OUTPUT);
    pinMode(CHANNEL_3_LAMP_PIN, OUTPUT);
    pinMode(CHANNEL_4_LAMP_PIN, OUTPUT);
      
    pinMode(TEMPO_UP_BUTTON_PIN, INPUT_PULLUP);
    pinMode(TEMPO_DOWN_BUTTON_PIN, INPUT_PULLUP);
    pinMode(COUNT_IN_UP_BUTTON_PIN, INPUT_PULLUP);
    pinMode(COUNT_IN_DOWN_BUTTON_PIN, INPUT_PULLUP);
    pinMode(STOP_ALL_BUTTON_PIN, INPUT_PULLUP);
    pinMode(PLAY_ALL_BUTTON_PIN, INPUT_PULLUP);
  }

  void initLcd() {
    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.print("Houston.");
    lcd.setCursor(0, 1);
    lcd.print("BPM: ");
    lcd.print(tempo);
    lcd.print(" CNT: ");
    lcd.print(countIn);
  }

  void initMidi() {
    midi1.begin(1);
    midi2.begin(1);
    midi3.begin(1);
    midi4.begin(1);
  }
  
  void setColor(const int color[], int matrixColumn) {
    Tlc.set(redOutputs[matrixColumn], color[0]);
    Tlc.set(greenOutputs[matrixColumn], color[1]);
    Tlc.set(blueOutputs[matrixColumn], color[2]);
  }

  void initMatrix() {
    Tlc.init(100);
    delay(50);
  }
#endif
