/*
  Modified TWI to remove some busy waits to work better with Houston's scheduler and the
  nature of the application (fire and forget)  
  
  twi.c - TWI/I2C library for Wiring & Arduino
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 2012 by Todd Krein (todd@krein.org) to implement repeated starts
*/

#include <math.h>
#include <stdlib.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <compat/twi.h>
#include "Arduino.h"

#ifndef cbi
  #define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
  #define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#include "pins_arduino.h"
#include "twi.h"

static volatile uint8_t twi_state;
static volatile uint8_t twi_slarw;
static volatile uint8_t twi_sendStop;			// should the transaction end with a stop
static volatile uint8_t twi_inRepStart;			// in the middle of a repeated start

static void (*twi_onSlaveTransmit)(void);
static void (*twi_onSlaveReceive)(uint8_t*, int);

static uint8_t twi_masterBuffer[TWI_BUFFER_LENGTH];
static volatile uint8_t twi_masterBufferIndex;
static volatile uint8_t twi_masterBufferLength;

static uint8_t twi_txBuffer[TWI_BUFFER_LENGTH];
static volatile uint8_t twi_txBufferIndex;
static volatile uint8_t twi_txBufferLength;

static volatile uint8_t twi_error;

/*
 * Function twi_init
 * Desc     readys twi pins and sets twi bitrate
 * Input    none
 * Output   none
 */
void twi_init(void)
{
  // initialize state
  twi_state = TWI_READY;
  twi_sendStop = true;		// default value
  twi_inRepStart = false;

  // initialize twi prescaler and bit rate
  cbi(TWSR, TWPS0);
  cbi(TWSR, TWPS1);
  TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;

  /* twi bit rate formula from atmega128 manual pg 204
  SCL Frequency = CPU Clock Frequency / (16 + (2 * TWBR))
  note: TWBR should be 10 or higher for master mode
  It is 72 for a 16mhz Wiring board with 100kHz TWI */

  // enable twi module, acks, and twi interrupt
  TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA);
}

/*
 * Function twi_slaveInit
 * Desc     sets slave address and enables interrupt
 * Input    none
 * Output   none
 */
void twi_setAddress(uint8_t address)
{
  // set twi slave address (skip over TWGCE bit)
  TWAR = address << 1;
}

/*
 * Function twi_writeTo
 * Desc     attempts to become twi bus master and write a
 *          series of bytes to a device on the bus
 * Input    address: 7bit i2c device address
 *          data: pointer to byte array
 *          length: number of bytes in array
 *          wait: boolean indicating to wait for write or not
 *          sendStop: boolean indicating whether or not to send a stop at the end
 * Output   0 .. success
 *          1 .. length to long for buffer
 *          2 .. address send, NACK received
 *          3 .. data send, NACK received
 *          4 .. other twi error (lost bus arbitration, bus error, ..)
 */
uint8_t twi_writeTo(uint8_t address, uint8_t* data, uint8_t length, uint8_t wait, uint8_t sendStop)
{
  uint8_t i;

  // wait until twi is ready, become master transmitter
  while(TWI_READY != twi_state){
    continue;
  }
  
  twi_state = TWI_MTX;
  twi_sendStop = sendStop;
  // reset error state (0xFF.. no error occured)
  twi_error = 0xFF;

  // initialize buffer iteration vars
  twi_masterBufferIndex = 0;
  twi_masterBufferLength = length;

  // copy data to twi buffer
  for(i = 0; i < length; ++i){
    twi_masterBuffer[i] = data[i];
  }

  // build sla+w, slave device address + w bit
  twi_slarw = TW_WRITE;
  twi_slarw |= address << 1;

  // if we're in a repeated start, then we've already sent the START
  // in the ISR. Don't do it again.
  if (true == twi_inRepStart) {
    // if we're in the repeated start state, then we've already sent the start,
    // (@@@ we hope), and the TWI statemachine is just waiting for the address byte.
    // We need to remove ourselves from the repeated start state before we enable interrupts,
    // since the ISR is ASYNC, and we could get confused if we hit the ISR before cleaning
    // up. Also, don't enable the START interrupt. There may be one pending from the
    // repeated start that we sent outselves, and that would really confuse things.
    twi_inRepStart = false;			// remember, we're dealing with an ASYNC ISR
    do {
      TWDR = twi_slarw;
    } while(TWCR & _BV(TWWC));
    TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);	// enable INTs, but not START
  }
  else
    // send start condition
    TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTA);	// enable INTs

  //fire and forget
  return 0;
}

/*
 * Function twi_transmit
 * Desc     fills slave tx buffer with data
 *          must be called in slave tx event callback
 * Input    data: pointer to byte array
 *          length: number of bytes in array
 * Output   1 length too long for buffer
 *          2 not slave transmitter
 *          0 ok
 */
uint8_t twi_transmit(const uint8_t* data, uint8_t length)
{
  uint8_t i;

  // set length and copy data into tx buffer
  for(i = 0; i < length; ++i){
    twi_txBuffer[twi_txBufferLength+i] = data[i];
  }
  twi_txBufferLength += length;

  return 0;
}

/*
 * Function twi_reply
 * Desc     sends byte or readys receive line
 * Input    ack: byte indicating to ack or to nack
 * Output   none
 */
void twi_reply(uint8_t ack)
{
  // transmit master read ready signal, with or without ack
  if(ack){
    TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA);
  }else{
	  TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
  }
}

/*
 * Function twi_stop
 * Desc     relinquishes bus master status
 * Input    none
 * Output   none
 */
void twi_stop(void)
{
  // send stop condition
  TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTO);

  // wait for stop condition to be exectued on bus
  // TWINT is not set after a stop condition!
  while(TWCR & _BV(TWSTO)){
    continue;
  }

  // update twi state
  twi_state = TWI_READY;
}

/*
 * Function twi_releaseBus
 * Desc     releases bus control
 * Input    none
 * Output   none
 */
void twi_releaseBus(void)
{
  // release bus
  TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT);

  // update twi state
  twi_state = TWI_READY;
}

ISR(TWI_vect)
{
  switch(TW_STATUS){
    // Master Transmitter
    case TW_MT_SLA_ACK:  // slave receiver acked address
    case TW_MT_DATA_ACK: // slave receiver acked data
      // if there is data to send, send it, otherwise stop
      if(twi_masterBufferIndex < twi_masterBufferLength){
        // copy data to output register and ack
        TWDR = twi_masterBuffer[twi_masterBufferIndex++];
        twi_reply(1);
      } else {
    	if (twi_sendStop) {
              twi_stop();
    	} else {
    	  twi_inRepStart = true;	// we're gonna send the START
    	  // don't enable the interrupt. We'll generate the start, but we
    	  // avoid handling the interrupt until we're in the next transaction,
    	  // at the point where we would normally issue the start.
    	  TWCR = _BV(TWINT) | _BV(TWSTA)| _BV(TWEN) ;
    	  twi_state = TWI_READY;
    	}
      }
      break;

    // All Master
    case TW_START:     // sent start condition
    case TW_REP_START: // sent repeated start condition
        // copy device address and r/w bit to output register and ack
        TWDR = twi_slarw;
        twi_reply(1);
        break;
    case TW_MT_SLA_NACK:  // address sent, nack received
      twi_error = TW_MT_SLA_NACK;
      twi_stop();
      break;
    case TW_MT_DATA_NACK: // data sent, nack received
      twi_error = TW_MT_DATA_NACK;
      twi_stop();
      break;
    case TW_MT_ARB_LOST: // lost bus arbitration
      twi_error = TW_MT_ARB_LOST;
      twi_releaseBus();
      break;

    // All
    case TW_NO_INFO:   // no state information
      break;
    case TW_BUS_ERROR: // bus error, illegal stop/start
      twi_error = TW_BUS_ERROR;
      twi_stop();
      break;
  }
}
