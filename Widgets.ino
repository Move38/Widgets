////GENERIC VARIABLES
bool inChooser = true;
enum widgetModes {COIN, D6, SPINNER, TIMER, RPS};
byte currentWidget = COIN;
byte currentVal = 1;
enum goSignals {INERT, GOING, RESOLVING, EXEMPT};
byte goSignal = EXEMPT;
bool isAnimating = false;
Timer animTimer;
byte animFrame = 0;

////WIDGET SPECIFIC VARIABLES
Color spinnerColors[6] = {RED, ORANGE, YELLOW, GREEN, BLUE, MAGENTA};
int spinInterval = 25;
byte spinLength;

bool inHiding = false;
enum rpsChoices {NADA, ROCK, PAPER, SCISSOR};
byte rpsSignal = 0;

Color headsColor;
Color tailsColor;

int ticksRemaining;
Timer tickTimer;
Timer tickOffsetTimer;
enum timerStates {SETTING, TIMING, COMPLETE};
byte timerState = SETTING;

void setup() {
}

////////////////
//MASTER LOOPS//
////////////////

void loop() {
  //listen for long press to switch in and out of chooser
  if (buttonLongPressed()) {
    if (inChooser) {//move to the game itself
      inChooser = false;
      switch (currentWidget) {
        case COIN:
          goSignal = INERT;
          headsColor = spinnerColors[rand(2)];
          tailsColor = spinnerColors[rand(2) + 3];
          coinDisplay(inChooser, 3);
          break;
        case D6:
          goSignal = INERT;
          d6Display(currentVal, inChooser);
          break;
        case SPINNER:
          goSignal = INERT;
          spinnerDisplay(7, false);
          break;
        case TIMER:
          currentVal = 1;
          break;
        case RPS:
          rpsDisplay(currentVal);
          break;
      }
    } else {
      inChooser = true;
      goSignal = EXEMPT;
      inHiding = false;
    }
  }

  //run the appropriate loop();
  if (inChooser) {
    osLoop();
  } else {
    //before the loops, update goSignal if we're not exempt
    if (goSignal != EXEMPT) {
      goSignalLoop();
    }
    //now run the loops
    switch (currentWidget) {
      case COIN:
        coinLoop();
        break;
      case D6:
        d6Loop();
        break;
      case SPINNER:
        spinnerLoop();
        break;
      case TIMER:
        timerLoop();
        break;
      case RPS:
        rpsLoop();
        break;
    }
  }

  //set up communication
  byte sendData = (inChooser << 5) + (goSignal << 3) + (inHiding << 2) + (rpsSignal);
  setValueSentOnAllFaces(sendData);
}

void osLoop() {
  //listen for clicking to cycle through widgets
  if (buttonSingleClicked()) {
    nextWidget();
  }

  //listen for animation updates
  if (animTimer.isExpired()) {
    switch (currentWidget) {
      case COIN:
        currentVal ++;
        if (currentVal == 3) {
          currentVal = 1;
        }
        coinDisplay(inChooser, currentVal);
        animTimer.set(400);
        break;
      case D6:
        currentVal++;
        if (currentVal == 7) {
          currentVal = 1;
        }
        d6Display(currentVal, true);
        animTimer.set(250);
        break;
      case SPINNER:
        currentVal = nextClockwise(currentVal);
        spinnerOSDisplay(currentVal);
        animTimer.set(100);
        break;
      case TIMER:
        currentVal ++;
        if (currentVal == 10) {
          currentVal = 0;
        }
        timerOSDisplay(currentVal);
        animTimer.set(100);
        break;
      case RPS:
        currentVal++;
        if (currentVal == 2) {
          currentVal = 0;
        }
        rpsOSDisplay(currentVal);
        animTimer.set(600);
        break;
    }
  }
}

void nextWidget() {
  switch (currentWidget) {
    case COIN:
      currentWidget = D6;
      currentVal = 1;
      break;
    case D6:
      currentWidget = SPINNER;
      currentVal = 0;
      break;
    case SPINNER:
      currentWidget = TIMER;
      currentVal = 0;
      break;
    case TIMER:
      currentWidget = RPS;
      currentVal = 1;
      break;
    case RPS:
      currentWidget = COIN;
      currentVal = 1;
      break;
  }
}

void goSignalLoop() {
  byte currentSignal = goSignal;//this is to avoid the loops automatically completing
  switch (currentSignal) {
    case INERT:
      if (goCheck() == true) {
        goSignal = GOING;
      }
      break;
    case GOING:
      if (resolveCheck() == true) {
        goSignal = RESOLVING;
      }
      break;
    case RESOLVING:
      if (inertCheck() == true) {
        goSignal = INERT;
      }
      break;
  }
}

////////////////
//WIDGET LOOPS//
////////////////

void coinLoop() {

  if (!isAnimating) {
    //there are two ways to start flipping: get clicked or be commanded
    if (buttonSingleClicked() || goSignal == GOING) {//were we clicked?
      isAnimating = true;
      animFrame = 20 + rand(1);
      goSignal = GOING;
    }
  }

  if (isAnimating) {
    if (animTimer.isExpired()) {
      coinDisplay(inChooser, currentVal);
      if (currentVal == 1) {
        currentVal = 2;
      } else {
        currentVal = 1;
      }
      animFrame --;//check for the end of the animation
      animTimer.set(75);
    }//end of timer loop

    if (animFrame == 0) {
      isAnimating = false;
    }
  }
}

void d6Loop() {

  if (!isAnimating) {
    //there are two ways to start rolling: get clicked or be commanded
    if (buttonSingleClicked() || goSignal == GOING) {//were we clicked?
      isAnimating = true;
      animFrame = 0;
      goSignal = GOING;
    }
  }

  if (isAnimating) {
    if (animTimer.isExpired()) {
      currentVal = rand(5) + 1;
      d6Display(currentVal, false);
      animFrame ++;
      animTimer.set(75);
    }

    if (animFrame == 15) {
      isAnimating = false;
    }
  }
}

void spinnerLoop() {
  if (!isAnimating) {
    //there are two ways to start spinning: get clicked or be commanded
    if (buttonSingleClicked() || goSignal == GOING) {
      isAnimating = true;
      spinLength = rand(5) + 36;
      spinInterval = 25;
      animFrame = 0;
      goSignal = GOING;
    }
  }

  if (isAnimating) {
    if (animTimer.isExpired()) {
      currentVal = nextClockwise(currentVal);
      spinnerDisplay(currentVal, false);
      animFrame ++;
      animTimer.set(spinInterval);
    }

    if (animFrame > spinLength) {
      spinInterval += 2;
    }

    if (animFrame == spinLength + 24) {
      isAnimating = false;
      spinnerDisplay(currentVal, true);
    }
  }
}

void timerLoop() {
  switch (timerState) {
    case SETTING:
      //in here we listen for button clicks to increment currentVal, which represents minutes on the timer
      if (buttonSingleClicked()) {
        currentVal++;
        if (currentVal == 6) {
          currentVal = 1;
        }
      }

      //if double clicked, we move on
      if (buttonDoubleClicked()) {
        ticksRemaining = currentVal * 60;
        timerState = TIMING;
      }
      break;
    case TIMING:
      //here we simply count down the remaining ticks
      break;
    case COMPLETE:
      if (buttonSingleClicked()) { //back to SETTING!
        timerState = SETTING;
        animFrame = 0;
      }
      break;
  }
  timerDisplay();
}

void timerDisplay() {
  switch (timerState) {
    case SETTING:
      if (animTimer.isExpired()) {
        setColor(dim(spinnerColors[currentVal - 1], 25));
        if (animFrame == 0) {
          FOREACH_FACE(f) {//just turn on the faces corresponding to the timer choice
            if (f <= currentVal) {
              setColorOnFace(dim(spinnerColors[currentVal - 1], 100), f);
            }
          }
          animFrame = 1;
        } else if (animFrame == 1) {//nothing here, just setting animFrame
          animFrame = 0;
        }
        animTimer.set(500);
      }
      setColorOnFace(WHITE, 0);
      break;
    case TIMING:
      break;
    case COMPLETE:
      if (animTimer.isExpired()) {
        setColor(dim(RED, 25));
        if (animFrame == 0) {
          setColor(RED);
          animFrame = 1;
        } else if (animFrame == 1) {
          animFrame = 0;
        }
      }
      setColorOnFace(WHITE, 0);
      break;
  }
}

void rpsLoop() {
  if (!inHiding) {
    if (buttonSingleClicked()) {
      currentVal ++;
      if (currentVal == 4) {
        currentVal = 1;
      }
      rpsDisplay(currentVal);
      rpsSignal = currentVal;
    }

    if (buttonDoubleClicked()) { //toggle hiding mode
      inHiding = true;
      rpsDisplay(4);
    }
  }

  if (inHiding) {//check for double clicks or combat
    if (buttonDoubleClicked()) { //toggle hiding mode
      inHiding = false;
      rpsDisplay(currentVal);
    }

    //we need to evaluate all neighbors, see if they are in RPS hidden mode
    byte neighborsIWin = 0;
    byte neighborsILose = 0;
    byte neighborsITie = 0;

    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f)) { //there is a neighbor here
        byte neighborData = getLastValueReceivedOnFace(f);
        if (getRPSHiddenSignal(neighborData) == 1) { //this neighbor is in hiding, ready to play
          int victoryCheck = rpsSignal - getRPSSignal(neighborData);
          if (victoryCheck == 1 || victoryCheck == -2) {
            neighborsIWin ++;
          } else if (victoryCheck == 2 || victoryCheck == -1) {
            neighborsILose ++;
          } else {
            neighborsITie ++;
          }
        }
      }
    }

    //decide how to display win/loss/tie state
    if (neighborsIWin + neighborsILose + neighborsITie > 0) {//first, do I have neighbors at all?
      if (neighborsILose > 0) {
        rpsCombatDisplay(rpsSignal, 0);
      } else if (neighborsITie > 0) {
        rpsCombatDisplay(rpsSignal, 1);
      } else if (neighborsIWin > 0) {
        rpsCombatDisplay(rpsSignal, 2);
      }
    }
  }
}

/////////////////
//DISPLAY LOOPS//
/////////////////

void coinDisplay(bool osMode, byte val) {
  if (osMode) {//OS display
    switch (val) {
      case 1:
        setColor(dim(WHITE, 25));
        setColorOnFace(WHITE, 0);
        setColorOnFace(WHITE, 1);
        setColorOnFace(WHITE, 2);
        break;
      case 2:
        setColor(dim(WHITE, 100));
        setColorOnFace(WHITE, 3);
        setColorOnFace(WHITE, 4);
        setColorOnFace(WHITE, 5);
        break;
    }
  } else {//not in OS mode
    switch (val) {
      case 1:
        setColorOnFace(headsColor, 0);
        setColorOnFace(headsColor, 1);
        setColorOnFace(headsColor, 2);
        setColorOnFace(dim(tailsColor, 25), 3);
        setColorOnFace(dim(tailsColor, 25), 4);
        setColorOnFace(dim(tailsColor, 25), 5);
        break;
      case 2:
        setColorOnFace(dim(headsColor, 25), 0);
        setColorOnFace(dim(headsColor, 25), 1);
        setColorOnFace(dim(headsColor, 25), 2);
        setColorOnFace(tailsColor, 3);
        setColorOnFace(tailsColor, 4);
        setColorOnFace(tailsColor, 5);
        break;
      case 3:
        setColorOnFace(headsColor, 0);
        setColorOnFace(headsColor, 1);
        setColorOnFace(headsColor, 2);
        setColorOnFace(tailsColor, 3);
        setColorOnFace(tailsColor, 4);
        setColorOnFace(tailsColor, 5);
        break;
    }
  }
}

void d6Display(byte num, bool osMode) {
  setColor(OFF);
  Color displayColor;
  byte rotationRandomizer = 0;
  bool displayArr[6] = {false, false, false, false, false, false};

  switch (num) {
    case 1:
      if (osMode) {
        displayArr[0] = true;
      } else {
        displayArr[rand(5)] = true;
      }
      displayColor = RED;
      break;
    case 2:
      if (osMode) {
        rotationRandomizer = 0;
      } else {
        rotationRandomizer = rand(2);
      }
      displayArr[rotationRandomizer] = true;
      displayArr[rotationRandomizer + 3] = true;
      displayColor = ORANGE;
      break;
    case 3:
      if (osMode) {
        rotationRandomizer = 0;
      } else {
        rotationRandomizer = rand(1);
      }
      displayArr[rotationRandomizer] = true;
      displayArr[rotationRandomizer + 2] = true;
      displayArr[rotationRandomizer + 4] = true;
      displayColor = YELLOW;
      break;
    case 4:
      if (osMode) {
        rotationRandomizer = 0;
      } else {
        rotationRandomizer = rand(2);
      }
      displayArr[0] = true;
      displayArr[1] = true;
      displayArr[2] = true;
      displayArr[3] = true;
      displayArr[4] = true;
      displayArr[5] = true;
      displayArr[rotationRandomizer] = false;
      displayArr[rotationRandomizer + 3] = false;
      displayColor = GREEN;
      break;
    case 5:
      displayArr[0] = true;
      displayArr[1] = true;
      displayArr[2] = true;
      displayArr[3] = true;
      displayArr[4] = true;
      displayArr[5] = true;
      if (osMode) {
        displayArr[3] = false;
      } else {
        displayArr[rand(5)] = false;
      }
      displayColor = BLUE;
      break;
    case 6:
      displayArr[0] = true;
      displayArr[1] = true;
      displayArr[2] = true;
      displayArr[3] = true;
      displayArr[4] = true;
      displayArr[5] = true;
      displayColor = MAGENTA;
      break;
  }

  if (osMode) {
    displayColor = WHITE;
    setColor(dim(WHITE, 25));
  }

  FOREACH_FACE(f) {
    if (displayArr[f] == true) {
      setColorOnFace(displayColor, f);
    }
  }
}

void spinnerDisplay(byte face, bool isFinal) {

  FOREACH_FACE(f) {
    setColorOnFace(dim(spinnerColors[f], 25), f);
  }

  if (isFinal) {
    setColorOnFace(spinnerColors[face], face);
  } else {
    setColorOnFace(dim(WHITE, 100), face);
  }
}

void spinnerOSDisplay(byte face) {
  setColor(dim(WHITE, 25));
  setColorOnFace(dim(WHITE, 100), 0);
  setColorOnFace(dim(WHITE, 100), 2);
  setColorOnFace(dim(WHITE, 100), 4);
  setColorOnFace(WHITE, face);
}

void timerOSDisplay(byte count) {
  setColor(dim(WHITE, 25));
  if (count < 6) {
    setColorOnFace(dim(WHITE, 100), count);
  }
  setColorOnFace(WHITE, 0);
}

void rpsDisplay(byte choice) {
  setColor(OFF);
  switch (choice) {
    case 1://rock
      setColorOnFace(BLUE, 0);
      setColorOnFace(BLUE, 1);
      setColorOnFace(BLUE, 2);
      setColorOnFace(BLUE, 3);
      break;
    case 2://paper
      setColorOnFace(YELLOW, 1);
      setColorOnFace(YELLOW, 2);
      setColorOnFace(YELLOW, 4);
      setColorOnFace(YELLOW, 5);
      break;
    case 3://scissor
      setColorOnFace(RED, 0);
      setColorOnFace(RED, 2);
      setColorOnFace(RED, 4);
      break;
    case 4://hiding
      setColorOnFace(dim(RED, 25), 0);
      setColorOnFace(dim(YELLOW, 25), 1);
      setColorOnFace(dim(BLUE, 25), 2);
      setColorOnFace(dim(RED, 25), 3);
      setColorOnFace(dim(YELLOW, 25), 4);
      setColorOnFace(dim(BLUE, 25), 5);
      break;
  }
}

void rpsCombatDisplay(byte choice, byte outcome) {
  byte brightness;
  if (outcome == 0) {
    brightness = 25;
    setColor(OFF);
  } else if (outcome == 1) {
    brightness = 150;
    setColor(OFF);
  } else if (outcome == 2) {
    brightness = 255;
    setColor(WHITE);
  }

  switch (choice) {
    case ROCK:
      setColorOnFace(dim(BLUE, brightness), 0);
      setColorOnFace(dim(BLUE, brightness), 1);
      setColorOnFace(dim(BLUE, brightness), 2);
      setColorOnFace(dim(BLUE, brightness), 3);
      break;
    case PAPER:
      setColorOnFace(dim(YELLOW, brightness), 1);
      setColorOnFace(dim(YELLOW, brightness), 2);
      setColorOnFace(dim(YELLOW, brightness), 4);
      setColorOnFace(dim(YELLOW, brightness), 5);
      break;
    case SCISSOR:
      setColorOnFace(dim(RED, brightness), 0);
      setColorOnFace(dim(RED, brightness), 2);
      setColorOnFace(dim(RED, brightness), 4);
      break;
  }
}

void rpsOSDisplay(byte frame) {
  byte brightness = frame * 230 + 25;
  setColor(dim(WHITE, 25));
  setColorOnFace(dim(WHITE, brightness), 0);
  setColorOnFace(dim(WHITE, brightness), 2);
  setColorOnFace(dim(WHITE, brightness), 4);
}

///////////////////////////
//COMMUNICATION FUNCTIONS//
///////////////////////////

byte getOSMode(byte data) {
  return (data >> 5);//first bit
}

byte getGoSignal(byte data) {
  return ((data >> 3) & 3);//second and third bit
}

byte getRPSHiddenSignal(byte data) { //fourth bit
  return ((data >> 2) & 1);
}

byte getRPSSignal(byte data) {
  return (data & 3);//last two bits
}

bool goCheck() {
  //check all neighbors for any neighbors giving go signal
  bool goBool = false;
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {
      byte neighborData = getLastValueReceivedOnFace(f);
      if (getGoSignal(neighborData) == GOING) {
        goBool = true;
      }
    }
  }
  return goBool;
}

bool resolveCheck() {
  bool resolveBool = true;
  //if all of my neighbors are going, resolving, or exempt, we can resolve
  //easy to determine, because the only remaining state is INERT
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {
      byte neighborData = getLastValueReceivedOnFace(f);
      if (getGoSignal(neighborData) ==  INERT) { //this is the only thing that prevents this transition
        resolveBool = false;
      }
    }
  }

  return resolveBool;
}

bool inertCheck() {
  bool inertBool = true;
  //if all of my neighbors are resolving, inert, or exempt, we can go inert
  //easy to determine, because the only remaining state is GOING
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {
      byte neighborData = getLastValueReceivedOnFace(f);
      if (getGoSignal(neighborData) ==  GOING) { //this is the only thing that prevents this transition
        inertBool = false;
      }
    }
  }

  return inertBool;
}

/////////////////////////
//CONVENIENCE FUNCTIONS//
/////////////////////////

byte nextClockwise (byte face) {
  if (face == 5) {
    return 0;
  } else {
    return face + 1;
  }
}

byte nextCounterclockwise (byte face) {
  if (face == 0) {
    return 5;
  } else {
    return face - 1;
  }
}
