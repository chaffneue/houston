#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>

#include <Tlc5940.h>
#include <tlc_config.h>

#include <TaskScheduler.h>

#include <LiquidCrystal.h>

#include "Houston.h"
Task tempoUpInteraction(tempoPollTime, -1, &tempoUpInteractionCallback);
Task tempoDownInteraction(tempoPollTime, -1, &tempoDownInteractionCallback);
Task countInUpInteraction(countInPollTime, -1, &countInUpInteractionCallback);
Task countInDownInteraction(countInPollTime, -1, &countInDownInteractionCallback);
Task countInUpDebounce(countInPollTime, -1, &countInUpDebounceCallback);
Task countInDownDebounce(countInPollTime, -1, &countInDownDebounceCallback);
Task comfortDownbeat(quarterNoteTime, -1, &comfortDownbeatCallback);
Task downbeatFlashComplete(10, 2, &downbeatFlashCompleteCallback);
Task stopAllButtonInteraction(transportPollTime, -1, &stopAllButtonInteractionCallback);
Task stopAllButtonInteractionDebounce(transportPollTime, -1, &stopAllButtonInteractionDebounceCallback);
Task playAllButtonInteraction(transportPollTime, -1, &playAllButtonInteractionCallback);
Task playAllButtonInteractionDebounce(transportPollTime, -1, &playAllButtonInteractionDebounceCallback);
Task midiClock(midiClockTime, -1, &midiClockCallback);

Task channel1Interaction(transportPollTime, -1, &channel1InteractionCallback);
Task channel1InteractionDebounce(transportPollTime, -1, &channel1InteractionDebounceCallback);
Task channel1Pending(250, -1, &channel1PendingCallback);
Task channel1PendingFlashComplete(90, 2, &channel1PendingFlashCompleteCallback);

Task channel2Interaction(transportPollTime, -1, &channel2InteractionCallback);
Task channel2InteractionDebounce(transportPollTime, -1, &channel2InteractionDebounceCallback);
Task channel2Pending(250, -1, &channel2PendingCallback);
Task channel2PendingFlashComplete(90, 2, &channel2PendingFlashCompleteCallback);

Task channel3Interaction(transportPollTime, -1, &channel3InteractionCallback);
Task channel3InteractionDebounce(transportPollTime, -1, &channel3InteractionDebounceCallback);
Task channel3Pending(250, -1, &channel3PendingCallback);
Task channel3PendingFlashComplete(90, 2, &channel3PendingFlashCompleteCallback);

Task channel4Interaction(transportPollTime, -1, &channel4InteractionCallback);
Task channel4InteractionDebounce(transportPollTime, -1, &channel4InteractionDebounceCallback);
Task channel4Pending(250, -1, &channel4PendingCallback);
Task channel4PendingFlashComplete(90, 2, &channel4PendingFlashCompleteCallback);

void dequeueChannel(int channel) {
  channelCountIn[channel] = -1;
  switch(channel) {
    case 0:
      channel1Pending.disable();
      channel1PendingFlashComplete.disable();
      break;
    case 1:
      channel2Pending.disable();
      channel2PendingFlashComplete.disable();
      break;
    case 2:
      channel3Pending.disable();
      channel3PendingFlashComplete.disable();
      break;
    case 3:
      channel4Pending.disable();
      channel4PendingFlashComplete.disable();
      break;
  }
  digitalWrite(channelIndicatorPins[channel], LOW);  
}

void startPerformance(int channel, midi::MidiInterface<HardwareSerial> &midiInterface) {
  performanceStarted = 1;
  midiInterface.sendRealTime(midi::Start);
  comfortDownbeat.disable();
  delay(1); //halt program execution so we don't run the chance of confusing the midi device
  channelCountIn[channel] = 0;
  midiClock.restart();
  digitalWrite(channelIndicatorPins[channel], HIGH);  
}

void stopChannel(int channel, midi::MidiInterface<HardwareSerial> &midiInterface) {
  midiInterface.sendRealTime(midi::Stop);
  channelCountIn[channel] = -1;
  digitalWrite(channelIndicatorPins[channel], LOW);  
}

void enqueueChannel(int channel, midi::MidiInterface<HardwareSerial> &midiInterface) {
  midiInterface.sendRealTime(midi::Start);
  channelCountIn[channel] = countIn;
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

void channel1InteractionCallback() {
  if(digitalRead(channelButtonPins[0]) == LOW) {
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

void channel1InteractionDebounceCallback() {
  if(digitalRead(channelButtonPins[0]) == HIGH) {
     channel1Interaction.restart();
     channel1InteractionDebounce.disable();
  }
}

void channel1PendingCallback() {
  digitalWrite(channelIndicatorPins[0], HIGH);
  channel1PendingFlashComplete.restart();
}

void channel1PendingFlashCompleteCallback() {
  if(!channel1PendingFlashComplete.isFirstIteration()) {
    digitalWrite(channelIndicatorPins[0], LOW);
  }
}

void channel2InteractionCallback() {
  if(digitalRead(channelButtonPins[1]) == LOW) {
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

void channel2InteractionDebounceCallback() {
  if(digitalRead(channelButtonPins[1]) == HIGH) {
     channel2Interaction.restart();
     channel2InteractionDebounce.disable();
  }
}

void channel2PendingCallback() {
  digitalWrite(channelIndicatorPins[1], HIGH);
  channel2PendingFlashComplete.restart();
}

void channel2PendingFlashCompleteCallback() {
  if(!channel2PendingFlashComplete.isFirstIteration()) {
    digitalWrite(channelIndicatorPins[1], LOW);
  }
}

void channel3InteractionCallback() {
  if(digitalRead(channelButtonPins[2]) == LOW) {
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

void channel3InteractionDebounceCallback() {
  if(digitalRead(channelButtonPins[2]) == HIGH) {
     channel3Interaction.restart();
     channel3InteractionDebounce.disable();
  }
}

void channel3PendingCallback() {
  digitalWrite(channelIndicatorPins[2], HIGH);
  channel3PendingFlashComplete.restart();
}

void channel3PendingFlashCompleteCallback() {
  if(!channel3PendingFlashComplete.isFirstIteration()) {
    digitalWrite(channelIndicatorPins[2], LOW);
  }
}

void channel4InteractionCallback() {
  if(digitalRead(channelButtonPins[3]) == LOW) {
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

void channel4InteractionDebounceCallback() {
  if(digitalRead(channelButtonPins[3]) == HIGH) {
     channel4Interaction.restart();
     channel4InteractionDebounce.disable();
  }
}

void channel4PendingCallback() {
  digitalWrite(channelIndicatorPins[3], HIGH);
  channel4PendingFlashComplete.restart();
}

void channel4PendingFlashCompleteCallback() {
  if(!channel4PendingFlashComplete.isFirstIteration()) {
    digitalWrite(channelIndicatorPins[3], LOW);
  }
}

void comfortDownbeatCallback() {
  digitalWrite(tempoBluePin, HIGH);
  downbeatFlashComplete.restart();
}

void downbeatFlashCompleteCallback() {
  if(!downbeatFlashComplete.isFirstIteration()) {
    digitalWrite(tempoRedPin, LOW);
    digitalWrite(tempoGreenPin, LOW);
    digitalWrite(tempoBluePin, LOW);
  }
}

void updateTempo() {
  quarterNoteTime = calulateQuarterNoteTime(tempo);
  midiClockTime = calulateMidiClockTime(quarterNoteTime);
  comfortDownbeat.setInterval(quarterNoteTime);
  printTempo();
}

void tempoUpInteractionCallback() {
  if (digitalRead(tempoUpPin) == LOW && tempo < 300) { 
    tempo++;
    updateTempo();
  }  
}

void tempoDownInteractionCallback() {
  if (digitalRead(tempoDownPin) == LOW && tempo > 30) {
    tempo--;
    updateTempo();
  }
}
 
void countInUpInteractionCallback() {
  if (digitalRead(countInUpPin) == LOW && countIn < 64 ) {
    countIn++;
    printCountIn();
    countInUpDebounce.enable();
    countInUpInteraction.disable();
  }  
}

void countInDownInteractionCallback() {
  if (digitalRead(countInDownPin) == LOW && countIn > 1) {
    countIn--;
    printCountIn();
    countInDownDebounce.enable();
    countInDownInteraction.disable();
  }
}

void countInUpDebounceCallback() {
  if(digitalRead(countInUpPin) == HIGH) {
     countInUpInteraction.restart();
     countInUpDebounce.disable();
  }
}

void countInDownDebounceCallback() {
  if(digitalRead(countInDownPin) == HIGH) {
     countInDownInteraction.restart();
     countInDownDebounce.disable();
  }
}

void stopAllButtonInteractionCallback() {
  if (digitalRead(stopPin) == LOW && performanceStarted == 1) {
    midiClock.disable();
    channel1Pending.disable();
    channel1PendingFlashComplete.disable();
    channel2Pending.disable();
    channel2PendingFlashComplete.disable();
    channel3Pending.disable();
    channel3PendingFlashComplete.disable();
    channel4Pending.disable();
    channel4PendingFlashComplete.disable();
    comfortDownbeat.restart();  
    performanceStarted = 0;
    for(int i = 0; i < channelButtonPinsLength; i++) {
      channelCountIn[i] = -1; 
      digitalWrite(channelIndicatorPins[i], LOW);
    }
    clocks = 1;
    quarterNotes = 1;
    bars = 1;
    stopAllButtonInteractionDebounce.restart();
    stopAllButtonInteraction.disable();
  }
}

void stopAllButtonInteractionDebounceCallback() {
  if (digitalRead(stopPin) == HIGH) {
    stopAllButtonInteraction.restart();
    stopAllButtonInteractionDebounce.disable();
  }
}

void playAllButtonInteractionCallback() {
  if (digitalRead(playPin) == LOW && performanceStarted == 0) {
    comfortDownbeat.disable();
    performanceStarted = 1;
    bars = 1;
    clocks = 1;
    quarterNotes = 1;
    bars = 1;
    midi1.sendRealTime(midi::Start);
    midi2.sendRealTime(midi::Start);
    midi3.sendRealTime(midi::Start);
    midi4.sendRealTime(midi::Start);
    for(int i = 0; i < channelButtonPinsLength; i++) {
      channelCountIn[i] = 0; 
      digitalWrite(channelIndicatorPins[i], HIGH);
    }
    playAllButtonInteractionDebounce.restart();
    playAllButtonInteraction.disable();
    delay(1);
    midiClock.restart();
  }
}

void playAllButtonInteractionDebounceCallback() {
  if (digitalRead(playPin) == HIGH) {
    playAllButtonInteraction.restart();
    playAllButtonInteractionDebounce.disable();
  }
}

void midiClockCallback() {
  if(channelCountIn[0] == 0) {
    midi1.sendRealTime(midi::Clock);
  }
  if(channelCountIn[1] == 0) {
    midi2.sendRealTime(midi::Clock);
  }
  if(channelCountIn[2] == 0) {
    midi3.sendRealTime(midi::Clock);
  }
  if(channelCountIn[3] == 0) {
    midi4.sendRealTime(midi::Clock);
  }
  
  if(clocks % 24 == 1) {
    if(clocks == 1 || quarterNotes % 4 == 1) {
      digitalWrite(tempoGreenPin, HIGH);

      if(vis > 7) {
        vis = 0; 
      }


      vis++;
      bars++;
      for(int i = 0; i < channelCountInLength; i++) {
        if(channelCountIn[i] > 0) {
          channelCountIn[i]--;
          if(channelCountIn[i] == 0) {
            switch(i){
              case 0:
                channel1Pending.disable();
                channel1PendingFlashComplete.disable();
                break;
              case 1:
                channel2Pending.disable();
                channel2PendingFlashComplete.disable();
                break;
              case 2:
                channel3Pending.disable();
                channel3PendingFlashComplete.disable();
                break;
              case 3:
                channel4Pending.disable();
                channel4PendingFlashComplete.disable();
                break;
            }
            digitalWrite(channelIndicatorPins[i], HIGH);
          }
        }
      }
    } else {
      digitalWrite(tempoRedPin, HIGH);
    }
    downbeatFlashComplete.restart();
    midiClock.setInterval(midiClockTime);
    quarterNotes++;
  }
  clocks++;
}

void setup() {
  setupPinIo();
  initLcd();
  initMidi();
  
  scheduler.init();
  scheduler.addTask(tempoUpInteraction);
  scheduler.addTask(tempoDownInteraction);
  scheduler.addTask(countInUpInteraction);
  scheduler.addTask(countInUpDebounce);
  scheduler.addTask(countInDownDebounce);  
  scheduler.addTask(countInDownInteraction);
  scheduler.addTask(stopAllButtonInteraction);
  scheduler.addTask(stopAllButtonInteractionDebounce);
  scheduler.addTask(playAllButtonInteraction);
  scheduler.addTask(playAllButtonInteractionDebounce);
  scheduler.addTask(comfortDownbeat);
  scheduler.addTask(downbeatFlashComplete);
  
  scheduler.addTask(channel1Interaction);
  scheduler.addTask(channel1InteractionDebounce);
  scheduler.addTask(channel1Pending);
  scheduler.addTask(channel1PendingFlashComplete);
  
  scheduler.addTask(channel2Interaction);
  scheduler.addTask(channel2InteractionDebounce);
  scheduler.addTask(channel2Pending);
  scheduler.addTask(channel2PendingFlashComplete);

  scheduler.addTask(channel3Interaction);
  scheduler.addTask(channel3InteractionDebounce);
  scheduler.addTask(channel3Pending);
  scheduler.addTask(channel3PendingFlashComplete);

  scheduler.addTask(channel4Interaction);
  scheduler.addTask(channel4InteractionDebounce);
  scheduler.addTask(channel4Pending);
  scheduler.addTask(channel4PendingFlashComplete);
  
  scheduler.addTask(midiClock);

  tempoUpInteraction.enable();
  tempoDownInteraction.enable();
  countInUpInteraction.enable();
  countInDownInteraction.enable();
  countInUpDebounce.disable();
  countInDownDebounce.disable();
  comfortDownbeat.enable();
  downbeatFlashComplete.disable();
  stopAllButtonInteraction.enable();
  stopAllButtonInteractionDebounce.disable();
  playAllButtonInteraction.enable();
  playAllButtonInteractionDebounce.disable();
  
  channel1Interaction.enable();
  channel1InteractionDebounce.disable();
  channel1Pending.disable();
  channel1PendingFlashComplete.disable();
  
  channel2Interaction.enable();
  channel2InteractionDebounce.disable();
  channel2Pending.disable();
  channel2PendingFlashComplete.disable();
  
  channel3Interaction.enable();
  channel3InteractionDebounce.disable();
  channel3Pending.disable();
  channel3PendingFlashComplete.disable();
  
  channel4Interaction.enable();
  channel4InteractionDebounce.disable();
  channel4Pending.disable();
  channel4PendingFlashComplete.disable();
  
  midiClock.disable();

  delay(20);
}

void loop() {
  scheduler.execute();
}
