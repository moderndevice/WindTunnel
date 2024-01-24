#define PWM_INCREMENT_AMOUNT 5 // amount to increment fan between iterations
#define TEMP_INCREMENT_AMOUNT 1 // amount to increment heater between iterations

const int PWMbeginPot = 2;
const int PWMendPot = 3;
const int CurvesTempPot = 6;
unsigned long curvesDelayMillis, printDebugMillis;
int beginFanPWM, endFanPWM, currentFanPWM;
static int beginCurvesTempC, endCurvesTempC, currentTempC;
bool runPcurvesInitialized = false;

const int LED_WHITE_PIN = 36;
const int LED_RED_PIN = 37;
const int LED_GREEN_PIN = 35;
const int LED_BLUE_PIN = 34;

const int startSwitchPin = 28;
const int resetSwitchPin = 26;

extern int fanPWM;
extern char modeNames[4][13];
extern int controlMode;

// Changing default values of enum constants
enum curveP_State {
  WAIT_4_START,
  STABLILZE_DELAY,
  WAIT_4_TEMP,
  CAPTURE_DATA,
  INCREMENT,
  DONE
};


/* states to control various test modes */
char cP_States[10][16] = {"WAIT", "STDL", "4TMP", "CPTR", "INCR", "DONE", "DONE", "MAN_C", "MAN_P"};
enum curveP_State cPstate = WAIT_4_START;

// reference char modeNames[4][13] = {"Man C", "Man P", "Crvs C", "Crvs P" };

resetLEDs() {
  digitalWrite(LED_WHITE_PIN, LOW);
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_BLUE_PIN, LOW);
}

void doP_CurvesDiplay(int modeNameNumber) { // Temp thing to put up mode names
  static unsigned long lastTimeSlow, lastTimeDispFast;
  if (millis() - lastTimeSlow > 500) {
    // print the the slower moving data once a second - need to synch up the positions with fast moving data
    lastTimeSlow = millis();

    // Serial1.print("?f"); // clear screen

    // line 0
    Serial1.print("?x00?y0");
    Serial1.print(modeNames[modeNameNumber]); // prints name of function mode triggered from mom switches
    Serial1.print("?y0?x15    ");
    Serial1.print("?y0?x15");
    Serial1.print(tach); // tach

    // line 1
    Serial1.print("?x00?y1");
    Serial1.print("   "); // erase space
    Serial1.print("?x00?y1");
    Serial1.print(beginFanPWM);
    Serial1.print("?x03?y1");
    Serial1.print("PWM - ");
    Serial1.print("?x09?y1");
    Serial1.print("   "); // erase space
    Serial1.print("?x09?y1");
    Serial1.print(endFanPWM);
    Serial1.print("PWM  ");
    Serial1.print("?x17?y1");
    Serial1.print(fanPWM);

    // line 2
    Serial1.print("?x00?y2");
    Serial1.print("  "); // erase space
    Serial1.print("?x00");
    Serial1.print((int)beginCurvesTempC);
    Serial1.print("C - ");
    Serial1.print("?x07");
    Serial1.print("   "); // erase space
    Serial1.print("?x07");
    Serial1.print((int)endCurvesTempC);
    Serial1.print("C TC ");
    Serial1.print("?x14");
    Serial1.print("   "); // erase space
    Serial1.print("?x14");
    Serial1.print(tempTestChTempC);
    Serial1.print("C");

    // line 3
    Serial1.print("?x00?y3Pt     ");
    Serial1.print("?x03?y3");
    Serial1.print(pitotOut, 0);  // pitot
    Serial1.print("?x08?y3");
    Serial.println(sensorVolts, 1);


    Serial1.print("    ");
    Serial1.print("?x12?y3");
    Serial1.print(cP_States[cPstate]); // state machine state
  }
}

void runCurves(int modeFlag) {
  /* this function is setup as a state machine that checks the temperature compliance
     then agregates sensor and voltage data over a period. And finally reports data.
     The "Start Switch" (momentary) starts the state machine but also resets it,
     if it is pushed in any other state than the WAIT_4_START
  */

  const unsigned long STABILIZE_DELAY_MS = 20000;  // waiting period for fan to stabilize
  const unsigned long DATA_GATHERING_PERIOD_MS = 30000;  // period to gather data to integrate
  const unsigned long COOL_DOWN_MS = 1000 * 60 * 15; // 15 min
  static unsigned long resetDelay = 1000, captureMillis, stabilizeMillis, timeGrainMillis, coolDownMillis;
  const  unsigned long timeGrain_MS = 7;  // overall sampling rate  odd number to go in and out of phase with power line freq (16.66 mS)

  static bool runPcurvesInitialized = false;
  static unsigned long long totalTach, totalPitotADC, totalSensorADC;
  static unsigned long dataPoints = 0;
  static int modeNameSel = 0;

  unsigned long long totalPitotADCScaled, totalSensorPWM_Scaled; // 64 bits
  float pitot, sensVolts;

  if (millis() - timeGrainMillis > timeGrain_MS) {
    timeGrainMillis = millis();



    if (modeFlag == REV_P) {
      modeNameSel = 3; // do curves display - it has its own millis timer too - param is modeName selector - Rev P
    } else {
      modeNameSel = 2; // do curves display - it has its own millis timer too - param is modeName selector - Rev C
    }
    doP_CurvesDiplay(modeNameSel); // do curves display - it has its own millis timer too - param is modeName selector


    /******************** WAIT_4_START ********************/
    if (cPstate ==  WAIT_4_START) {  //
      /* read pots and set the curves parameters, wait here for user to press start switch
        update the display to ref);lect current parameters
      */

      beginFanPWM = ((pots[PWMbeginPot] / 4) / 5) * 5; // quantize to 5's
      endFanPWM = ((pots[PWMendPot] / 4) / 5) * 5;
      beginCurvesTempC = map(pots[6], 0, 1023, 15, 48); // temp times 2 in map
      endCurvesTempC = map(pots[7], 0, 1023, 15, 48);
      currentTempC = beginCurvesTempC;
      targetTempC = beginCurvesTempC;
      currentTemp = beginCurvesTempC;
      Serial.println("W_4_S");
      doFan(fanPWM);
      resetLEDs();
      digitalWrite(LED_BLUE_PIN , HIGH);
      // doP_CurvesDiplay(modeNameSel);  // display updated changes


      if (startSwitch) {
        startSwitch = 0;
        stabilizeMillis = millis();
        doFan(fanPWM = beginFanPWM);
        //doP_CurvesDiplay(1); delete this if not necessary
        targetTempC = beginCurvesTempC;

        delay(500); // debounce the switch
        cPstate = STABLILZE_DELAY;
      }

      /******************** STABLILZE_DELAY ********************/
    } else if (cPstate == STABLILZE_DELAY) {  // this is the first state
      // initialize some things and wait for speed to stabilize

      //      Serial.print("targetTempC wait f start" ); Serial.println(targetTempC);
      //      Serial.println();

      resetLEDs();
      digitalWrite(LED_WHITE_PIN , HIGH);

      // delay to allow things to stabilize
      if (millis() - stabilizeMillis > STABILIZE_DELAY_MS) {
        cPstate = WAIT_4_TEMP;
      }

      if (resetSwitch) {
        resetSwitch = 0;   // reset this manually every time - the switch read function only sets this variable
        coolDownMillis = millis();
        cPstate = DONE;
        delay(resetDelay); // to debounce switch and make sure it doesn't "shoot through"
      }

      /******************** WAIT_4_TEMP ********************/
    } else if (cPstate == WAIT_4_TEMP) {  // wait for the temp to get up to spec with PID loop
      // delay 15 secondes to allow things to stabilize

      resetLEDs();
      digitalWrite(LED_RED_PIN , HIGH);

      if (targetTempReached) {
        captureMillis = millis();
        totalTach = 0;
        totalPitotADC = 0;
        totalSensorADC = 0;
        dataPoints = 0;
        cPstate = CAPTURE_DATA;
      }

      if (resetSwitch) {
        resetSwitch = 0;
        coolDownMillis = millis();
        cPstate = DONE;
        delay(resetDelay);
      }

      /***************************** CAPTURE_DATA ************************************/
    } if (cPstate == CAPTURE_DATA) {  // this is first just to shorten else if testing

      if (!targetTempReached) {
        cPstate = WAIT_4_TEMP;
      }

      if (millis() - captureMillis > DATA_GATHERING_PERIOD_MS) { // Gathering is complete - on to the next measurement
        cPstate = INCREMENT;
      } else {  // gather data here
        totalPitotADC += analogRead(pitotPin);
        totalSensorADC += analogRead(revPoutPin);
        totalTach += tach;
        dataPoints ++;
      }

      resetLEDs();
      digitalWrite(LED_GREEN_PIN , HIGH);

      if (resetSwitch) {
        resetSwitch = 0;
        resetDelay = millis();
        coolDownMillis = millis();
        cPstate = DONE;
      }


      /******************** INCREMENT ********************/
    }  else if (cPstate == INCREMENT) {  // increment the fanPWM (speed) and start from state STABLILZE_DELAY

      resetLEDs();
      digitalWrite(LED_BLUE_PIN , HIGH);
      digitalWrite(LED_RED_PIN , HIGH);
      delay(200);

      float pitot_FP = (float)totalPitotADC / (float)dataPoints;
      float tachFloat =  (float)totalTach / (float)dataPoints;
      float pitotVolts = (pitot_FP * ADC_REFERNCEVOLTAGE) / 1024.0;
      float sensorADC = (float)totalSensorADC / (float)dataPoints;
      float sensorVolts = (sensorADC * ADC_REFERNCEVOLTAGE) / 1024.0;
      float tachPitotRatio = (pitotVolts * 1000 / tachFloat) ;
      totalPitotADC = 0;
      totalSensorADC = 0;
      totalTach = 0;
      dataPoints = 0;

#ifdef sendDataToProcessing

      printDebugMillis = millis();
      Serial.print("fP,"); Serial.print(fanPWM);  Serial.print(",tch,"); Serial.print(tachFloat);
      Serial.print(",TR,"); Serial.print(tachPitotRatio, 4);
      Serial.print(",bR,");  Serial.print(barometer); Serial.print(",pV,");
      Serial.print(pitotVolts, 3); Serial.print(",sV,"); Serial.println(sensorVolts, 3);

#endif
      /* Increment PWM (Fan Speed) and Temperature
        Check for PWM endpoint - move to increment temp OR increment PWM
        Check for Temperature endpoint  - move to done OR increment temperature and reset PWM to beginPoint
      */

      if (fanPWM != endFanPWM) {  /* Increment fand speed (PWM) */
        if ( endFanPWM < endFanPWM) {
          fanPWM += PWM_INCREMENT_AMOUNT;
        } else {
          fanPWM -= PWM_INCREMENT_AMOUNT;
        }
      } else {    /* Increment temp */
        if (currentTempC == endCurvesTempC) {    // check for done
          fanPWM = 100;
          doFan(fanPWM);
          coolDownMillis = millis();
          cPstate = DONE;
        } else {  // increment
          if ( beginCurvesTempC < endCurvesTempC) {  // calculate and report data
            currentTempC += TEMP_INCREMENT_AMOUNT;
          } else {
            currentTempC -= TEMP_INCREMENT_AMOUNT;
          }

          targetTempC = (float)currentTempC;  // updates the heater temperature
        }
      }
    }

    /******************** DONE ********************/
  } else if (cPstate == DONE) {   // this is kind of an abort setting to reset in middle of a cycle

    resetLEDs();
    digitalWrite(LED_BLUE_PIN , HIGH);
    digitalWrite(LED_GREEN_PIN , HIGH);
    targetTempC = 0;
    doFan(100);

    if (millis() - coolDownMillis > COOL_DOWN_MS) {

      cPstate = WAIT_4_START;
      doFan(0);
      if (resetSwitch || startSwitch) {  // user can bail out at any time
        resetSwitch = 0;
        startSwitch = 0;
        fanPWM = 0;
        cPstate = WAIT_4_START;
      }
    }

    if (!digitalRead(resetSwitchPin)) {  // user can bail out at any time
      fanPWM = 0;
      cPstate = WAIT_4_START;
    }

  }

#ifdef curvePdebug
  if (millis() - printDebugMillis > 1000) {
    printDebugMillis = millis();

    // Serial.print(" cPstate "); Serial.print(cPstate); Serial.print("targetTempC"); Serial.println(targetTempC);
    unsigned long elapsed = (millis() - captureMillis);
    Serial.print("elapsed ");  Serial.println(elapsed);   Serial.print("  "); Serial.println(DATA_GATHERING_PERIOD_MS);
    //Serial.print("mode "); Serial.print(mode); Serial.print(" cPstate "); Serial.println(cPstate);
  }
#endif

}
}
