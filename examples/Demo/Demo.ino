// Demo of MAX7219_Dot_Matrix library
// Author: Nick Gammon
// Date: 13 May 2015

#include <SPI.h>
#include <bitBangedSPI.h>
#include <MAX7219_Dot_Matrix.h>

MAX7219_Dot_Matrix myDisplay (8, 10);  // 8 chips, and then specify the LOAD pin only

void setup ()
  {  
  myDisplay.begin ();
  myDisplay.setIntensity (15);
  myDisplay.sendString ("Hello there");
  }  // end of setup

void loop ()
  {
  // do whatever here
  }  // end of loop
  
