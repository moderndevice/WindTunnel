char modeNames[4][13] = {"Man C", "Man P", "Crvs C", "Crvs P" };

void doSwitchDecode() {
  static unsigned long lastStateRead;
  if (millis() - lastStateRead > 60) {
    lastStateRead = millis();

    if (momSw != 0) {  // button pressed
      switch (momSw) {
        /*
               8
          4          1
               2
        */

        case 4: // Rev P Test
          controlMode = 1;
          CPflag = REVCFLAG;
          break;

        case 2: // run C curves
          controlMode = 2;
          CPflag = REVPFLAG;
          break;

        case 1: // Rev C test
          controlMode = 0;
          break;

        case 8: // run P curves
          controlMode = 3;
          break;

      }

      if (controlMode != lastMode) {
        lastMode = controlMode;
        newMode = 1;
      }
    }
  }

 doControlModes(); 
}



doControlModes(){

    switch (controlMode) {
    case 0:  // Rev C test
      //      testSensors();
      //      CPflag = REVCFLAG;
      break;

    case 1:  // Rev P test
      testSensors();
      CPflag = REVPFLAG;
      break;

    case 2:  // C run curves
      //      runCcurves();
      break;

    case 3:  // P run curves
      runPcurves();
      break;
  }
}
