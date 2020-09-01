/* ******  Modified AR Mar 2013  *******

Modified to use 595 shift register to drive display, rather than direct IO
Reduces number of pins required from 10/6 (8 bit/4 bit) to 3

Wire up 595 as follows:


                          74HC595 
                       ------------------
  	RS (LCD pin 4) <- | 1 QB    Vcc   16 | <- 5v
 	   E (LCD pin 6) <- | 2 QC      QA  15 | -> spare
   D4 (LCD pin 11) <- | 3 QD      SER 14 | <- Arduino pin _data_pins[DATA_PIN_IDX]
   D5 (LCD pin 12) <- | 4 QE      ~OE 13 | <- Gnd
   D6 (LCD pin 13) <- | 5 QF     RCLK 12 | <- Arduino pin _data_pins[LATCH_PIN_IDX]
   D7 (LCD pin 14) <- | 6 QG    SRCLK 11 | <- Arduino pin _data_pins[CLOCK_PIN_IDX]
     				 spare <- | 7 QH   ~SRCLR 10 | <- 5v
               Gnd -> | 8 Gnd      ~QH 9 | -> 74HC595 cascade (N/C)
                       ------------------
                       
Wire up LCD display as follows:


LCD display:

Pin     Symbol	Function
1				Vss			Display power <- Gnd
2				Vdd			Display power <- +5V
3				Vo			Contrast Adjust <- Gnd
4				RS			Register select <- 595 pin 1
5				R/W			Data read/write selector <- Gnd
6				E				Enable strobe <- 595 pin 2
7				DB0			Data Bus 0 - N/C
8				DB1			Data Bus 1 - N/C
9				DB2			Data Bus 2 - N/C
10			DB3			Data Bus 3 - N/C
11			DB4			Data Bus 4 <- 595 pin 3
12			DB5			Data Bus 5 <- 595 pin 4
13			DB6			Data Bus 6 <- 595 pin 5
14			DB7			Data Bus 7 <- 595 pin 6
15			A 			LED backlight power <- +5V
16			K				LED backlight power <- Gnd

*/




#ifndef LiquidCrystal_h
#define LiquidCrystal_h

#include <inttypes.h>
#include "Print.h"

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// AR addition - constants for shift register usage
const uint8_t RS_DATA_LINE		= 1;			// = pin 1 on 595
const uint8_t E_DATA_LINE		  = 2;			// = pin 2 on 595
const uint8_t D4_DATA_LINE		= 3;			// = pin 3 on 595.  D5, D6, & D7 must follow in sequence


class LiquidCrystal : public Print {
public:
  LiquidCrystal(uint8_t rs, uint8_t enable,
		uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
		uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
  LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
		uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
		uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
  LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
		uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
  LiquidCrystal(uint8_t rs, uint8_t enable,
		uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
		
  LiquidCrystal(uint8_t data, uint8_t clock, uint8_t latch);   // AR addition

  void init(uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable,
	    uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	    uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
    
  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);

  void clear();
  void home();
  
  void clearRow(uint8_t rowNum);				// AR mod

  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void leftToRight();
  void rightToLeft();
  void autoscroll();
  void noAutoscroll();

  void createChar(uint8_t, uint8_t[]);
  void setCursor(uint8_t, uint8_t); 
  virtual size_t write(uint8_t);
  void command(uint8_t);
  
  using Print::write;
private:
  void send(uint8_t, uint8_t);
  void write4bits(uint8_t);
  void write8bits(uint8_t);
  void pulseEnable();
  
  void setDataLine(uint8_t dataLine, uint8_t state);			// AR addition
  uint8_t _directIO;																			// True means use normal library functions; false means use shift register
  uint8_t _dataLineState;																	// Holds state of each 595 data line - written out to 595 then latched
  uint8_t _dataPin;																				// Physical Arduino pins used to communicate with 595
  uint8_t _clockPin;
  uint8_t _latchPin;
  
  uint8_t _rs_pin; // LOW: command.  HIGH: character.
  uint8_t _rw_pin; // LOW: write to LCD.  HIGH: read from LCD.
  uint8_t _enable_pin; // activated by a HIGH pulse.
  uint8_t _data_pins[8];

  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;

  uint8_t _initialized;

  uint8_t _numlines,_currline;
  uint8_t _numCols;																				// AR addition
};

#endif
