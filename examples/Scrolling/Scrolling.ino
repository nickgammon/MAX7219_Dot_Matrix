// Demo of MAX7219_Dot_Matrix library - sideways scrolling
// Author: Nick Gammon
// Date: 13 May 2015

// Note: This does not scroll a pixel at a time, but rather a character (8 pixels)
//       Doing a pixel would be more complex (you would need to build up a bitmap
//       in memory. If memory is tight you may not want to do that.
//       If you really want smooth scrolling I'm sure you can make it work. :)

#include <SPI.h>
#include <bitBangedSPI.h>
#include <MAX7219_Dot_Matrix.h>

const byte chips = 8;

// 8 chips, SPI with load on D10
MAX7219_Dot_Matrix display (chips, 10);  // Chips / LOAD 

const char message [] = "Hello there - testing 123456789 ";

void setup ()
  {
  display.begin ();
  }  // end of setup

unsigned long lastMoved = 0;
unsigned long MOVE_INTERVAL = 500;  // mS
unsigned int  messageOffset;

void updateDisplay ()
  {
  display.sendString (& message [messageOffset]);

  // next time show one character on
  if (messageOffset++ >= strlen (message))
    messageOffset = 0;
  }  // end of updateDisplay

void loop () 
  { 
    
  // update display if time is up
  if (millis () - lastMoved >= MOVE_INTERVAL)
    {
    updateDisplay ();
    lastMoved = millis ();
    }

  // do other stuff here    
     
  }  // end of loop

