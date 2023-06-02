#pragma once

/*****************************************************************************
  Based on https://github.com/adafruit/TinyWireM

  Delays mostly removed or reduced, since only the one at the end of
  USI_TWI_Master_Stop matters in practice on my devices, and the
  specification I read requires much shorter minimum times then

  Only tested to do I2C communication from an ATtiny85 running at 1, 8 or 16 MHz,
  to an SSD1306.
*/

//********** Defines **********//

/****************************************************************************
  Bit and byte definitions
****************************************************************************/
#define USI_TWI_READ_BIT 0 //!< Bit position for R/W bit in "address byte".
#define USI_TWI_ADR_BITS                                                           \
  1 //!< Bit position for LSB of the slave address bits in the init byte.
#define USI_TWI_NACK_BIT 0 //!< Bit position for (N)ACK bit.

// Note these have been renumbered from the Atmel Apps Note. Most likely errors
// are now lowest numbers so they're easily recognized as LED flashes.
enum USI_TWI_ErrorLevel {
  USI_TWI_OK = 0,
  USI_TWI_NO_SCL_HI = 11, //!< SCL did not go high when instructed to.
  USI_TWI_NO_SCL_LO = 12, //!< SCL did not go low when instructed to.
  USI_TWI_ME_START_CON = 8, //!< Missing Expected Start Condition
  USI_TWI_UE_START_CON = 7, //!< Unexpected Start Condition
  USI_TWI_UE_STOP_CON = 6,  //!< Unexpected Stop Condition
  USI_TWI_UE_DATA_COL = 5,  //!< Unexpected Data Collision (arbitration)
  USI_TWI_NO_ACK_ON_DATA = 2, //!< The slave did not acknowledge all data
  USI_TWI_NO_ACK_ON_ADDRESS = 1, //!< The slave did not acknowledge the address
  USI_TWI_MISSING_START_CON = 3, //!< Generated Start Condition not detected on bus
  USI_TWI_MISSING_STOP_CON  = 4, //!< Generated Stop Condition not detected on bus
};

// Device dependant defines ADDED BACK IN FROM ORIGINAL ATMEL .H

#if defined(__AVR_AT90Mega169__) | defined(__AVR_ATmega169__) |                \
    defined(__AVR_AT90Mega165__) | defined(__AVR_ATmega165__) |                \
    defined(__AVR_ATmega325__) | defined(__AVR_ATmega3250__) |                 \
    defined(__AVR_ATmega645__) | defined(__AVR_ATmega6450__) |                 \
    defined(__AVR_ATmega329__) | defined(__AVR_ATmega3290__) |                 \
    defined(__AVR_ATmega649__) | defined(__AVR_ATmega6490__)
#define DDR_USI DDRE
#define PORT_USI PORTE
#define PIN_USI PINE
#define PORT_USI_SDA PORTE5
#define PORT_USI_SCL PORTE4
#define PIN_USI_SDA PINE5
#define PIN_USI_SCL PINE4
#endif

#if defined(__AVR_ATtiny25__) | defined(__AVR_ATtiny45__) |                    \
    defined(__AVR_ATtiny85__) | defined(__AVR_AT90Tiny26__) |                  \
    defined(__AVR_ATtiny26__)
#define DDR_USI DDRB
#define PORT_USI PORTB
#define PIN_USI PINB
#define PORT_USI_SDA PORTB0
#define PORT_USI_SCL PORTB2
#define PIN_USI_SDA PINB0
#define PIN_USI_SCL PINB2
#endif

#if defined(__AVR_ATtiny84__) | defined(__AVR_ATtiny44__)
#define DDR_USI DDRA
#define PORT_USI PORTA
#define PIN_USI PINA
#define PORT_USI_SDA PORTA6
#define PORT_USI_SCL PORTA4
#define PIN_USI_SDA PINA6
#define PIN_USI_SCL PINA4
#endif

#if defined(__AVR_AT90Tiny2313__) | defined(__AVR_ATtiny2313__)
#define DDR_USI DDRB
#define PORT_USI PORTB
#define PIN_USI PINB
#define PORT_USI_SDA PORTB5
#define PORT_USI_SCL PORTB7
#define PIN_USI_SDA PINB5
#define PIN_USI_SCL PINB7
#endif

/* From the original .h
// Device dependant defines - These for ATtiny2313. // CHANGED FOR ATtiny85

    #define DDR_USI             DDRB
    #define PORT_USI            PORTB
    #define PIN_USI             PINB
    #define PORT_USI_SDA        PORTB0   // was PORTB5 - N/U
    #define PORT_USI_SCL        PORTB2   // was PORTB7 - N/U
    #define PIN_USI_SDA         PINB0    // was PINB5
    #define PIN_USI_SCL         PINB2    // was PINB7
*/

//********** Prototypes **********//

void               USI_TWI_Master_Initialise();
USI_TWI_ErrorLevel USI_TWI_Master_Start_Sending(unsigned char address);
USI_TWI_ErrorLevel USI_TWI_Master_Send(unsigned char msg);
USI_TWI_ErrorLevel USI_TWI_Master_Receive(unsigned char address, unsigned char* buf, unsigned char len);
USI_TWI_ErrorLevel USI_TWI_Master_Stop();
