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
byte currentWidget = DICE;
enum signalTypes {INERT, GO, RESOLVE};
byte pushSignal = INERT;
byte goSignal = INERT;

Timer animTimer;
#define DICE_ROLL_DURATION 1000
#define SPINNER_DURATION 1000
#define COIN_FLIP_DURATION 1000
Color outcomeColors[6] = {RED, ORANGE, YELLOW, GREEN, BLUE, MAGENTA};
byte randomOutcome = 1;

void setup() {
  startWidget();
}

void loop() {
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
      animTimer.set(DICE_ROLL_DURATION);
      randomOutcome = random(5) + 1;
      break;
    case SPINNER:
      animTimer.set(SPINNER_DURATION);
      randomOutcome = random(5);
      break;
    case COIN:
      animTimer.set(COIN_FLIP_DURATION);
      randomOutcome = random(1);
      break;
  }
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
