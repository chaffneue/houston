/*
Houston Multiple MIDI Master Interface

@author Richard Hoar <richard.hoar@streeme.com>
@see https://github.com/chaffneue/houston

Change Log
---
1.0.0 - January 13, 2016
  - Initial Release

1.0.1 - February 2, 2016
  - Minor Code cleanup and integration of official microsecond support in Task Scheduler
  - Added BOM and docblocks

1.0.2 - February 3, 2016
  - Converting digitalwrite abstraction to DirectIO to increase bit banging speed of the main sketch
  - Integrating startNow() functionality in Task Scheduler to prevent cascading tasks on startup

1.0.3 - March 3, 2017
  - Adding fractional tempos to one decimal place
  - Making stop and start behaviour more consistent through the app
  - Adding a song pointer reset to ensure patterns start from the beginning
  - Moving midi start to immediately before the ticks while enqueued so the gear doesn't time out

2.0.0 - April 9, 2017
  - Hardware refresh based on findings over the year
  - New UI layout with new visualization features, tap functionality and an extra channel over USB.
  - Single Board, Hand-solderable SMD layout with an off the shelf oled display
  - Schematics and PCB layout moved to Altium Circuit Maker because of board size constraints with Eagle.
  - Link to hardware: https://workspace.circuitmaker.com/Projects/Details/Chaffneue-Industries/Houston  
   
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
#define ARDUINO_AVR_MEGA2560 1
#include <DirectIO.h>

#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>

#define _TASK_MICRO_RES
#include <TaskScheduler.h>

#include <AltSoftSerial.h>
AltSoftSerial altSerial;

#include "oled.h"
OLED oled = OLED();

#include "Houston.h"

Task midiClock(midiClockTime, -1, &midiClockCallback, &scheduler);

Task comfortDownbeat(quarterNoteTime, -1, &comfortDownbeatCallback, &scheduler, true);
Task downbeatFlashComplete(DOWNBEAT_LAMP_FLASH_OFF, 2, &downbeatFlashCompleteCallback, &scheduler);

Task stopAllButtonInteraction(POLL_TIME_TRANSPORT, -1, &stopAllButtonInteractionCallback, &scheduler, true);
Task stopAllButtonInteractionDebounce(POLL_TIME_TRANSPORT, -1, &stopAllButtonInteractionDebounceCallback, &scheduler);

Task playAllButtonInteraction(POLL_TIME_TRANSPORT, -1, &playAllButtonInteractionCallback, &scheduler, true);
Task playAllButtonInteractionDebounce(POLL_TIME_TRANSPORT, -1, &playAllButtonInteractionDebounceCallback, &scheduler);

Task channel1Interaction(POLL_TIME_TRANSPORT, -1, &channel1InteractionCallback, &scheduler, true);
Task channel1InteractionDebounce(POLL_TIME_TRANSPORT, -1, &channel1InteractionDebounceCallback, &scheduler);
Task channel1Pending(CHANNEL_PENDING_LAMP_ON, -1, &channel1PendingCallback, &scheduler);
Task channel1PendingFlashComplete(CHANNEL_PENDING_LAMP_OFF, 2, &channel1PendingFlashCompleteCallback, &scheduler);

Task channel2Interaction(POLL_TIME_TRANSPORT, -1, &channel2InteractionCallback, &scheduler, true);
Task channel2InteractionDebounce(POLL_TIME_TRANSPORT, -1, &channel2InteractionDebounceCallback, &scheduler);
Task channel2Pending(CHANNEL_PENDING_LAMP_ON, -1, &channel2PendingCallback, &scheduler);
Task channel2PendingFlashComplete(CHANNEL_PENDING_LAMP_OFF, 2, &channel2PendingFlashCompleteCallback, &scheduler);

Task channel3Interaction(POLL_TIME_TRANSPORT, -1, &channel3InteractionCallback, &scheduler, true);
Task channel3InteractionDebounce(POLL_TIME_TRANSPORT, -1, &channel3InteractionDebounceCallback, &scheduler);
Task channel3Pending(CHANNEL_PENDING_LAMP_ON, -1, &channel3PendingCallback, &scheduler);
Task channel3PendingFlashComplete(CHANNEL_PENDING_LAMP_OFF, 2, &channel3PendingFlashCompleteCallback, &scheduler);

Task channel4Interaction(POLL_TIME_TRANSPORT, -1, &channel4InteractionCallback, &scheduler, true);
Task channel4InteractionDebounce(POLL_TIME_TRANSPORT, -1, &channel4InteractionDebounceCallback, &scheduler);
Task channel4Pending(CHANNEL_PENDING_LAMP_ON, -1, &channel4PendingCallback, &scheduler);
Task channel4PendingFlashComplete(CHANNEL_PENDING_LAMP_OFF, 2, &channel4PendingFlashCompleteCallback, &scheduler);

Task channel5Interaction(POLL_TIME_TRANSPORT, -1, &channel5InteractionCallback, &scheduler, true);
Task channel5InteractionDebounce(POLL_TIME_TRANSPORT, -1, &channel5InteractionDebounceCallback, &scheduler);
Task channel5Pending(CHANNEL_PENDING_LAMP_ON, -1, &channel5PendingCallback, &scheduler);
Task channel5PendingFlashComplete(CHANNEL_PENDING_LAMP_OFF, 2, &channel5PendingFlashCompleteCallback, &scheduler);

Task encoderDebounce(ENCODER_DEBOUNCE_TIME, 2, &encoderDebounceCallback, &scheduler, true);
Task tapDebounce(ENCODER_DEBOUNCE_TIME, 2, &tapDebounceCallback, &scheduler, true);

Task printSomething(1000, -1, &printSomethingCallback, &scheduler, true);
Task changeRects(20000, -1, &changeRectsCallback, &scheduler, true);

void changeRectsCallback() {
  /**
  switch(changerectct) {
    case 0:
      //Master Slave Mode
      oled.drawRect(10, 10, 42, 8);
      changeRects.setInterval(25000);
      break;
    case 1:
      //Beat counter
      oled.drawRect(11, 58, 28, 6);
      changeRects.setInterval(12000);
      break;
    case 2:
      //Tempo
      oled.drawRect(25, 10, 56, 16);
      changeRects.setInterval(60000);
      break;
    case 3:
      //Bar
      oled.drawRect(54, 50, 36, 8);
      changeRects.setInterval(22000);
      break;
    case 4:
      //Latency
      oled.drawRect(78, 10, 76, 8);
      changeRects.setInterval(22000);
      break;
    default: 
      changerectct=0;
  }
  changerectct = random(0, 10);
  **/
}

void printSomethingCallback() {
  oled.raster();
}

/** Remove a MIDI channel from the queued state
 * @param: channel - the channel to dequeue
 */
void dequeueChannel(int channel) {
  channelCountIn[channel] = -1;
  channelBars[channel] = -1;
  switch(channel) {
    case 0:
      channel1Pending.disable();
      channel1PendingFlashComplete.disable();
      analogWrite(CHANNEL_1_LAMP_PIN, buttonMinBrightness);
      break;
    case 1:
      channel2Pending.disable();
      channel2PendingFlashComplete.disable();
      analogWrite(CHANNEL_2_LAMP_PIN, buttonMinBrightness);
      break;
    case 2:
      channel3Pending.disable();
      channel3PendingFlashComplete.disable();
      analogWrite(CHANNEL_3_LAMP_PIN, buttonMinBrightness);
      break;
    case 3:
      channel4Pending.disable();
      channel4PendingFlashComplete.disable();
      analogWrite(CHANNEL_4_LAMP_PIN, buttonMinBrightness);
      break;
    case 4:
      channel5Pending.disable();
      channel5PendingFlashComplete.disable();
      analogWrite(CHANNEL_5_LAMP_PIN, buttonMinBrightness);
      break;    
  } 
}

/** Start the performance
 *  @param: channel - the channel initiating the state change 
 *  @param: midiInterface - the midi interface pointer for the channel
 */
void startPerformance(int channel, midi::MidiInterface<HardwareSerial> &midiInterface) {
  performanceStarted = 1;
  playChannel(channel, midiInterface);
  comfortDownbeat.disable();
  analogWrite(PLAY_ALL_LAMP_PIN, buttonMaxBrightness);
  analogWrite(STOP_ALL_LAMP_PIN, buttonMinBrightness);
  channelCountIn[channel] = 0;
  midiClock.restart();
}
void startPerformance(int channel, midi::MidiInterface<AltSoftSerial> &midiInterface) {
  performanceStarted = 1;
  playChannel(channel, midiInterface);
  comfortDownbeat.disable();
  analogWrite(PLAY_ALL_LAMP_PIN, buttonMaxBrightness);
  analogWrite(STOP_ALL_LAMP_PIN, buttonMinBrightness);
  channelCountIn[channel] = 0;
  midiClock.restart();
}

/** Stop a single channel
 *  @param: channel - the channel to stop immediately
 *  @param: midiInterface - the midi interface pointer for the channel
 */
void stopChannel(int channel, midi::MidiInterface<HardwareSerial> &midiInterface) {
  midiInterface.sendRealTime(midi::Stop);
  channelCountIn[channel] = -1;
  channelBars[channel] = -1;
  switch(channel) {
    case 0:
      channel1Pending.disable();
      channel1PendingFlashComplete.disable();
      analogWrite(CHANNEL_1_LAMP_PIN, buttonMinBrightness);
      break;
    case 1:
      channel2Pending.disable();
      channel2PendingFlashComplete.disable();
      analogWrite(CHANNEL_2_LAMP_PIN, buttonMinBrightness);
      break;
    case 2:
      channel3Pending.disable();
      channel3PendingFlashComplete.disable();
      analogWrite(CHANNEL_3_LAMP_PIN, buttonMinBrightness);
      break;
    case 3:
      channel4Pending.disable();
      channel4PendingFlashComplete.disable();
      analogWrite(CHANNEL_4_LAMP_PIN, buttonMinBrightness);
      break;
    case 4:
      channel5Pending.disable();
      channel5PendingFlashComplete.disable();
      analogWrite(CHANNEL_5_LAMP_PIN, buttonMinBrightness);
      break;
  }  
}
void stopChannel(int channel, midi::MidiInterface<AltSoftSerial> &midiInterface) {
  midiInterface.sendRealTime(midi::Stop);
  channelCountIn[channel] = -1;
  channelBars[channel] = -1;
  switch(channel) {
    case 0:
      channel1Pending.disable();
      channel1PendingFlashComplete.disable();
      analogWrite(CHANNEL_1_LAMP_PIN, buttonMinBrightness);
      break;
    case 1:
      channel2Pending.disable();
      channel2PendingFlashComplete.disable();
      analogWrite(CHANNEL_2_LAMP_PIN, buttonMinBrightness);
      break;
    case 2:
      channel3Pending.disable();
      channel3PendingFlashComplete.disable();
      analogWrite(CHANNEL_3_LAMP_PIN, buttonMinBrightness);
      break;
    case 3:
      channel4Pending.disable();
      channel4PendingFlashComplete.disable();
      analogWrite(CHANNEL_4_LAMP_PIN, buttonMinBrightness);
      break;
    case 4:
      channel5Pending.disable();
      channel5PendingFlashComplete.disable();
      analogWrite(CHANNEL_5_LAMP_PIN, buttonMinBrightness);
      break;
  }  
}
/** Start playback for a channel
 *  @param: channel - the channel initiating the state change 
 *  @param: midiInterface - the midi interface pointer for the channel
 */
void playChannel(int channel, midi::MidiInterface<HardwareSerial> &midiInterface) {
  midiInterface.sendSongPosition(0);
  midiInterface.sendRealTime(midi::Start);

  switch(channel) {
    case 0:
      channel1Pending.disable();
      channel1PendingFlashComplete.disable();
      analogWrite(CHANNEL_1_LAMP_PIN, buttonMaxBrightness);
      break;
    case 1:
      channel2Pending.disable();
      channel2PendingFlashComplete.disable();
      analogWrite(CHANNEL_2_LAMP_PIN, buttonMaxBrightness);
      break;
    case 2:
      channel3Pending.disable();
      channel3PendingFlashComplete.disable();
      analogWrite(CHANNEL_3_LAMP_PIN, buttonMaxBrightness);
      break;
    case 3:
      channel4Pending.disable();
      channel4PendingFlashComplete.disable();
      analogWrite(CHANNEL_4_LAMP_PIN, buttonMaxBrightness);
      break;
    case 4:
      channel5Pending.disable();
      channel5PendingFlashComplete.disable();
      analogWrite(CHANNEL_5_LAMP_PIN, buttonMaxBrightness);
      break;
  }
}
void playChannel(int channel, midi::MidiInterface<AltSoftSerial> &midiInterface) {
  midiInterface.sendSongPosition(0);
  midiInterface.sendRealTime(midi::Start);

  switch(channel) {
    case 0:
      channel1Pending.disable();
      channel1PendingFlashComplete.disable();
      analogWrite(CHANNEL_1_LAMP_PIN, buttonMaxBrightness);
      break;
    case 1:
      channel2Pending.disable();
      channel2PendingFlashComplete.disable();
      analogWrite(CHANNEL_2_LAMP_PIN, buttonMaxBrightness);
      break;
    case 2:
      channel3Pending.disable();
      channel3PendingFlashComplete.disable();
      analogWrite(CHANNEL_3_LAMP_PIN, buttonMaxBrightness);
      break;
    case 3:
      channel4Pending.disable();
      channel4PendingFlashComplete.disable();
      analogWrite(CHANNEL_4_LAMP_PIN, buttonMaxBrightness);
      break;
    case 4:
      channel5Pending.disable();
      channel5PendingFlashComplete.disable();
      analogWrite(CHANNEL_5_LAMP_PIN, buttonMaxBrightness);
      break;
  }
}

/** Queue a channel for playback 
 *  @param: channel - the channel initiating the state change 
 *  @param: midiInterface - the midi interface pointer for the channel
 */
void enqueueChannel(int channel, midi::MidiInterface<HardwareSerial> &midiInterface) {
  channelCountIn[channel] = countIn;
  if(playHead + countIn > 63) {
    channelBars[channel] = playHead + countIn - 64;    
  } else {
    channelBars[channel] = playHead + countIn;
  }
    
  switch(channel) {
    case 0:
      channel1Pending.restart();
      break;
    case 1:
      channel2Pending.restart();
      break;
    case 2:
      channel3Pending.restart();
      break;
    case 3:
      channel4Pending.restart();
      break;
    case 4:
      channel5Pending.restart();
      break;
  }
}
void enqueueChannel(int channel, midi::MidiInterface<AltSoftSerial> &midiInterface) {
  channelCountIn[channel] = countIn;
  if(playHead + countIn > 63) {
    channelBars[channel] = playHead + countIn - 64;    
  } else {
    channelBars[channel] = playHead + countIn;
  }
    
  switch(channel) {
    case 0:
      channel1Pending.restart();
      break;
    case 1:
      channel2Pending.restart();
      break;
    case 2:
      channel3Pending.restart();
      break;
    case 3:
      channel4Pending.restart();
      break;
    case 4:
      channel5Pending.restart();
      break;
  }
}

/** Set a listener for the encoder
 */
void handleEncoder() {
  if (encoderClkPin == LOW) {
    detachInterrupt(encInterruptPin);
    
    if (encoderDtPin == LOW) {
      tempo++;
    } else {
      tempo--;
    }
   
    updateTempo();
    encoderDebounce.restart();
  }
}

/** Set a listener for tapping
 */
void handleTap() {
  detachInterrupt(tapInterruptPin);
  tapDebounce.restart();
}


/** Debounce the tap button
 */
void tapDebounceCallback() {
  if (!tapDebounce.isFirstIteration()) {
    attachInterrupt(tapInterruptPin, handleTap, FALLING);
  }
}

/** Debounce the encoder
 */
void encoderDebounceCallback() {
  if (!encoderDebounce.isFirstIteration()) {
    attachInterrupt(encInterruptPin, handleEncoder, FALLING);
  }
}

/** Scan the buttons assigned to the channel for state changes
 */
void channel1InteractionCallback() {
  if(channel1ButtonPin == LOW) {
    if (performanceStarted == 0) {
      startPerformance(0, midi1);
    } else if ( performanceStarted == 1 && channelCountIn[0] == 0) {
      stopChannel(0, midi1);
    } else if ( performanceStarted == 1 && channelCountIn[0] > 0) {
      dequeueChannel(0);
    } else if ( performanceStarted == 1 && channelCountIn[0] == -1) {
      enqueueChannel(0, midi1);
    }
    
    channel1InteractionDebounce.restart();
    channel1Interaction.disable();  
  }
}

/** Debounce the channel interaction until the button returns to a high value
 */
void channel1InteractionDebounceCallback() {
  if(channel1ButtonPin == HIGH) {
     channel1Interaction.restart();
     channel1InteractionDebounce.disable();
  }
}

/** Flash the channel's LED to indicate it is queued for playback
 */
void channel1PendingCallback() {
  analogWrite(CHANNEL_1_LAMP_PIN, buttonMaxBrightness);
  channel1PendingFlashComplete.restart();
}

/** Complete the flash by pulling the channel indicator low again  
 */
void channel1PendingFlashCompleteCallback() {
  if(!channel1PendingFlashComplete.isFirstIteration()) {
    analogWrite(CHANNEL_1_LAMP_PIN, buttonMinBrightness);
  }
}

/** Scan the buttons assigned to the channel for state changes
 */
void channel2InteractionCallback() {
  if(channel2ButtonPin == LOW) {
    if (performanceStarted == 0) {
      startPerformance(1, midi2);
    } else if ( performanceStarted == 1 && channelCountIn[1] == 0) {
      stopChannel(1, midi2);
    } else if ( performanceStarted == 1 && channelCountIn[1] > 0) {
      dequeueChannel(1);
    } else if ( performanceStarted == 1 && channelCountIn[1] == -1) {
      enqueueChannel(1, midi2);
    }
    
    channel2InteractionDebounce.restart();
    channel2Interaction.disable();  
  }
}

/** Debounce the channel interaction until the button returns to a high value
 */
void channel2InteractionDebounceCallback() {
  if(channel2ButtonPin == HIGH) {
     channel2Interaction.restart();
     channel2InteractionDebounce.disable();
  }
}

/** Flash the channel's LED to indicate it is queued for playback
 */
void channel2PendingCallback() {
  analogWrite(CHANNEL_2_LAMP_PIN, buttonMaxBrightness);
  channel2PendingFlashComplete.restart();
}

/** Complete the flash by pulling the channel indicator low again  
 */
void channel2PendingFlashCompleteCallback() {
  if(!channel2PendingFlashComplete.isFirstIteration()) {
    analogWrite(CHANNEL_2_LAMP_PIN, buttonMinBrightness);
  }
}

/** Scan the buttons assigned to the channel for state changes
 */
void channel3InteractionCallback() {
  if(channel3ButtonPin == LOW) {
    if (performanceStarted == 0) {
      startPerformance(2, midi3);
    } else if ( performanceStarted == 1 && channelCountIn[2] == 0) {
      stopChannel(2, midi3);
    } else if ( performanceStarted == 1 && channelCountIn[2] > 0) {
      dequeueChannel(2);
    } else if ( performanceStarted == 1 && channelCountIn[2] == -1) {
      enqueueChannel(2, midi3);
    }
    
    channel3InteractionDebounce.restart();
    channel3Interaction.disable();  
  }
}

/** Debounce the channel interaction until the button returns to a high value
 */
void channel3InteractionDebounceCallback() {
  if(channel3ButtonPin == HIGH) {
     channel3Interaction.restart();
     channel3InteractionDebounce.disable();
  }
}

/** Flash the channel's LED to indicate it is queued for playback
 */
void channel3PendingCallback() {
  analogWrite(CHANNEL_3_LAMP_PIN, buttonMaxBrightness);
  channel3PendingFlashComplete.restart();
}

/** Complete the flash by pulling the channel indicator low again  
 */
void channel3PendingFlashCompleteCallback() {
  if(!channel3PendingFlashComplete.isFirstIteration()) {
    analogWrite(CHANNEL_3_LAMP_PIN, buttonMinBrightness);
  }
}

/** Scan the buttons assigned to the channel for state changes
 */
void channel4InteractionCallback() {
  if(channel4ButtonPin == LOW) {
    if (performanceStarted == 0) {
      startPerformance(3, midi4);
    } else if ( performanceStarted == 1 && channelCountIn[3] == 0) {
      stopChannel(3, midi4);
    } else if ( performanceStarted == 1 && channelCountIn[3] > 0) {
      dequeueChannel(3);
    } else if ( performanceStarted == 1 && channelCountIn[3] == -1) {
      enqueueChannel(3, midi4);
    }
    
    channel4InteractionDebounce.restart();
    channel4Interaction.disable();  
  }
}

/** Debounce the channel interaction until the button returns to a high value
 */
void channel4InteractionDebounceCallback() {
  if(channel4ButtonPin == HIGH) {
     channel4Interaction.restart();
     channel4InteractionDebounce.disable();
  }
}

/** Flash the channel's LED to indicate it is queued for playback
 */
void channel4PendingCallback() {
  analogWrite(CHANNEL_4_LAMP_PIN, buttonMaxBrightness);
  channel4PendingFlashComplete.restart();
}

/** Complete the flash by pulling the channel indicator low again  
 */
void channel4PendingFlashCompleteCallback() {
  if(!channel4PendingFlashComplete.isFirstIteration()) {
    analogWrite(CHANNEL_4_LAMP_PIN, buttonMinBrightness);
  }
}

/** Scan the buttons assigned to the channel for state changes
 */
void channel5InteractionCallback() {
  if(channel5ButtonPin == LOW) {
    if (performanceStarted == 0) {
      startPerformance(4, midi5);
    } else if ( performanceStarted == 1 && channelCountIn[4] == 0) {
      stopChannel(4, midi5);
    } else if ( performanceStarted == 1 && channelCountIn[4] > 0) {
      dequeueChannel(4);
    } else if ( performanceStarted == 1 && channelCountIn[4] == -1) {
      enqueueChannel(4, midi5);
    }
    
    channel5InteractionDebounce.restart();
    channel5Interaction.disable();  
  }
}

/** Debounce the channel interaction until the button returns to a high value
 */
void channel5InteractionDebounceCallback() {
  if(channel5ButtonPin == HIGH) {
     channel5Interaction.restart();
     channel5InteractionDebounce.disable();
  }
}

/** Flash the channel's LED to indicate it is queued for playback
 */
void channel5PendingCallback() {
  analogWrite(CHANNEL_5_LAMP_PIN, buttonMaxBrightness);
  channel5PendingFlashComplete.restart();
}

/** Complete the flash by pulling the channel indicator low again  
 */
void channel5PendingFlashCompleteCallback() {
  if(!channel5PendingFlashComplete.isFirstIteration()) {
    analogWrite(CHANNEL_5_LAMP_PIN, buttonMinBrightness);
  }
}

/** Indicate playback is not yet started by flashing the downbeat lamp an alternate color
 */
void comfortDownbeatCallback() {
  tempoRedPin = HIGH;
  downbeatFlashComplete.restart();
}

/** Clear the downbeat lamp by pulling all colors low
 */
void downbeatFlashCompleteCallback() {
  if(!downbeatFlashComplete.isFirstIteration()) {
    tempoRedPin = LOW;
  }
}

/** Update the global tempo value and display the value on screen
 */
void updateTempo() {
  quarterNoteTime = calulateQuarterNoteTime(tempo);
  midiClockTime = calulateMidiClockTime(quarterNoteTime);
  oled.setTempo(tempo);
  comfortDownbeat.setInterval(quarterNoteTime);
}

/** Poll for the "stop all channels" button
 */
void stopAllButtonInteractionCallback() {
  if (stopPin == LOW && performanceStarted == 1) {
    midiClock.disable();
    analogWrite(PLAY_ALL_LAMP_PIN, buttonMinBrightness);
    analogWrite(STOP_ALL_LAMP_PIN, buttonMaxBrightness);
    stopChannel(0, midi1);
    stopChannel(1, midi2);
    stopChannel(2, midi3);
    stopChannel(3, midi4);
    stopChannel(4, midi5);
    comfortDownbeat.restart();  
    performanceStarted = 0;
    for(int i = 0; i < channelCountInLength; i++) {
      channelCountIn[i] = -1; 
      channelBars[i] = -1;
    }
    clocks = 1;
    quarterNotes = 1;
    playHead = 0;
    stopAllButtonInteractionDebounce.restart();
    stopAllButtonInteraction.disable();
  }
}

/** Debounce the stop all button by waiting until the pin is high again
 */
void stopAllButtonInteractionDebounceCallback() {
  if (stopPin == HIGH) {
    stopAllButtonInteraction.restart();
    stopAllButtonInteractionDebounce.disable();
  }
}

/** Poll for the "play all channels" button
 */
void playAllButtonInteractionCallback() {
  if (playPin == LOW && performanceStarted == 0) {
    comfortDownbeat.disable();
    analogWrite(PLAY_ALL_LAMP_PIN, buttonMaxBrightness);
    analogWrite(STOP_ALL_LAMP_PIN, buttonMinBrightness);
    
    performanceStarted = 1;
    playHead = 0;
    clocks = 1;
    quarterNotes = 1;

    playChannel(0, midi1);
    playChannel(1, midi2);
    playChannel(2, midi3);
    playChannel(3, midi4);
    playChannel(4, midi5);
    
    for(int i = 0; i < channelCountInLength; i++) {
      channelCountIn[i] = 0;
    }
    
    playAllButtonInteractionDebounce.restart();
    playAllButtonInteraction.disable();
    delay(1);
    midiClock.restart();
  }
}

/** Debounce the play all channels button by waiting until the pin is high again
 */
void playAllButtonInteractionDebounceCallback() {
  if (playPin == HIGH) {
    playAllButtonInteraction.restart();
    playAllButtonInteractionDebounce.disable();
  }
}

/** The main MIDI loop during playback
 */
void midiClockCallback() {
  if(channelCountIn[0] == 0) {
    channelBars[0] = -1;
    midi1.sendRealTime(midi::Clock);
  }
  if(channelCountIn[1] == 0) {
    channelBars[1] = -1;
    midi2.sendRealTime(midi::Clock);
  }
  if(channelCountIn[2] == 0) {
    channelBars[2] = -1;
    midi3.sendRealTime(midi::Clock);
  }
  if(channelCountIn[3] == 0) {
    channelBars[3] = -1;
    midi4.sendRealTime(midi::Clock);
  }
  if(channelCountIn[4] == 0) {
    channelBars[4] = -1;
    midi5.sendRealTime(midi::Clock);
  } 
  if(clocks % 24 == 1) {
    tempoRedPin = HIGH;
    
    if(clocks == 1 || quarterNotes > 4) {
      quarterNotes = 1;

      if(clocks > 24) {
        playHead++;
        clocks = 1;
  
        if(playHead > 63) {
          playHead = 0;
        }
      }
      
      for(int i = 0; i < channelCountInLength; i++) {
        if(channelCountIn[i] > 0) {
          channelCountIn[i]--;
          if(channelCountIn[i] == 0) {
            switch(i){
              case 0:
                playChannel(0, midi1);
                break;
              case 1:
                playChannel(1, midi2);
                break;
              case 2:
                playChannel(2, midi3);
                break;
              case 3:
                playChannel(3, midi4);
                break;
              case 4:
                playChannel(4, midi5);
                break; 
            }
          }
        }
      }
    }
    
    downbeatFlashComplete.restart();
    midiClock.setInterval(midiClockTime);
    quarterNotes++;
  }
  clocks++;
}

void setup() {
  setupPinIo();
  initMidi();
  oled.begin();
  oled.drawRect(0, 0, 96, 96);
  oled.initialize();
  oled.setTempo(tempo);
  oled.drawRect(25, 10, 56, 16);
  scheduler.startNow();
}

void loop() {
  scheduler.execute();
}
