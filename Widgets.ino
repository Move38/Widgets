/*
    Widgets
    by Move38, Inc. 2019
    Lead development by Dan King
    original game by Dan King, Vanilla Liu, Justin Ha, Junege Hong, Kristina Atanasoski, Jonathan Bobrow

    Rules: https://github.com/Move38/Widgets/blob/master/README.md

    --------------------
    Blinks by Move38
    Brought to life via Kickstarter 2018

    @madewithblinks
    www.move38.com
    --------------------
*/

////GENERIC VARIABLES////
enum widgets {DICE, SPINNER, COIN, TIMER};
byte currentWidget = SPINNER;
enum signalTypes {INERT, GO, RESOLVE};
byte pushSignal = INERT;
byte goSignal = INERT;

Timer animTimer;
byte framesRemaining = 0;
Color outcomeColors[6] = {RED, ORANGE, YELLOW, GREEN, BLUE, MAGENTA};
byte currentOutcome = 1;

////WIDGET VARIABLES////
#define DICE_ROLL_INTERVAL 75
#define COIN_FLIP_INTERVAL 500

#define SPINNER_INTERVAL_RESET 25
#define SPINNER_ACTIVE_DIM 100
#define SPINNER_FINAL_DIM 50
word spinInterval = SPINNER_INTERVAL_RESET;

void setup() {
  startWidget();
}

void loop() {
  //listen for button clicks
  if (buttonSingleClicked()) {
    startWidget();
  }

  //listen for signals
  pushLoop();
  goLoop();

  //do things
  switch (currentWidget) {
    case DICE:
      diceLoop();
      break;
    case SPINNER:
      spinnerLoop();
      break;
    case COIN:
      break;
    case TIMER:
      break;
  }

  //set up communication
  byte sendData = (currentWidget << 4) + (pushSignal << 2) + (goSignal);
  setValueSentOnAllFaces(sendData);
}

void pushLoop() {
  if (pushSignal == INERT) {
    //look for neighbors trying to push me
    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
        byte neighborData = getLastValueReceivedOnFace(f);
        if (getPushSignal(neighborData) == GO) {
          //this neighbor is pushing a widget
          currentWidget = getCurrentWidget(neighborData);
          pushSignal = GO;
          //if we're not becoming a TIMER, we gotta also roll/spin/flip
          if (currentWidget != TIMER) {
            startWidget();
          }
        }
      }
    }
  } else if (pushSignal == GO) {
    pushSignal = RESOLVE;//this is corrected within the face loop if it shouldn't happen
    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
        byte neighborData = getLastValueReceivedOnFace(f);
        if (getPushSignal(neighborData) == INERT) {
          pushSignal = GO;//we should still be in GO, there are neighbors to inform
        }
      }
    }
  } else if (pushSignal == RESOLVE) {
    pushSignal = INERT;//this is corrected within the face loop if it shouldn't happen
    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
        byte neighborData = getLastValueReceivedOnFace(f);
        if (getPushSignal(neighborData) == GO) {
          pushSignal = RESOLVE;//we should still be in RESOLVE, there are neighbors to inform
        }
      }
    }
  }
}

void goLoop() {
  if (goSignal == INERT) {
    //look for neighbors trying to push me
    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
        byte neighborData = getLastValueReceivedOnFace(f);
        if (getGoSignal(neighborData) == GO) {
          //this neighbor is pushing a widget
          //if we're not becoming a TIMER, we gotta also roll/spin/flip
          if (currentWidget != TIMER) {
            startWidget();
          }
        }
      }
    }
  } else if (goSignal == GO) {
    goSignal = RESOLVE;//this is corrected within the face loop if it shouldn't happen
    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
        byte neighborData = getLastValueReceivedOnFace(f);
        if (getGoSignal(neighborData) == INERT) {
          goSignal = GO;//we should still be in GO, there are neighbors to inform
        }
      }
    }
  } else if (goSignal == RESOLVE) {
    goSignal = INERT;//this is corrected within the face loop if it shouldn't happen
    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
        byte neighborData = getLastValueReceivedOnFace(f);
        if (getGoSignal(neighborData) == GO) {
          goSignal = RESOLVE;//we should still be in RESOLVE, there are neighbors to inform
        }
      }
    }
  }
}

void startWidget() {
  switch (currentWidget) {
    case DICE:
      //totalAnimationTimer.set(DICE_ROLL_DURATION);
      currentOutcome = random(5) + 1;
      framesRemaining = 25;
      animTimer.set(DICE_ROLL_INTERVAL);
      diceFaceDisplay(currentOutcome);
      goSignal = GO;
      break;
    case SPINNER:
      //totalAnimationTimer.set(SPINNER_DURATION);
      framesRemaining = random(5) + 42;
      spinInterval = SPINNER_INTERVAL_RESET;
      animTimer.set(spinInterval);
      goSignal = GO;
      break;
    case COIN:
      //totalAnimationTimer.set(COIN_FLIP_DURATION);
      framesRemaining = random(1) + 10;
      animTimer.set(COIN_FLIP_INTERVAL);
      goSignal = GO;
      break;
  }
}

void diceLoop() {
  if (animTimer.isExpired() && framesRemaining > 0) {
    animTimer.set(DICE_ROLL_INTERVAL);
    framesRemaining--;
    currentOutcome += 5;
    if (currentOutcome > 6) {
      currentOutcome = currentOutcome % 6;
    }
    diceFaceDisplay(currentOutcome);
  }
}

void diceFaceDisplay(byte val) {
  byte orientFace = random(5);
  setColor(OFF);
  switch (val) {
    case 1:
      setColorOnFace(RED, orientFace);
      break;
    case 2:
      setColorOnFace(ORANGE, orientFace);
      setColorOnFace(ORANGE, (orientFace + 3) % 6);
      break;
    case 3:
      setColorOnFace(YELLOW, orientFace);
      setColorOnFace(YELLOW, (orientFace + 2) % 6);
      setColorOnFace(YELLOW, (orientFace + 4) % 6);
      break;
    case 4:
      setColor(GREEN);
      setColorOnFace(OFF, orientFace);
      setColorOnFace(OFF, (orientFace + 3) % 6);
      break;
    case 5:
      setColor(BLUE);
      setColorOnFace(OFF, orientFace);
      break;
    case 6:
      setColor(MAGENTA);
      break;
  }
}

void spinnerLoop() {
  if (animTimer.isExpired() && framesRemaining > 0) {
    framesRemaining--;
    //determine how long the next frame is
    if (framesRemaining < 24) {//we're in the slow down
      spinInterval = (spinInterval * 23) / 20;
    }
    animTimer.set(spinInterval);

    currentOutcome = (currentOutcome + 1) % 6;
    spinnerFaceDisplay(currentOutcome, SPINNER_ACTIVE_DIM);
    if (framesRemaining == 0) { //this is the last frame
      spinnerFaceDisplay(currentOutcome, SPINNER_FINAL_DIM);
    }
  }
}

void spinnerFaceDisplay(byte val, byte dimness) {
  FOREACH_FACE(f) {
    setColorOnFace(dim(outcomeColors[f], dimness), f);
  }
  if (dimness == SPINNER_ACTIVE_DIM) {
    setColorOnFace(WHITE, val);
  } else {
    setColorOnFace(outcomeColors[val], val);
  }

}

void coinDisplay() {

}

void timerDisplay() {

}

byte getCurrentWidget(byte data) {
  return (data >> 4);
}

byte getPushSignal(byte data) {
  return ((data >> 2) & 3);
}

byte getGoSignal(byte data) {
  return (data & 3);
}
