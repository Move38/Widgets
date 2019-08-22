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
byte currentWidget = COIN;
enum signalTypes {INERT, GO, RESOLVE};
byte pushSignal = INERT;
byte goSignal = INERT;

Timer animTimer;
byte framesRemaining = 0;
Color outcomeColors[6] = {RED, ORANGE, YELLOW, GREEN, BLUE, MAGENTA};
byte currentOutcome = 1;

////WIDGET VARIABLES////
#define DICE_ROLL_INTERVAL 75
#define COIN_FLIP_INTERVAL 150

#define SPINNER_INTERVAL_RESET 25
#define SPINNER_ACTIVE_DIM 100
#define SPINNER_FINAL_DIM 50
word spinInterval = SPINNER_INTERVAL_RESET;

void setup() {
  startWidget();
  randomize();
}

void loop() {
  //listen for button clicks
  if (buttonSingleClicked()) {
    startWidget();
  }

  if (buttonLongPressed()) {
    currentWidget = (currentWidget + 1) % 4;
    startWidget();
    pushSignal = GO;
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
      coinLoop();
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
      framesRemaining = 20 + random(5);
      animTimer.set(DICE_ROLL_INTERVAL);
      diceFaceDisplay(currentOutcome);
      goSignal = GO;
      break;
    case SPINNER:
      //totalAnimationTimer.set(SPINNER_DURATION);
      framesRemaining = random(11) + 36;
      spinInterval = SPINNER_INTERVAL_RESET;
      animTimer.set(spinInterval);
      goSignal = GO;
      break;
    case COIN:
      //totalAnimationTimer.set(COIN_FLIP_DURATION);
      framesRemaining = random(3) + 22;
      goSignal = GO;
      if (animTimer.isExpired()) {//reset the timer if it isn't currently going
        animTimer.set(COIN_FLIP_INTERVAL);
        if (currentOutcome == 0) {
          currentOutcome = 1;
        } else {
          currentOutcome = 0;
        }
      }
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

void coinLoop() {
  if (animTimer.isExpired() && framesRemaining > 0) {
    framesRemaining--;
    animTimer.set(COIN_FLIP_INTERVAL);
    //change the outcome from 0 to 1 and back
    if (currentOutcome == 0) {
      currentOutcome = 1;
    } else {
      currentOutcome = 0;
    }
  }

  if (framesRemaining == 0) {
    coinDisplay(true);
  } else {
    coinDisplay(false);
  }
}

void coinDisplay(bool finalFlip) {
  Color faceColor;
  if (currentOutcome == 0) {
    faceColor = YELLOW;
  } else {
    faceColor = WHITE;
  }

  byte animPosition = COIN_FLIP_INTERVAL - animTimer.getRemaining();
  byte leftStart = 0;
  byte centerStart = COIN_FLIP_INTERVAL / 6;
  byte rightStart = COIN_FLIP_INTERVAL / 3;
  byte edgeDuration = (COIN_FLIP_INTERVAL / 3) * 2;

  setColor(OFF);

  if (animPosition >= leftStart && animPosition <= leftStart + edgeDuration) {
    byte brightness = sin8_C(map(animPosition, leftStart, leftStart + edgeDuration, 0, 255));
    setColorOnFace(dim(faceColor, brightness), 0);
    setColorOnFace(dim(faceColor, brightness), 1);
  }

  if (finalFlip && animPosition >= leftStart + (edgeDuration / 2)) {
    setColorOnFace(faceColor, 0);
    setColorOnFace(faceColor, 1);
  }

  if (animPosition >= centerStart && animPosition <= centerStart + edgeDuration) {
    byte brightness = sin8_C(map(animPosition, centerStart, centerStart + edgeDuration, 0, 255));
    setColorOnFace(dim(faceColor, brightness), 2);
    setColorOnFace(dim(faceColor, brightness), 5);
  }

  if (finalFlip && animPosition >= centerStart + (edgeDuration / 2)) {
    setColorOnFace(faceColor, 2);
    setColorOnFace(faceColor, 5);
  }

  if (animPosition >= rightStart && animPosition <= rightStart + edgeDuration) {
    byte brightness = sin8_C(map(animPosition, rightStart, rightStart + edgeDuration, 0, 255));
    setColorOnFace(dim(faceColor, brightness), 3);
    setColorOnFace(dim(faceColor, brightness), 4);
  }

  if (finalFlip && animPosition >= rightStart + (edgeDuration / 2)) {
    setColorOnFace(faceColor, 3);
    setColorOnFace(faceColor, 4);
  }


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
