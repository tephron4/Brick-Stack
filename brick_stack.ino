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
int gameBoard[8][4];

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

  /* Set the gameboard to default (-1 for all) */
  resetGameBoard();
  printGameBoard();
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
  memset(ledPositions, -1, sizeof(ledPositions));
  for(int i=0; i<bricks; i++){
    ledPositions[i] = position+i;
  }
}

/*
  This function lights up the board.
*/
void lightGameBoard() {
  for (int i=0; i<8; i++) {
    lc.setRow(0, i, B00000000);
    for (int j=0; j<4; j++) {
      int position=i==line ? ledPositions[j] : gameBoard[i][j];
      if (position>=0 && position<=7) {
        lc.setLed(0, i, position, true);
      }
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

void resetGameBoard() {
  memset(gameBoard, -1, sizeof(gameBoard));
}

void handlePlayButtonPress() {
  for (int i=0; i<bricks; i++) {
    /* Handle bricks outside of grid */
    if (ledPositions[i]<0 || ledPositions[i]>7) {
      bricks-=1;
    }
  }
  /* Handle bricks not on top of other bricks */
  if (line>0) {
    checkBricks();
  }
}

void checkBricks() {
  int previousLine[4]={-1,-1,-1,-1};
  memcpy(previousLine, gameBoard[line-1], sizeof(previousLine));
  /* Loop through led positions */
  for (int i=0; i<4; i++) {
    bool found = false;
    /* Loop through last line */
    for (int j=0; j<4; j++) {
      // Serial.print("Checking ");
      // Serial.println(ledPositions[i]);
      if (previousLine[j]==ledPositions[i]) {
        // Serial.print(ledPositions[i]);
        // Serial.println(" found");
        found = true;
      }
    }
    
    if (!found) {
      // Serial.print(ledPositions[i]);
      // Serial.println(" not found");
      ledPositions[i] = -1;
      printLEDPositions();
      bricks-=1;
    }
  }
}

void printGameBoard() {
  Serial.println("------------");
  for (int i=0; i<8; i++) {
    for (int j=0; j<4; j++) {
      Serial.print(gameBoard[i][j]);
      if (j != 3) {
        Serial.print(", ");
      }
    }
    Serial.println();
  }
  Serial.println("------------");
}

void resetGame() {
  /* Reset game */
  bricks=4;
  line=0;
  gameOver=false;
  resetGameBoard();
  resetMovement();
  resetDisplay();
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

    lightGameBoard();

    if (newPlayButtonState!=lastPlayButtonState) {
      if (newPlayButtonState==LOW) {
        /* Save position of bricks to gameBoard */
        handlePlayButtonPress();
        if (bricks==0) {
          gameOver=true;
        }
        lightGameBoard();
        memcpy(gameBoard[line], ledPositions, sizeof(ledPositions));
        printLEDPositions();
        printGameBoard();

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
      resetGame();
    }

    shift();
    delay(delayTime);
  }

  if(digitalRead(resetButtonPin)==LOW) {
    resetGame();
  }
}
