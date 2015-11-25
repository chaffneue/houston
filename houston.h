#ifndef Houston_h
#define Houston_h

#define length(inputArray) (sizeof(inputArray) / sizeof((inputArray)[0]))
#define calulateQuarterNoteTime(bpm) (floor((1/((float)bpm / 60) * 4000)/4))
#define calulateMidiClockTime(quarterNoteTime) (floor(quarterNoteTime/24));

//tempo counter
#define TEMPO_LAMP_RED_PIN 24
#define TEMPO_LAMP_GREEN_PIN 23
#define TEMPO_LAMP_BLUE_PIN 22

//channel buttons
#define CHANNEL_1_BUTTON_PIN 46
#define CHANNEL_2_BUTTON_PIN 48
#define CHANNEL_3_BUTTON_PIN 50
#define CHANNEL_4_BUTTON_PIN 52

//channel lamps
#define CHANNEL_1_LAMP_PIN 47
#define CHANNEL_2_LAMP_PIN 49
#define CHANNEL_3_LAMP_PIN 51
#define CHANNEL_4_LAMP_PIN 53

//global buttons
#define TEMPO_UP_BUTTON_PIN 38
#define TEMPO_DOWN_BUTTON_PIN 40
#define COUNT_IN_UP_BUTTON_PIN 32
#define COUNT_IN_DOWN_BUTTON_PIN 30
#define STOP_ALL_BUTTON_PIN 36
#define PLAY_ALL_BUTTON_PIN 27

//startup settings
#define DEFAULT_TEMPO 120
#define POLL_TIME_TEMPO 150
#define POLL_TIME_COUNT_IN 30
#define POLL_TIME_TRANSPORT 30
#define POLL_TIME_CHANNEL 30

#endif