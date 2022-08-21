/* code for two LCDs
  one LCD on wall and one on test board
  LCDs are outfitte with LCD117 boards so use PAnderson's LCD117 serial protocol
*/

char modeNames[4][13] = {"Test Rev C", "Testw Rev P", "Run Curves C", "Run Curves P" };
extern int newMode, mode;

void initLCD_display() {
  Serial.begin(57600);
  SerialLCD1.begin(19200);
  delay(20);
  SerialLCD1.print("?BFF"); // backlight full
  delay(20);
  SerialLCD1.print("?G420");  // 4x20 chars
  delay(20);
  SerialLCD1.print("?c0"); // turn cursor off
  delay(50);
  SerialLCD1.print("?f?a");
  SerialLCD1.print("?f");
  SerialLCD1.print("?a");
  SerialLCD1.print("Fan:  ");
  SerialLCD1.print("?x00?y1");
  SerialLCD1.print("Temp: ");  Serial.print(targetTempC);
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

/*     mode 0: Rev C test, mode 1: Rev P test, mode 2: C run curves, mode 3:  P run curves */

/*  4 x 20 LCD display info - test Mode

   0 Mode     tach
   1 Set PWM,   Pitot
   2  Set Temp, test chamber Temp, 'H'
   3 Unit Voltage, Unit Temp

*/


void printTempDiplay() {
  static unsigned long lastTimeSlow, lastTimeDispFast;
  static int lastTargetTempC, lastFanPWM;
  static float tempFloat;

//  if (millis() - lastTimeDispFast > 200) {  // print the fast moving data
//    lastTimeDispFast = millis();
//
//
//    if (heatFlag) {
//      SerialLCD1.print("?x13?y0");
//      SerialLCD1.print('H');
//    }
//
//    SerialLCD1.print("?x15??y0");  SerialLCD1.print(tach); // tach
//    if (tach < 1000) {
//      SerialLCD1.print(" ");
//
//      SerialLCD1.print("?y2?x12");    // print Temp Data
//      SerialLCD1.print(testChTempC);
//      SerialLCD1.print("?y2?x19"); SerialLCD1.print("    ");
//
//      //   if (lastTargetTempC != targetTempC || lastFanPWM != fanPWM) { // only write display if temp or fan setting changes
//    }
//  }




  if (millis() - lastTimeSlow > 1500) {
    // print the the slower moving data once a second - need to synch up the positions with fast moving data
    lastTimeSlow = millis();

    SerialLCD1.print("?f"); // clear screen


    //SerialLCD1.print("?y0?l");  // line 0
    SerialLCD1.print(modeNames[mode]); // prints name of function mode triggered from mom switches
    SerialLCD1.print("?y0?x15");
    SerialLCD1.print(tach); // tach

    // line 1
    SerialLCD1.print("?x00?y1PWM ");
    SerialLCD1.print(fanPWM); // PWM
    SerialLCD1.print("?x10?y1 Pit ");
    SerialLCD1.print(pitotOut, 0);  // pitot

    // line 2
    SerialLCD1.print("?x00?y2");
    SerialLCD1.print("Set ");
    SerialLCD1.print((int)targetTempC);  // PWM
    SerialLCD1.print("C");

    SerialLCD1.print("?011?y2");
    SerialLCD1.print("T ");
    SerialLCD1.print((float)testChTempC, 2);
    SerialLCD1.print(" C ");

    if (heatFlag) {
      SerialLCD1.print('H');
    }

    // line 3

    // second LCD line
    SerialLCD1.print("?x00?y3");
    SerialLCD1.print("Pitot ");
    SerialLCD1.print(pitotOut, 0);

    lastFanPWM = fanPWM;
    lastTargetTempC = targetTempC;
    newMode = 1;
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
