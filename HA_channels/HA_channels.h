 /*
    Copyright (C) 2011  Andrew Richards
 
    Part of home automation suite
    
    Contains the HA_channel class used to access devices
    
                           
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
		Channel characteristics (see global constants below)
		----------------------------------------------------
		
		Protocol - determines commms type supported by the channel - eg direct IO, serial IO, SPI, TWI
		Access - determines the way the channel provides access to the device - eg direct, muxed etc
		Alert - determines the way the channel raises device alerts - eg polled (ie, no alert) or a range of interrupt types
		
		Illustrative schematics for each channel type
		---------------------------------------------
		
		
		CHAN_PROTOCOL_PIO + CHAN_ACCESS_MUX + CHAN_ALERT_NONE
		-----------------------------------------------------
		    
    Provides up to MAX_PINS virtual non-latching I/O pins accessed through one port polled directly by the main program
    Uses a combination of 74HC4067 muxes and 74HC595 shift registers (2 x muxes per 74HC595). Requires 3 digital ports to control the 74HC595s
    
    Example schematic showing 64 mux pins as one channel
  
                          74HC595 A
                       ------------------
     4067 A pin 11 <- | 1 QB    Vcc   16 | <- 5v
     4067 A pin 14 <- | 2 QC      QA  15 | -> 4067 A pin 10
     4067 A pin 13 <- | 3 QD      SER 14 | <- Mega pin 10 (data in)
     4067 B pin 10 <- | 4 QE      ~OE 13 | <- Gnd
     4067 B pin 11 <- | 5 QF     RCLK 12 | <- Mega pin 12 (latch)
     4067 B pin 14 <- | 6 QG    SRCLK 11 | <- Mega pin 11 (clock)
     4067 B pin 13 <- | 7 QH   ~SRCLR 10 | <- 5v
               Gnd -> | 8 Gnd      ~QH 9 | -> 74HC595 B pin 14 (data out)
               				 ------------------
                         
				                          74HC4067 A
				                       -----------------
				 Mega data pin(s) <-> | 1 I/O    Vcc 24 | <- 5v
				           				<-> | 2 I7      I8 23 | <->
				                      | 3            22 |
				                      | 4            21 |
				                      | 5            20 |
				                      | 6            19 |
				                      | 7            18 |
				                      | 8            17 |
				        Mux pin 0 <-> | 9 I0     I15 16 | <-> Mux pin 15
				  74HC595 A pin 15 -> | 10 S0     ~E 15 | <- Gnd
				   74HC595 A pin 1 -> | 11 S1     S2 14 | <- 74HC595 A pin 2 (QC)
				               Gnd -> | 12 Gnd    S3 13 | <- 74HC595 A pin 3 (QD)
				                       -----------------
				
				                          74HC4067 B
				                       -----------------
				 Mega data pin(s) <-> | 1 I/O    Vcc 24 | <- 5v
				           				<-> | 2 I7      I8 23 | <->
				                      | 3            22 |
				                      | 4            21 |
				                      | 5            20 |
				                      | 6            19 |
				                      | 7            18 |
				                      | 8            17 |
				       Mux pin 16 <-> | 9 I0     I15 16 | <-> Mux pin 31
				   74HC595 A pin 4 -> | 10 S0     ~E 15 | <- Gnd
				   74HC595 A pin 5 -> | 11 S1     S2 14 | <- 74HC595 pin 6 (QG)
				               Gnd -> | 12 Gnd    S3 13 | <- 74HC595 pin 7 (QH)
				                       -----------------
        
                       74HC595 B
                       ------------------
     4067 C pin 11 <- | 1 QB    Vcc   16 | <- 5v
     4067 C pin 14 <- | 2 QC      QA  15 | -> 4067 C pin 10
     4067 C pin 13 <- | 3 QD      SER 14 | <- 74HC595 A pin 9 (data in)
     4067 D pin 10 <- | 4 QE      ~OE 13 | <- Gnd
     4067 D pin 11 <- | 5 QF     RCLK 12 | <- Mega pin 12 (latch)
     4067 D pin 14 <- | 6 QG    SRCLK 11 | <- Mega pin 11 (clock)
     4067 D pin 13 <- | 7 QH   ~SRCLR 10 | <- 5v
               Gnd -> | 8 Gnd      ~QH 9 |
                       ------------------ 
    
				                          74HC4067 C
				                       -----------------
				 Mega data pin(s) <-> | 1 I/O    Vcc 24 | <- 5v
				           				<-> | 2 I7      I8 23 | <->
				                      | 3            22 |
				                      | 4            21 |
				                      | 5            20 |
				                      | 6            19 |
				                      | 7            18 |
				                      | 8            17 |
				        Mux pin 0 <-> | 9 I0     I15 16 | <-> Mux pin 15
				  74HC595 B pin 15 -> | 10 S0     ~E 15 | <- Gnd
				   74HC595 B pin 1 -> | 11 S1     S2 14 | <- 74HC595 B pin 2 (QC)
				               Gnd -> | 12 Gnd    S3 13 | <- 74HC595 B pin 3 (QD)
				                       -----------------
				
				                          74HC4067 D
				                       -----------------
				 Mega data pin(s) <-> | 1 I/O    Vcc 24 | <- 5v
				           				<-> | 2 I7      I8 23 | <->
				                      | 3            22 |
				                      | 4            21 |
				                      | 5            20 |
				                      | 6            19 |
				                      | 7            18 |
				                      | 8            17 |
				       Mux pin 16 <-> | 9 I0     I15 16 | <-> Mux pin 31
				   74HC595 B pin 4 -> | 10 S0     ~E 15 | <- Gnd
				   74HC595 B pin 5 -> | 11 S1     S2 14 | <- 74HC595 pin 6 (QG)
				               Gnd -> | 12 Gnd    S3 13 | <- 74HC595 pin 7 (QH)
				                       -----------------
				                       
				                       
 		CHAN_PROTOCOL_PIO + CHAN_ACCESS_POWER
		-------------------------------------
		    
    Provides up to MAX_PINS virtual latching output pins driven through three physical pins.
    TPIC6595 shift registers can sink 250mA @ 45v per pin and used to switch 12v relays
    Can be linked together
    
    Example schematic showing 16 pins as one channel
  
                     
		                          TPIC6595 A
		                       -----------------------
		 							 Gnd -> | 1       		  PGND 20 | <- Gnd
		           			5v -> | 2 Vcc   	 	  LGND 19 | <- Gnd
		       Mega pin 10 -> | 3 Ser in   Ser out 18 | -> TPIC6595 B pin 3
		           Relay 0 -> | 4 Drain 0  Drain 7 17 | <- Relay 7
		           Relay 1 -> | 5 Drain 1  Drain 6 16 | <- Relay 6
		           Relay 2 -> | 6 Drain 2  Drain 5 15 | <- Relay 5
		           Relay 3 -> | 7 Drain 3  Drain 4 14 | <- Relay 4
		                5v -> | 8 ~SRCLR      SRCK 13 | <- Mega pin 11 (clock)
		        			 Gnd -> | 9 ~G     	 		 RCK 12 | <- Mega pin 12 (latch)
		  						 Gnd -> | 10 PGND       PGND 11 | <- Gnd
		                       -----------------------
		                       
															TPIC6595 B
		                       -----------------------
		 							 Gnd -> | 1       		  PGND 20 | <- Gnd
		           			5v -> | 2 Vcc   		  LGND 19 | <- Gnd
		 TPIC6595 B pin 18 -> | 3 Ser in 	 Ser out 18 | 
		           Relay 8 -> | 4 Drain 0  Drain 7 17 | <- Relay 15
		           Relay 9 -> | 5 Drain 1  Drain 6 16 | <- Relay 14
		          Relay 10 -> | 6 Drain 2  Drain 5 15 | <- Relay 13
		          Relay 11 -> | 7 Drain 3  Drain 4 14 | <- Relay 12
		                5v -> | 8 ~SRCLR      SRCK 13 | <- Mega pin 11 (clock)
		        			 Gnd -> | 9 ~G     			 RCK 12 | <- Mega pin 12 (latch)
		  						 Gnd -> | 10 PGND       PGND 11 | <- Gnd
		                       -----------------------
     
		                           
		CHAN_PROTOCOL_SIO + CHAN_ACCESS_MUX + CHAN_ALERT_INT
		----------------------------------------------------
		
		Provides two replicated mux channels to support RX/TX - single set of shift registers drive parallel muxes
		Alert is provided through direct interrupt         
		
    Example schematic showing 2 muxes providing 16 serial ports; illustrating ePIR on pin 7 driven by Serial2 (phys pins 16 & 17) and driving interrupt 1 (pin 3)
  
                          74HC595 A
                       ------------------
 4067 RX/TX pin 11 <- | 1 QB    Vcc   16 | <- 5v
 4067 RX/TX pin 14 <- | 2 QC      QA  15 | -> 4067 RX/TX pin 10
 4067 RX/TX pin 13 <- | 3 QD      SER 14 | <- Mega pin 10 (data in)
     							 <- | 4 QE      ~OE 13 | <- Gnd
     							 <- | 5 QF     RCLK 12 | <- Mega pin 12 (latch)
     							 <- | 6 QG    SRCLK 11 | <- Mega pin 11 (clock)
     							 <- | 7 QH   ~SRCLR 10 | <- 5v
               Gnd -> | 8 Gnd      ~QH 9 | -> 
               				 ------------------
                         
				                          74HC4067 RX
				                       -----------------
				 		Mega RX pin 17 <- | 1 I/O    Vcc 24 | <- 5v
				        ePIR pin 4 -> | 2 I7      I8 23 | <-
				                      | 3            22 |
				                      | 4            21 |
				                      | 5            20 |
				                      | 6            19 |
				                      | 7            18 |
				                      | 8            17 |
				         Mux pin 0 -> | 9 I0     I15 16 | <- Mux pin 15
				  74HC595 A pin 15 -> | 10 S0     ~E 15 | <- Gnd
				   74HC595 A pin 1 -> | 11 S1     S2 14 | <- 74HC595 A pin 2 (QC)
				               Gnd -> | 12 Gnd    S3 13 | <- 74HC595 A pin 3 (QD)
				                       -----------------
				
				                          74HC4067 TX
				                       -----------------
				 		Mega TX pin 16 -> | 1 I/O    Vcc 24 | <- 5v
				        ePIR pin 3 <- | 2 I7      I8 23 | ->
				                      | 3            22 |
				                      | 4            21 |
				                      | 5            20 |
				                      | 6            19 |
				                      | 7            18 |
				                      | 8            17 |
				        Mux pin 0 <-  | 9 I0     I15 16 | -> Mux pin 15
				  74HC595 A pin 15 -> | 10 S0     ~E 15 | <- Gnd
				   74HC595 A pin 1 -> | 11 S1     S2 14 | <- 74HC595 A pin 2 (QC)
				               Gnd -> | 12 Gnd    S3 13 | <- 74HC595 A pin 3 (QD)
				                       -----------------

				        					Zilog ePIR
				                       
										GND pin 1 || <- GND                                                                        
                    VDD pin 2 || <- 3.3v                                                                       
                RXD/DLY pin 3 || <- TXD 74HC4067 TX pin 2
                TXD/SNS pin 4 || -> RXD 74HC4067 RX pin 2
                							|| <- 100k <- 3.3v
                 MD/RST pin 5 || -> Mega pin 3 (Interrupt 1)
                     LG pin 6 || <- 3.3v 
                SLP/DBG pin 7 || <- 3.3v
                    GND pin 8 || <- GND                                                                        
                                                                                                              
             (Pins 1 & 8 are internally connected on the ePIR and only one needs to be connected)                                                                                                                           
            **Also**   a 100K resistor needs to be connected from TXD/SNS pin of the ePIR to 3.3V             
                (without the resistor the device will not communicate with the Arduino board)    
  
		CHAN_ALERT_SINT
		---------------    
		
		Alert provided through single MCP23S17 (up to 16 interrupt lines)
		
		          				       Zilog ePIR
				                       
										GND pin 1 || <- GND                                                                        
                    VDD pin 2 || <- 3.3v                                                                       
                RXD/DLY pin 3 || <- TXD 74HC4067 TX pin 2
                TXD/SNS pin 4 || -> RXD 74HC4067 RX pin 2
                							|| <- 100k <- 3.3v
                 MD/RST pin 5 || -> MCP23S17 pin 21
                     LG pin 6 || <- 3.3v 
                SLP/DBG pin 7 || <- 3.3v
                    GND pin 8 || <- GND
                    
                
		                 MCP23S17 - Interrupt handler
                       -----------------
                      | 1 B0      A7 28 |
                      | 2 B1         27 |
                      | 3            26 |
                      | 4            25 |
                      | 5            24 |
                      | 6            23 | <- 
                      | 7            22 | <- 
                      | 8 B7      A0 21 | <- ePIR pin 5 
               +5v -> | 9 Vdd   INTA 20 | -> Arduino pin 3 (Interrupt 1)
               Gnd -> | 10 Vss  INTB 19 |
    Arduino pin 30 -> | 11 CS  RESET 18 | <- Arduino pin 29
    Arduino pin 52 -> | 12 SCK    A2 17 | <- Gnd
    Arduino pin 51 -> | 13 SI     A1 16 | <- Gnd
    Arduino pin 50 -> | 14 SO     A0 15 | <- Gnd
                       -----------------
		
                       
		CHAN_ALERT_MINT
		---------------                       

		Alert provided through a 2-level cascade of MCP23S17s, with one MCP23S17 acting as a mux for up to 16 slave devices,
		thus allowing up to 256 interrupt lines.  Slave devices addressed directly in two banks of 8 (3 bit addressing)

                         MCP23S17 MUX CS0 - Interrupt MUX
                       -----------------
  MCP23 008 pin 20 -> | 1 B0      A7 28 |
                      | 2 B1         27 |
                      | 3            26 |
                      | 4            25 |
                      | 5            24 |
                      | 6            23 | <- MCP23 002 pin 20
                      | 7            22 | <- MCP23 001 pin 20
                      | 8 B7      A0 21 | <- MCP23 000 pin 20
               +5v -> | 9 Vdd   INTA 20 | -> Arduino pin 3 (Interrupt 1)
               Gnd -> | 10 Vss  INTB 19 |
    Arduino pin 30 -> | 11 CS  RESET 18 | <- Arduino pin 29
    Arduino pin 52 -> | 12 SCK    A2 17 | <- Gnd
    Arduino pin 51 -> | 13 SI     A1 16 | <- Gnd
    Arduino pin 50 -> | 14 SO     A0 15 | <- Gnd
                       -----------------
    
Bank one slaves:

                                           MCP23S17 000 CS1
                                         -----------------
                                        | 1 B0      A7 28 |
                         							  | 2 B1         27 |
                                        | 3            26 |
                                        | 4            25 |
                                        | 5            24 |
                                        | 6            23 |
                                        | 7            22 |
                                        | 8 B7      A0 21 | 
                                 +5v -> | 9 Vdd   INTA 20 | -> MCP23 MUX pin 21
                                 Gnd -> | 10 Vss  INTB 19 |
                      Arduino pin 31 -> | 11 CS  RESET 18 | <- Arduino pin 29
                      Arduino pin 52 -> | 12 SCK    A2 17 | <- Gnd
                      Arduino pin 51 -> | 13 SI     A1 16 | <- Gnd
                      Arduino pin 50 -> | 14 SO     A0 15 | <- Gnd
                                         -----------------
                                         
                                           MCP23S17 001 CS1
                                         -----------------
                                        | 1 B0      A7 28 | <- ATMega328 pin 15
                         								| 2 B1         27 | <- ePIR pin 5
                         								| 3            26 |
                                        | 4            25 |
                                        | 5            24 |
                                        | 6            23 |
                                        | 7            22 |
                                        | 8 B7      A0 21 |
                                 +5v -> | 9 Vdd   INTA 20 | -> MCP23 MUX pin 22
                                 Gnd -> | 10 Vss  INTB 19 |
                      Arduino pin 31 -> | 11 CS  RESET 18 | <- Arduino pin 29
                      Arduino pin 52 -> | 12 SCK    A2 17 | <- Gnd
                      Arduino pin 51 -> | 13 SI     A1 16 | <- Gnd
                      Arduino pin 50 -> | 14 SO     A0 15 | <- +5v
                                         -----------------
                  
                                           MCP23S17 002 CS1
                                         -----------------
                                        | 1 B0      A7 28 | 
                                        | 2 B1         27 |
                                        | 3            26 |
                                        | 4            25 |
                                        | 5            24 |
                                        | 6            23 |
                                        | 7            22 |
                                        | 8 B7      A0 21 |
                                 +5v -> | 9 Vdd   INTA 20 | -> MCP23 MUX pin 23
                                 Gnd -> | 10 Vss  INTB 19 |
                      Arduino pin 31 -> | 11 CS  RESET 18 | <- Arduino pin 29
                      Arduino pin 52 -> | 12 SCK    A2 17 | <- Gnd
                      Arduino pin 51 -> | 13 SI     A1 16 | <- +5v
                      Arduino pin 50 -> | 14 SO     A0 15 | <- Gnd
                                         -----------------
                  
Bank two slaves, as above for bank one, but:
                                         
                                         MCP23S17 008 CS1
                                         -----------------
                                        | 1 B0      A7 28 | 
                                        | 2 B1         27 |
                                        | 3            26 |
                                        | 4            25 |
                                        | 5            24 |
                                        | 6            23 |
                                        | 7            22 |
                                        | 8 B7      A0 21 |
                                 +5v -> | 9 Vdd   INTA 20 | -> MCP23 MUX pin 1     <<<<<<<<<<<
                                 Gnd -> | 10 Vss  INTB 19 |
         >>>>>>>>>    Arduino pin 32 -> | 11 CS  RESET 18 | <- Arduino pin 29
                      Arduino pin 52 -> | 12 SCK    A2 17 | <- Gnd
                      Arduino pin 51 -> | 13 SI     A1 16 | <- +5v
                      Arduino pin 50 -> | 14 SO     A0 15 | <- Gnd
                                         -----------------
                  
*/
 
#ifndef HA_channels_h
#define HA_channels_h

#include "HA_globals.h"
#include "Mcp23s17.h"
#include "Bitstring.h"


static volatile unsigned int counterInts;
		
// ************* Global constants


static const byte VAL_CHAN_TYPE 			= 0;
static const byte VAL_CHAN_IO_PIN 		= 1;
static const byte VAL_CHAN_LATCH_PIN 	= 2;
static const byte VAL_CHAN_DATA_PIN 	= 3;
static const byte VAL_CHAN_CLOCK_PIN 	= 4;
static const byte VAL_CHAN_MAX_PIN 		= 5;
static const byte VAL_CHAN_PROTOCOL   = 6;
static const byte VAL_CHAN_ACCESS		  = 7;
static const byte VAL_CHAN_ALERT   		= 8;

static const byte CHAN_PROTOCOL_PIO		= 0x00;					// Single Pin (digital or analog)
static const byte CHAN_PROTOCOL_SIO		= 0x01;					// HardwareSerial
static const byte CHAN_PROTOCOL_SPI		= 0x02;					// SPI 
static const byte CHAN_PROTOCOL_TWI		= 0x03;					// Twin Wire 

static const byte CHAN_ACCESS_DIRECT	= 0x00 << 2;		// Direct access to pin(s)
static const byte CHAN_ACCESS_MUX			= 0x01 << 2;		// Non-latching io using 4067 mux access controlled by 74HC595 shift registers 
static const byte CHAN_ACCESS_LATCH		= 0x02 << 2;		// Latching IO using MCP23s17 (low power)
static const byte CHAN_ACCESS_POWER		= 0x03 << 2;		// Latching output using power shift registers (TPIC6595) and relays 

static const byte CHAN_ALERT_NONE			= 0x00 << 4;		// No interrupt
static const byte CHAN_ALERT_INT			= 0x01 << 4;		// Single interrupt
static const byte CHAN_ALERT_SINT			= 0x02 << 4;		// Up to 16 interrupts from single MCP23s17
static const byte CHAN_ALERT_MINT			= 0x03 << 4;		// Up to 256 interrupts from two level cascade of MCP23s17s

static const byte UNLOCKED = 0;
static const byte LOCKED   = 1;

static const byte OBJ_TYPE_CHANNEL			= 100;				// Arbitrary value to avoid clash with DEV_TYPE_n and VAR_TYPE_n
static const byte MAX_MUX_PINS 					= 64;
//static const byte NUM_MCP23_SLAVES 			= 16;

// SPI Interface common lines:
    
static const byte SPI_MISO             	= 50; 				//arduino   <->   SPI Master In Slave Out   -> SO  (Pin 14 on MCP23S17 DIP)
static const byte SPI_MOSI             	= 51; 				//arduino   <->   SPI Master Out Slave In   -> SI  (Pin 13 on MCP23S17 DIP)
static const byte SPI_CLOCK            	= 52; 				//arduino   <->   SPI Slave Clock Input     -> SCK (Pin 12 on MCP23S17 DIP) 

static const int DAEMON_FREQUENCY				= 100;				// Wakeup every 100ms to check if any interrupts received
	
static const byte NUM_INTERRUPT_RANGES_SINT	= 4;			// Number of segments into which single interrupt channel can be divided
static const byte NUM_INTERRUPT_RANGES_MINT	= 4;			// Number of segments into which multi-interrupt channel can be divided


// *********** Classes


class HA_channel {
	public:
		boolean initChan(byte protocolType, byte maxPins, byte ioPin);	
		boolean initChanAccess(byte accessType, byte latchPin, byte dataPin, byte clockPin);
		
		boolean initChanAlert(byte alertType, byte chanNum, byte intNum, byte intMode);
		boolean initChanAlert(byte alertType, byte chanNum, byte intNum, byte intMode, byte resetPin, byte slaveSelectPin);
		boolean initChanAlert(byte alertType, byte chanNum, byte intNum, byte intMode, byte resetPin, byte slaveSelectPin, byte slaveSelectPinBank0, byte slaveSelectPinBank1);
		void put(byte type, byte val);
		byte get(byte type);	
		HA_channel *getChanObj(); 
		boolean registerDevRange(byte rangeNum, byte intPin, byte devType, byte devNum);
		unsigned int findIntPin(byte devNum);
		boolean enablePin(byte pin);
		boolean digitalWrite(byte pin, byte val);
		boolean intEnable(byte intPin);
		boolean intMode(byte intPin, byte mode);
		void chanISR();
		void chanDaemon(byte arg);
	
		boolean lock();
		boolean unlock();	

	private:
	
		// Constants
		static const byte CHAN_PROTOCOL_MASK				= B00000011;				// Comms protocol
		static const byte CHAN_ACCESS_MASK 					= B00001100;				// Method of accessing the device
		static const byte CHAN_ALERT_MASK 					= B00110000;				// Way in which device alerts input
		static const byte NUM_MINT_SLAVES						= 3;								// Number of slaves for multi-interrupt handler - max 16		
		static const byte NUM_SLAVES_PER_BANK				= 8;
		static const byte NUM_LINES_PER_SLAVE				= 16;
//		static const byte TYPE_74HC595							= 1;
//		static const byte TYPE_TPIC6595							= 2;
		
		// Properties common to all channel types
		byte _chanType;							// See CHAN_PROTOCOLxxx, CHAN_ACCESSxxx & CHAN_ALERTxxx above
		byte _maxPin;								// Max number of pin accessible through mux	- 1 (maxPins - 1)
		byte _inUse;								// Implements a mutex on the channel		
		
		// Protcol properties
		byte _ioPin;								// Physical pin through which data is transferred to virtual pin, or index of HardwareSerial
		HardwareSerial *serialPtr;	// Serial0, Serial1, Serial2 or Serial3
		
		// Access properties
		byte _latchPin;							// Shift register pin - HIGH latches data to output pins
		byte _dataPin;							// Shift register pin - loads data one bit at a time
		byte _clockPin;							// Shift register pin - each clock pulse shifts the register and loads _dataPin to QA		
		byte _numShifts;						// Number of shift registers
		BITSTRING *pinStates;				// Used for CHAN_ACCESS_POWER pins
		
		// Alert properties
		struct structINT {					// Additional fields for CHAN_ALERT_INT
			byte devType;
			byte devNum;
			volatile boolean intRaised;
		};
		
		struct structIntRange {			// SINT and MINT interrupt handlers can be used by multiple device types, each occupying range comprising a sequential series of pins synchronised with a series of device numbers
			byte intPin;							// Starting (virtual) interrupt pin for range.  End interrupt pin = intPin of next range - 1
			byte devType;							// Device type proper to range
			byte devNum;							// Device number mapped to intPin.  devNum + n maps to intPin + n 
		};
		
		struct structSINT {					// Additional fields for CHAN_ALERT_SINT
			structIntRange intRange[NUM_INTERRUPT_RANGES_SINT];
			byte slaveSelectPin;
			MCP23S17 MCP23SINT;											// Single device
			volatile unsigned int flagBitsSINT;			// Holds 16 bits corresponding to the interrupt status (1 = interrupt) of each of the lines
		};
		
		struct structMINT {					// Additional fields for CHAN_ALERT_MINT
			structIntRange intRange[NUM_INTERRUPT_RANGES_MINT];
			byte slaveSelectPinMux;
			byte slaveSelectPinBank0;
			byte slaveSelectPinBank1;
			MCP23S17 MCP23MUX;																	// Single device for muxing between up to 16 slave devices
			MCP23S17 MCP23SLAVE[NUM_MINT_SLAVES];								// Multiple slaves
			volatile unsigned int flagBitsMUX;
			volatile unsigned int flagBits[NUM_MINT_SLAVES];		// Holds 16 bits corresponding to the interrupt status (1 = interrupt) of each of the lines on each slave
		};
		
		union {											// Ptr to malloced data appropriate to channel alert type (if interrupt driven)
			structINT *chan_alert_int;
			structSINT *chan_alert_sint;
			structMINT *chan_alert_mint;
		};

		// Methods
		void startDaemon();
		void invokeDevInt(byte alertingPin);

		friend class HA_devMotion;	// Allows class to access serialPtr

};


typedef void (HA_channel::*HA_channelMemPtr)(byte);
int saveContext(HA_channel *object, HA_channelMemPtr fPtr, byte arg = 0, byte repeated = 0);
void switcherChan(void *context);

// *********** Interrupt handling methods ******************

void registerChanISR(byte intNum, byte chanNum, byte intMode);				// Register ISR and associated chanNum to be called on receipt of interrupt on intNum (Int 0 = pin 2; 1 = 3; 2 = 21; 3 = 20; 4 = 19; 5 = 18)
void ISR0();																													// Interrupt Service Routine invoked on interrupt 0 - calls ISRChanNum[0]
void ISR1();
void ISR2();
void ISR3();
void ISR4();
void ISR5();

		
// *********** VARIABLES *****************

static byte ISRChanNum[NUM_INTERRUPT_PORTS];					// Holds the number of the channnel to invoke on receipt of interrupt [n]


#endif
