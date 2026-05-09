#include "LedControl.h"

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn 
 pin 11 is connected to LOAD(CS)
 pin 10 is connected to the CLK 
 We have only a single MAX72XX.
 */
LedControl lc=LedControl(12,10,11,1);

/* we always wait a bit between updates of the display */
unsigned long delaytime=100;

int bricks=4; // Start with 4 dots
int line=0; // Start with dots on the bottom
int position=-4; // Start with dots off the left side
int change=1;
int ledPositions[4]={-1,-1,-1,-1};

void setup() {
  Serial.begin(9600);
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  /* and clear the display */
  lc.clearDisplay(0);
}

/*
  This function handles shifting the lights
  along the current line given the current
  position and number of lights.
*/
void shift() {
  if(position==0-bricks) { // change directions after going off the left side
    change=1;
  } else if (position==8) { // change directions after going off the right side
    change=-1;
  }
  position=position+change;
}

/*
  This function returns a list of the positions
  of the lights to turn on.
*/
void getLEDPositions() {
  for(int i=0; i<bricks; i++){
    ledPositions[i] = position+i;
  }
}

/*
  This function lights up the bricks.
*/
void lightBricks() {
  lc.setRow(0,line,B00000000); // Clear the line so that the bricks move
  for(int i=0; i<bricks; i++) {
    int ledPos=ledPositions[i];
    if(ledPos>=0 && ledPos<=7) {
      lc.setLed(0,line,ledPos,true);
    }
  }
}

void printLEDPositions() {
  Serial.print("[");
  for (int i=0; i<4; i++) {
    Serial.print(ledPositions[i]);
    if (i!=3) {
      Serial.print(", ");
    }
  }
  Serial.println("]");
}

void loop() { 
  getLEDPositions();

  Serial.println(line);
  Serial.println(position);
  Serial.println(bricks);
  Serial.println(change);
  printLEDPositions();

  lightBricks();
  shift();
  delay(delaytime);
}
