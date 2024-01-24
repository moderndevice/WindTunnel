char modeNames[4][13] = {"Man C", "Man P", "Crvs C", "Crvs P" };
int controlMode = 0, lastMode = 20, newMode = 1; // state variable
int momSw; //variable for momentary switches
int startSwitch, resetSwitch;

extern const int startSwitchPin;
extern const int resetSwitchPin;

extern void manualTest(int cpFlag);
extern void  runCurves(int cpFlag);

enum CP_FLAG {
  REV_C,
  REV_P
};

enum CP_FLAG cpFlag = REV_P;

doControlModes(int controlMode) {

  switch (controlMode) {
    case 0:  // Rev C test
      cpFlag = REV_C;
      manualTest(cpFlag);
      //      CPflag = REVCFLAG;
      break;

    case 1:  // Rev P test
      cpFlag = REV_P;
      manualTest(cpFlag);
      // testSensors();
      CPflag = REVPFLAG;
      break;

    case 2:  // C run curves
      cpFlag = REV_C;
      runCurves(cpFlag);
      break;

    case 3:  // P run curves
      cpFlag = REV_P;
      runCurves(cpFlag);
      break;
  }
}


void doSwitchDecode() {
  static unsigned long lastStateRead;

  if (millis() - lastStateRead > 30) {
    lastStateRead = millis();

    if (momSw != 0) {  // button pressed
      switch (momSw) {
        /*
               8
          4          1
               2
        */

        case 8: // Rev P manual test
          cpFlag = REV_P;
          manualTest(cpFlag);
          controlMode = 1;
          break;

        case 1: // run C curves
          cpFlag = REV_C;
          runCurves(cpFlag);
          controlMode = 2;
          break;

        case 4: // Rev C manual test

          cpFlag = REV_C;
          manualTest(cpFlag);
          controlMode = 0;
          break;

        case 2: // run P curves
          cpFlag = REV_P;
          runCurves(cpFlag);
          controlMode = 3;
          break;

      }

      /* what is this doing? */
      if (controlMode != lastMode) {
        lastMode = controlMode;  // saves last controlMode but why?
        newMode = 1;  // new Mode triggers printing the mode name
      }
    }

    /* note that start and reset swtiches are ony set here (1), reest in code from elsewhere CHECK THIS!! */
    if (!digitalRead(startSwitchPin)) {
      startSwitch = 1;  // true when switch is pressed - need to figure out debouncing
    }

    if (!digitalRead(resetSwitchPin)) {
      resetSwitch = 1;
    }
  }
  
  doControlModes(controlMode);
}
