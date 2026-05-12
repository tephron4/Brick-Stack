#include <LedControl.h>

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn 
 pin 11 is connected to LOAD(CS)
 pin 10 is connected to the CLK 
 We have only a single MAX72XX.
 */
LedControl lc=LedControl(12,10,11,1);

/* Set pin for button */
int playButtonPin=9;
int resetButtonPin=6;

/* Button state variables to handle button press holds */
int lastPlayButtonState=HIGH;

/* we always wait a bit between updates of the display */
unsigned long delayTime=100;

int bricks=4; // Start with 4 dots
int line=0; // Start with dots on the bottom
int position=-4; // Start with dots off the left side
int change=1;
bool gameOver=false;
int ledPositions[4]={-1,-1,-1,-1};

void setup() {
  Serial.begin(9600);
  pinMode(playButtonPin, INPUT_PULLUP);
  pinMode(resetButtonPin, INPUT_PULLUP);
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

void resetMovement() {
  position=0-bricks;
  change=1;
  memset(ledPositions, -1, sizeof(ledPositions));
}

void resetDisplay() {
  lc.setRow(0,0,B00000000);
  lc.setRow(0,1,B00000000);
  lc.setRow(0,2,B00000000);
  lc.setRow(0,3,B00000000);
  lc.setRow(0,4,B00000000);
  lc.setRow(0,5,B00000000);
  lc.setRow(0,6,B00000000);
  lc.setRow(0,7,B00000000);
}

void loop() {
  int newPlayButtonState=digitalRead(playButtonPin);
  // Serial.println("--------");
  // Serial.println(newPlayButtonState);
  // Serial.println(lastPlayButtonState);
  // Serial.println(newPlayButtonState!=lastPlayButtonState);
  // Serial.println("--------");

  if (!gameOver) {
    getLEDPositions();

    // Serial.println(line);
    // Serial.println(position);
    // Serial.println(bricks);
    // Serial.println(change);
    // printLEDPositions();

    lc.setRow(0,line,B00000000); // Clear the line so that the bricks move
    lightBricks();

    if (newPlayButtonState!=lastPlayButtonState) {
      if (newPlayButtonState==LOW) {
        // Serial.println("Button pressed");
        line=line+1;
        resetMovement();

        if (line > 7) {
          // Serial.println("GAME OVER");
          gameOver=true;
        }
      }

      lastPlayButtonState=newPlayButtonState;
    }

    if(digitalRead(resetButtonPin)==LOW) {
      // Serial.println("Reset game");
      /* Reset game */
      bricks=4;
      line=0;
      gameOver=false;
      resetMovement();
      resetDisplay();
    }

    shift();
    delay(delayTime);
  } else {
    // Serial.println("Game is over");
  }

  if(digitalRead(resetButtonPin)==LOW) {
    /* Reset game */
    bricks=4;
    line=0;
    gameOver=false;
    resetMovement();
    resetDisplay();
  }
}
