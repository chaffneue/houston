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

#include <Tlc5940.h>
#include <tlc_config.h>

#define _TASK_MICRO_RES
#include <TaskScheduler.h>

#include <LiquidCrystal.h>

#include "Houston.h"

// Task implementation 
Task midiClock(midiClockTime, -1, &midiClockCallback, &scheduler);
Task updateMatrix(MATRIX_ROW_UPDATE_INTERVAL, -1, &updateMatrixCallback, &scheduler, true);

Task tempoUpInteraction(POLL_TIME_TEMPO, -1, &tempoUpInteractionCallback, &scheduler, true);
Task tempoDownInteraction(POLL_TIME_TEMPO, -1, &tempoDownInteractionCallback, &scheduler, true);
Task countInUpInteraction(POLL_TIME_COUNT_IN, -1, &countInUpInteractionCallback, &scheduler, true);
Task countInDownInteraction(POLL_TIME_COUNT_IN, -1, &countInDownInteractionCallback, &scheduler, true);
Task countInUpDebounce(POLL_TIME_COUNT_IN, -1, &countInUpDebounceCallback, &scheduler);
Task countInDownDebounce(POLL_TIME_COUNT_IN, -1, &countInDownDebounceCallback, &scheduler);
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
      channel1LampPin = LOW;
      break;
    case 1:
      channel2Pending.disable();
      channel2PendingFlashComplete.disable();
      channel2LampPin = LOW;
      break;
    case 2:
      channel3Pending.disable();
      channel3PendingFlashComplete.disable();
      channel3LampPin = LOW;
      break;
    case 3:
      channel4Pending.disable();
      channel4PendingFlashComplete.disable();
      channel4LampPin = LOW;
      break;
  } 
}

/** Start the performance
 *  @param: channel - the channel initiating the statee change 
 *  @param: midiInterface - the midi interface pointer for the channel
 */
void startPerformance(int channel, midi::MidiInterface<HardwareSerial> &midiInterface) {
  performanceStarted = 1;
  midiInterface.sendRealTime(midi::Start);
  comfortDownbeat.disable();
  delay(1); //halt program execution so we don't run the chance of confusing the midi device
  channelCountIn[channel] = 0;
  midiClock.restart();
  switch(channel) {
    case 0:
      channel1LampPin = HIGH;
      break;
    case 1:
      channel2LampPin = HIGH;
      break;
    case 2:
      channel3LampPin = HIGH;
      break;
    case 3:
      channel4LampPin = HIGH;
      break;
  } 
}

/** Stop a single channel
 *  @param: channel - the channel to stop immediately 
 */
void stopChannel(int channel, midi::MidiInterface<HardwareSerial> &midiInterface) {
  midiInterface.sendRealTime(midi::Stop);
  channelCountIn[channel] = -1;
  channelBars[channel] = -1;
  switch(channel) {
    case 0:
      channel1LampPin = LOW;
      break;
    case 1:
      channel2LampPin = LOW;
      break;
    case 2:
      channel3LampPin = LOW;
      break;
    case 3:
      channel4LampPin = LOW;
      break;
  }  
}

/** Queue a channel for playback 
 *  @param: channel - the channel initiating the statee change 
 *  @param: midiInterface - the midi interface pointer for the channel
 */
void enqueueChannel(int channel, midi::MidiInterface<HardwareSerial> &midiInterface) {
  midiInterface.sendRealTime(midi::Start);
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
  channel1LampPin = HIGH;
  channel1PendingFlashComplete.restart();
}

/** Complete the flash by pulling the channel indicator low again  
 */
void channel1PendingFlashCompleteCallback() {
  if(!channel1PendingFlashComplete.isFirstIteration()) {
    channel1LampPin = LOW;
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
  channel2LampPin = HIGH;
  channel2PendingFlashComplete.restart();
}

/** Complete the flash by pulling the channel indicator low again  
 */
void channel2PendingFlashCompleteCallback() {
  if(!channel2PendingFlashComplete.isFirstIteration()) {
    channel2LampPin = LOW;
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
  channel3LampPin = HIGH;
  channel3PendingFlashComplete.restart();
}

/** Complete the flash by pulling the channel indicator low again  
 */
void channel3PendingFlashCompleteCallback() {
  if(!channel3PendingFlashComplete.isFirstIteration()) {
    channel3LampPin = LOW;
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
  channel4LampPin = HIGH;
  channel4PendingFlashComplete.restart();
}

/** Complete the flash by pulling the channel indicator low again  
 */
void channel4PendingFlashCompleteCallback() {
  if(!channel4PendingFlashComplete.isFirstIteration()) {
    channel4LampPin = LOW;
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
    tempoGreenPin =  LOW;
    tempoBluePin = LOW;
  }
}

/** Update the global tempo value and display the value on screen
 */
void updateTempo() {
  quarterNoteTime = calulateQuarterNoteTime(tempo);
  midiClockTime = calulateMidiClockTime(quarterNoteTime);
  comfortDownbeat.setInterval(quarterNoteTime);
  printTempo();
}

/** Poll for the tempo up button 
 */
void tempoUpInteractionCallback() {
  if (tempoUpPin == LOW && tempo < 300) { 
    tempo = tempo + 0.1;
    updateTempo();
  }  
}

/** Poll for the tempo down button 
 */
void tempoDownInteractionCallback() {
  if (tempoDownPin == LOW && tempo > 30) {
    tempo = tempo - 0.1;
    updateTempo();
  }
}

/** Poll for the "count in" up button  
 */
void countInUpInteractionCallback() {
  if (countInUpPin == LOW && countIn < 64 ) {
    countIn++;
    printCountIn();
    countInUpDebounce.enable();
    countInUpInteraction.disable();
  }  
}

/** Poll for the "count in" down button
 */
void countInDownInteractionCallback() {
  if (countInDownPin == LOW && countIn > 1) {
    countIn--;
    printCountIn();
    countInDownDebounce.enable();
    countInDownInteraction.disable();
  }
}

/** debounce the count in button by waiting until the pin is high again
 */
void countInUpDebounceCallback() {
  if(countInUpPin == HIGH) {
     countInUpInteraction.restart();
     countInUpDebounce.disable();
  }
}

/** debounce the count in button by waiting until the pin is high again
 */
void countInDownDebounceCallback() {
  if(countInDownPin == HIGH) {
     countInDownInteraction.restart();
     countInDownDebounce.disable();
  }
}

/** Poll for the "stop all channels" button
 */
void stopAllButtonInteractionCallback() {
  if (stopPin == LOW && performanceStarted == 1) {
    midiClock.disable();
    channel1Pending.disable();
    channel1PendingFlashComplete.disable();
    channel1LampPin = LOW;
    channel2Pending.disable();
    channel2PendingFlashComplete.disable();
    channel2LampPin = LOW;
    channel3Pending.disable();
    channel3PendingFlashComplete.disable();
    channel3LampPin = LOW;
    channel4Pending.disable();
    channel4PendingFlashComplete.disable();
    channel4LampPin = LOW;
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
    
    performanceStarted = 1;
    playHead = 0;
    clocks = 1;
    quarterNotes = 1;

    midi1.sendRealTime(midi::Start);
    channel1LampPin = HIGH;
    midi2.sendRealTime(midi::Start);
    channel2LampPin = HIGH;
    midi3.sendRealTime(midi::Start);
    channel3LampPin = HIGH;
    midi4.sendRealTime(midi::Start);
    channel4LampPin = HIGH;
    
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
  
  if(clocks % 24 == 1) {
    if(clocks == 1 || quarterNotes > 4) {
      tempoGreenPin = HIGH;

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
                channel1Pending.disable();
                channel1PendingFlashComplete.disable();
                channel1LampPin = HIGH;
                break;
              case 1:
                channel2Pending.disable();
                channel2PendingFlashComplete.disable();
                channel2LampPin = HIGH;
                break;
              case 2:
                channel3Pending.disable();
                channel3PendingFlashComplete.disable();
                channel3LampPin = HIGH;
                break;
              case 3:
                channel4Pending.disable();
                channel4PendingFlashComplete.disable();
                channel4LampPin = HIGH;
                break;
            }
          }
        }
      }
    } else {
      tempoBluePin = HIGH;
    }
    
    downbeatFlashComplete.restart();
    midiClock.setInterval(midiClockTime);
    quarterNotes++;
  }
  clocks++;
}

/** Update the 8x8 RGB matrix display to give feedback about queued channels and the playhead location
 */
void updateMatrixCallback() {
  Tlc.clear();
  
  for (matrixColumn = 0; matrixColumn < 8; matrixColumn++) {
    currentGridLocation = matrixRow * 8 + matrixColumn;
    
    if(channelBars[0] != -1 && channelBars[0] == currentGridLocation) {
      setColor(red, matrixColumn);
    } else if (channelBars[1] != -1 && channelBars[1] == currentGridLocation) {
      setColor(green, matrixColumn);
    } else if (channelBars[2] != -1 && channelBars[2] == currentGridLocation) {
      setColor(yellow, matrixColumn);
    } else if (channelBars[3] != -1 && channelBars[3] == currentGridLocation) {
      setColor(purple, matrixColumn);
    } else if (currentGridLocation == playHead) {
      setColor(white, matrixColumn);
    } else if (matrixColumn == 0 || matrixColumn == 4) { 
      setColor(lightBlue, matrixColumn);
    } else {
      setColor(darkBlue, matrixColumn);      
    }
  }
  
  //column control
  Tlc.set(rowOutputs[matrixRow], 1000);    
  Tlc.update();
  
  if(matrixRow > 6) {
    matrixRow = 0;
  } else {
    matrixRow++;
  }
}

void setup() {
  setupPinIo();
  initLcd();
  initMidi();
  initMatrix();
  scheduler.startNow();
}

void loop() {
  scheduler.execute();
}
