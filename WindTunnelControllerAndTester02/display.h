/* code for two LCDs
  one LCD on wall and one on test board
  LCDs are outfitte with LCD117 boards so use PAnderson's LCD117 serial protocol
*/

char modeNames[4][13] = {"Rev C Test", "Rev P Test", "Run Curves C", "Run Curves P" };
extern int newMode, mode;

void initLCD_display() {
  Serial.begin(57600);
  Serial1.begin(19200);
  delay(20);
  Serial1.print("?BFF"); // backlight full
  delay(20);
  Serial1.print("?G420");  // 4x20 chars
  delay(20);
  Serial1.print("?c0"); // turn cursor off
  delay(50);
  Serial1.print("?f?a");
  Serial1.print("?f");
  Serial1.print("?a");
  Serial1.print("Fan:  ");
  Serial1.print("?x00?y1");
  Serial1.print("Temp: ");  Serial.print(targetTempC);
}


void printTempC() {
  static unsigned long lastPrintTime;
  if (millis() - lastPrintTime > 300) {
    lastPrintTime = millis();
    Serial.print("set ");
    Serial.print(targetTempC);            Serial.print("\t");
    Serial.print("test ");     Serial.print((float)testChTempC, 2);  Serial.print("\t");
    Serial.print("heat ");     Serial.print((float)heaterTempC, 2);  Serial.print("\t");
    Serial.print("delta "); Serial.print(deltaTempC);  Serial.print("\t");
    Serial.print("output ");   Serial.print(outputOffsetC);   Serial.print("\t");
    Serial.print("innerSetpt "); Serial.print(innerLoopSetpoint);   Serial.print("\t");
    Serial.println(OutputPWM, 2);
  }
}

void printTempDiplay() {
  static unsigned long lastTime, lastTimeDisp;
  static int lastTargetTempC, lastFanPWM;
  static float tempFloat;

  // put in a tolerance to make sure that an absolute value millis
  // does not get missed with an LCD write.
  // Possibly not a realistic worry but Software serial has no UART
  if (millis() - lastTime > 300 ) { // 10 times a sec
    lastTime = millis();

    if (millis() - lastTimeDisp > 250) {
      lastTimeDisp = millis();
      Serial1.print("?x17?y0");
      Serial1.print(tach);
    }

    if (lastTargetTempC != targetTempC || lastFanPWM != fanPWM) { // only write display if temp or fan setting changes
      Serial1.print("?f");
      Serial1.print("?a");
      Serial1.print("Set: "); Serial1.print((int)targetTempC); Serial1.print("C P ");
      //Serial1.print("F ");
      Serial1.print(fanPWM); Serial1.print(" T ");
      lastFanPWM = fanPWM;
      lastTargetTempC = targetTempC;

      newMode = 1;

      // second LCD line
      Serial1.print("?x00?y1");
      Serial1.print("Temp: ");
    }

    // testChTempC = tempWT.GetTemperature();
    Serial1.print("?y1?x06");
    Serial1.print(testChTempC);
    Serial1.print(" C    ");
    Serial1.print(momSw);

    Serial1.print("?y2?x00");
    Serial1.print("pit "); Serial1.print(pitotOut, 0);

    if (newMode) {  // do both LCD's here to make sure that newMode resets cleanly
      Serial1.print("?y3?l"); 
      Serial1.print(modeNames[mode]); // prints name of function mode triggered from mom switches
      Serial2.print("?y3?l");
      Serial2.print(modeNames[mode]);
      newMode = 0;
    }

    /* do mode printout */
    //  Serial1.print(" ? x2");

    /*
      if (heatFlag) {
      Serial1.print(" ? x15 ? y0");
      delay(10);
      Serial1.print('H');
      }
      else {
      Serial1.print(" ? x15 ? y0");
      delay(10);
      Serial1.print(' ');
      } */
  }
}

/******************************** LCD 2 - the tester LCD *********************************/


void initLCD2() { // the tester LCD
  Serial2.begin(19200);
  delay(20);
  Serial2.print(" ? BFF"); // backlight full
  delay(20);
  Serial2.print(" ? G420");  // 4x20 chars
  delay(20);
  Serial2.print(" ? c0"); // turn cursor off
  delay(20);
  Serial2.print(" ? f ? a"); // turn cursor off
  Serial2.print(" ? D10000040E1B111F00");
  delay(100);
  Serial2.print(" ? y3 ? l"); Serial2.print(modeNames[mode]);

  Serial.println("initLCD2 ");
  //delay(500);
}

void doLCD2() {
  static unsigned long lastTime;


  if (millis() - lastTime > 300) {
    lastTime =   millis();
    Serial2.print(" ? x00 ? y0"); Serial2.print(modeNames[4]);
    Serial2.print(" ? l ? x00");  Serial2.print(tester.ADCcountAverage, 1); Serial2.print(" adc ");
    Serial2.print(" ? 1 "); Serial2.print(tester.deltaCount);

    Serial2.print(" ? y1 ? l");  Serial2.print(tester.ADCvolts, 3); Serial2.print(" volt");
    Serial2.print(" ? x6");

    /**** need to test shutdown pin on Rev P in here somewhere ****/

    Serial2.print(" ? y2 ? l"); Serial2.print(tester.tempC, 1); Serial2.print(" deg C ");
    if (mode == 0) { // rev C
      Serial2.print(" ? x11 ? y1Out "); Serial2.print(tester.revCoutputVolts, 3);
      Serial2.print("V");
    }



    
  }
  
}
