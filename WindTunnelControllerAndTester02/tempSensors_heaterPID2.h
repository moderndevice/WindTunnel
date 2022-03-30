/* Code for heater PID control and temp sensors
    There are two temp sensors, one after the test chamber and one after the heater.
    Two PID loops control the heaters based on the difference between the two sensors
    The test chamber PID loop is slower than the heater PID to give
    changes in air temp a chance to make their way through the tunnel loop - maybe 16 feet
*/

//Define Variables we'll be connecting to

double targetTempC = 18.0, testChTempC, heaterTempC, OutputPWM, deltaTempC, outputOffsetC, setPoint = 0;
double innerLoopSetpoint;
const float lowPassFilterVal = 0.8; // 0.0 - 1.0  - 1.0 is more filtering
int HeaterI2Caddress =  0x1E;
int WindTunnelI2Caddress = 0x2A;

LibTempTMP421 tempH = LibTempTMP421(0, HeaterI2Caddress);       // sensor just after heater
LibTempTMP421 tempWT = LibTempTMP421(0, WindTunnelI2Caddress);  // temp sensor after test chamber

#define P_ON_M 0
#define P_ON_E 1
bool pOnE = true;
double initInput;
const int heaterPin = 45;
int heatOn = 0, heatFlag = 0;

//Specify the links and initial tuning parameters
// these params work well at low air 75 pwm @ 32 C
// output should never go to zero!
//double Kp = 500, Ki = 12.0, Kd = 0;
//double H_Kp = 30, H_Ki = 0.03, H_Kd = 0.05;  //proportional on error settings

//double H_Kp = 45, H_Ki = 0.9, H_Kd = 3.0; // old tuning
//double H_Kp = 30, H_Ki = 0.4, H_Kd = 1.0;

//PID heaterPID(&heaterTempC, &OutputPWM, &innerLoopSetpoint, H_Kp, H_Ki, H_Kd, P_ON_E, DIRECT);

double TC_Kp = 120.0, TC_Ki = 2.0, TC_Kd = 0.3;  // outer, slower loop - detemines delta
PID testChPID(&testChTempC, &OutputPWM, &setPoint, TC_Kp, TC_Ki, TC_Kd, P_ON_E, DIRECT);

int WindowSize = 1000;
unsigned long windowStartTime;
double tempTestChTempC;

void initHeaterPID2() {

  pinMode(20, INPUT_PULLUP);
  pinMode(21, INPUT_PULLUP);

  windowStartTime = millis();
  //tell the PID to range between 0 and the full window size
  /* Inner loop PID setup */

  //  heaterPID.SetOutputLimits(0, WindowSize);
  //  heaterPID.SetSampleTime(333); // ms sample rate computes at this rate
  //  heaterPID.SetMode(AUTOMATIC); //turn the PID on

  /* loop PID setup */
  testChPID.SetOutputLimits(0, WindowSize);
  testChPID.SetSampleTime(2000); // ms sample rate computes at this rate //used to be 2000
  testChPID.SetMode(AUTOMATIC); // turn the PID on

  windowStartTime = millis();

  tempH.Init();
  tempWT.Init();
  pinMode(heaterPin, OUTPUT);
  Serial.println("initHeater ");
}

void doHeaterLoop2() { // heater control - controls solid state relay
  if (millis() - windowStartTime > WindowSize)
  { //time to shift the Relay Window
    windowStartTime += WindowSize;
  }
  if ((unsigned long)OutputPWM > (millis() - windowStartTime)) {
    digitalWrite(heaterPin, HIGH);
    //  Serial.print(1);            Serial.print("\t");
    heatFlag = 1;
  }
  else {
    digitalWrite(heaterPin, LOW);
    // Serial.print(0); Serial.print("\t");
    heatFlag = 0;
  }

  testChPID.Compute();
}

void doTestChamberLoop2() { // slower loop to add temp offset for test chamber
  static unsigned long lastTime, lastPrintTime;

  if (millis() - lastTime > 500) {
    lastTime = millis();
    //  if (targetTempC + 4 > testChTempC ) { // what is this for????
    // PID testChPID(&deltaTempC, &outputOffsetC, &setPoint
    //  setPoint = 0;

    deltaTempC = testChTempC - targetTempC; // should be negative most times
    // innerLoopSetpoint = outputOffsetC + targetTempC;
    // }
    // else innerLoopSetpoint = 0;
  }



    if (millis() - lastPrintTime > 300) {
      lastPrintTime = millis();
      Serial.print("target "); Serial.print(targetTempC);
      Serial.print("\t TCHT ");     Serial.print((float)testChTempC, 2);
      Serial.print("\t HT ");     Serial.print((float)heaterTempC, 2);
      Serial.print("\t delta ");     Serial.print((float)deltaTempC, 2);
      Serial.print("\t output ");   Serial.println(OutputPWM, 2);
    }
}


void readTempC2() {
  /* read temp sensors and lowpass filter */
  static unsigned long lastTime;
  double tempHeaterTempC;



  if (millis() - lastTime >  200) { // this timing will affect smoothing filter value
    //                               // may require lower I term values
    lastTime = millis();
    /* turn these variables back on to work! */
    tempHeaterTempC = tempH.GetTemperature();
    tempTestChTempC = tempWT.GetTemperature();
    targetTempC = map(pots[4], 0, 1023, 15, 48); // temp times 2 in map
    setPoint = (double)targetTempC;

    heaterTempC = smooth(tempHeaterTempC, lowPassFilterVal, heaterTempC);
    testChTempC = smooth(tempTestChTempC, lowPassFilterVal, testChTempC);

    //    Serial.print(tempTestChTempC);
    //    Serial.print("\t heater ");
    //    Serial.println(tempHeaterTempC);

  }
}
