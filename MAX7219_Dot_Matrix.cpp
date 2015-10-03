/*

 MAX7219_Dot_Matrix class
 Author: Nick Gammon
 Date:   13 May 2015


 PERMISSION TO DISTRIBUTE

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.


 LIMITATION OF LIABILITY

 The software is provided "as is", without warranty of any kind, express or implied,
 including but not limited to the warranties of merchantability, fitness for a particular
 purpose and noninfringement. In no event shall the authors or copyright holders be liable
 for any claim, damages or other liability, whether in an action of contract,
 tort or otherwise, arising from, out of or in connection with the software
 or the use or other dealings in the software.

*/

#include <SPI.h>
#include <bitBangedSPI.h>
#include <MAX7219_Dot_Matrix.h>
#include <MAX7219_Dot_Matrix_font.h>

/*

Wiring:

  Hardware SPI:

  Wire DIN (data) to the MOSI pin (D11 on a Uno)
  Wire CLK (clock) to the SCK pin (D13 on a Uno)
  Wire LOAD to the /SS (slave select) pin (D10 on a Uno)

  Make an instance of the class:

    MAX7219_Dot_Matrix myDisplay (1, 10);  // 1 chip, and then specify the LOAD pin only

  Bit-banged SPI:

  Wire LOAD, DIN, CLK to any pins of your choice.

  Make an instance of the class:

    MAX7219_Dot_Matrix myDisplay (2, 6, 7, 8);  // 2 chips, then specify the LOAD, DIN, CLK pins

Usage:

  Initialize:

    myDisplay.begin ();

  Shut down:

    myDisplay.end ();

  Write to display:

    myDisplay.sendString ("Hello there");

  Smooth scrolling:

    myDisplay.sendSmooth.("Hello there", 10);  // start at pixel 10

  Set the intensity (from 0 to 15):

    myDisplay.setIntensity (8);

  For the class to compile you need to include these three files:

      SPI.h
      bitBangedSPI.h
      MAX7219_Dot_Matrix.h

  You can obtain the bitBangedSPI library from:

    https://github.com/nickgammon/bitBangedSPI

*/



// destructor
MAX7219_Dot_Matrix::~MAX7219_Dot_Matrix ()
  {
  end ();
  } // end of destructor

void MAX7219_Dot_Matrix::begin ()
  {
  pinMode (load_, OUTPUT);
  digitalWrite (load_, HIGH);

  // prepare SPI
  if (bitBanged_)
    {
    if (bbSPI_ == NULL)
      bbSPI_ = new bitBangedSPI (din_, bitBangedSPI::NO_PIN, clock_);
    bbSPI_->begin ();
    } // end of bit banged SPI
  else
    {  // hardware SPI
    SPI.begin ();
//    SPI.setClockDivider (SPI_CLOCK_DIV8);
    }  // end of hardware SPI

  // repeatedly configure the chips in case they start up in a mode which
  // draws a lot of current
  for (byte chip = 0; chip < chips_; chip++)
    {
    sendToAll (MAX7219_REG_SCANLIMIT, 7);     // show 8 digits
    sendToAll (MAX7219_REG_DECODEMODE, 0);    // use bit patterns
    sendToAll (MAX7219_REG_DISPLAYTEST, 0);   // no display test
    sendToAll (MAX7219_REG_INTENSITY, 15);    // character intensity: range: 0 to 15
    sendString ("");                          // clear display
    sendToAll (MAX7219_REG_SHUTDOWN, 1);      // not in shutdown mode (ie. start it up)
    }
  } // end of MAX7219_Dot_Matrix::begin

void MAX7219_Dot_Matrix::end ()
  {
  sendToAll (MAX7219_REG_SHUTDOWN, 0);  // shutdown mode (ie. turn it off)

  if (bbSPI_ != NULL)
    {
    delete bbSPI_;
    bbSPI_ = NULL;
    }

  if (!bitBanged_)
    SPI.end ();

  } // end of MAX7219_Dot_Matrix::end

void MAX7219_Dot_Matrix::setIntensity (const byte amount)
  {
  sendToAll (MAX7219_REG_INTENSITY, amount & 0xF);     // character intensity: range: 0 to 15
  } // end of MAX7219_Dot_Matrix::setIntensity

// send one byte to MAX7219
void MAX7219_Dot_Matrix::sendByte (const byte reg, const byte data)
  {
  if (bitBanged_)
    {
    if (bbSPI_ != NULL)
      {
      bbSPI_->transfer (reg);
      bbSPI_->transfer (data);
      }
    }
  else
    {
    SPI.transfer (reg);
    SPI.transfer (data);
    }
  }  // end of sendByte

void MAX7219_Dot_Matrix::sendToAll (const byte reg, const byte data)
  {
  digitalWrite (load_, LOW);
  for (byte chip = 0; chip < chips_; chip++)
    sendByte (reg, data);
  digitalWrite (load_, HIGH);
  }  // end of sendToAll

void MAX7219_Dot_Matrix::sendChar (const byte chip, const byte data)
  {
  // get this character from PROGMEM
  byte pixels [8];
  for (byte i = 0; i < 8; i++)
     pixels [i] = pgm_read_byte (&MAX7219_Dot_Matrix_font [data] [i]);

  send64pixels (chip, pixels);
  }  // end of sendChar

// send one character (data) to position (chip)
void MAX7219_Dot_Matrix::send64pixels (const byte chip, const byte pixels [8])
  {
  for (byte col = 0; col < 8; col++)
    {
    // start sending
    digitalWrite (load_, LOW);
    // send extra NOPs to push the pixels out to extra displays
    for (byte i = 0; i < chip; i++)
      sendByte (MAX7219_REG_NOOP, MAX7219_REG_NOOP);
    // rotate pixels 90 degrees
    byte b = 0;
    for (byte i = 0; i < 8; i++)
      b |= bitRead (pixels [i], col) << (7 - i);
    sendByte (col + 1, b);
    // end with enough NOPs so later chips don't update
    for (int i = 0; i < chips_ - chip - 1; i++)
      sendByte (MAX7219_REG_NOOP, MAX7219_REG_NOOP);
    // all done!
    digitalWrite (load_, HIGH);
    }   // end of for each column
  }  // end of sendChar

// write an entire null-terminated string to the LEDs
void MAX7219_Dot_Matrix::sendString (const char * s)
{
  byte chip;

  for (chip = 0; chip < chips_ && *s; chip++)
    sendChar (chip, *s++);

 // space out rest
  while (chip < (chips_))
    sendChar (chip++, ' ');

}  // end of sendString

void MAX7219_Dot_Matrix::sendSmooth (const char * s, const int pixel)
  {
  int len = strlen (s);
  byte thisChip [3 * 8];  // pixels for current chip with allowance for one each side
  int firstByte = pixel / 8;
  int offset = pixel - (firstByte * 8);

  byte chip;

  for (chip = 0; chip < chips_; chip++)
    {
    memset (thisChip, 0, sizeof thisChip);

    // get pixels to left of current character in case "pixel" is negative
    if (offset < 0)
      {
      if (firstByte + chip - 1 >= 0 && firstByte + chip - 1 < len)
        for (byte i = 0; i < 8; i++)
           thisChip [i] = pgm_read_byte (&MAX7219_Dot_Matrix_font [s [firstByte  + chip - 1]] [i]);
      }  // negative offset

    // get the current character
    if (firstByte + chip >= 0 && firstByte + chip < len)
      for (byte i = 0; i < 8; i++)
         thisChip [i + 8] = pgm_read_byte (&MAX7219_Dot_Matrix_font [s [firstByte + chip]] [i]);

    // get pixels to right of current character in case "pixel" is positive
    if (offset > 0)
      {
      if (firstByte + chip + 1 >= 0 && firstByte + chip + 1 < len)
        for (byte i = 0; i < 8; i++)
           thisChip [i + 16] = pgm_read_byte (&MAX7219_Dot_Matrix_font [s [firstByte + chip + 1]] [i]);
      }  // positive offset

    // send the appropriate 8 pixels (offset will be from -7 to +7)
    send64pixels (chip, &thisChip [8 + offset]);

    } // for each chip

  } // end of MAX7219_Dot_Matrix::sendSmooth
