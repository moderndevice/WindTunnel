/* Code for heater PID control and temp sensors
    There are two temp sensors, one after the test chamber and one after the heater.
    Two PID loops control the heaters based on the difference between the two sensors
    The test chamber PID loop is slower than the heater PID to give
    changes in air temp a chance to make their way through the tunnel loop - maybe 16 feet
*/

//Define Variables we'll be connecting to

double targetTempC, testChTempC, heaterTempC, OutputPWM, deltaTempC, outputOffsetC, setPoint = 0;
double innerLoopSetpoint;
const float lowPassFilterVal = 0.5; // 0.0 - 1.0  - 1.0 is more filtering
int HeaterI2Caddress =  0x1E;
int WindTunnelI2Caddress = 0x2A;
bool targetTempReached = false;

/* wind tunnel now has MCP9808 temp sensors */

Adafruit_MCP9808 WT_temp1 = Adafruit_MCP9808();
Adafruit_MCP9808 WT_temp2 = Adafruit_MCP9808();
Adafruit_MCP9808 HT_temp =  Adafruit_MCP9808();

//LibTempTMP421 tempH = LibTempTMP421(0, HeaterI2Caddress);       // sensor just after heater
//LibTempTMP421 tempWT = LibTempTMP421(0, WindTunnelI2Caddress);  // temp sensor after test chamber

#define P_ON_M 0
#define P_ON_E 1
bool pOnE = true;  // this changes the entire calculation of PID
double initInput;
const int heaterPin = 45;
int heatOn = 0, heatFlag = 0;

//Specify the links and initial tuning parameters
// these params work well at low air 75 pwm @ 32 C
// output should never go to zero!
//double Kp = 500, Ki = 12.0, Kd = 0;
//double H_Kp = 30, H_Ki = 0.03, H_Kd = 0.05;  //proportional on error settings

/********************* inner loop which controls the heater ******************************/
//double H_Kp = 45, H_Ki = 0.9, H_Kd = 3.0; // old tuning
double H_Kp = 40, H_Ki = 0.1, H_Kd = 0.005;   // the loop that actually controls the heater
PID heaterPID(&heaterTempC, &OutputPWM, &innerLoopSetpoint, H_Kp, H_Ki, H_Kd,  DIRECT);

/******************** outer, slower loop which controls the differential between the two sensors ************************/
double TC_Kp = 6.0, TC_Ki = 0.04, TC_Kd = 2.0;  // outer, slower loop - detemines delta between heater and test chamber output
PID testChPID(&deltaTempC, &outputOffsetC, &setPoint, TC_Kp, TC_Ki, TC_Kd, DIRECT);
int WindowSize = 1000;
unsigned long windowStartTime;
double tempTestChTempC;

void initHeaterPID() {

  pinMode(20, INPUT_PULLUP);
  pinMode(21, INPUT_PULLUP);

  windowStartTime = millis();
  //tell the PID to range between 0 and the full window size
  /* Inner loop PID setup */
  heaterPID.SetOutputLimits(0, WindowSize);
  heaterPID.SetSampleTime(333); // ms sample rate computes at this rate
  heaterPID.SetMode(AUTOMATIC); //turn the PID on
  /* outer loop PID setup */
  testChPID.SetOutputLimits(-2, 20);
  testChPID.SetSampleTime(2000); // ms sample rate computes at this rate //used to be 2000
  testChPID.SetMode(AUTOMATIC); // turn the PID on

  windowStartTime = millis();
  pinMode(heaterPin, OUTPUT);
  Serial.println("initHeater ");


  /******* init temp sensors ********/
  if (!WT_temp1.begin(0x18)) {
    Serial.println("Couldn't find MCP9808 WT_temp1!");
  }
  else {
    Serial.println("MCP9808 sens1 init");
  }
  WT_temp1.setResolution(3); // sets the resolution mode of reading, the modes are defined in the table bellow:

  if (!WT_temp2.begin(0x19)) {
    Serial.println("Couldn't find MCP9808 WT_temp2!");
  }
  else {
    Serial.println("MCP9808 sens1 init");
  }

  WT_temp2.setResolution(3); // sets the resolution mode of reading, the modes are defined in the table bellow:

  if (!HT_temp.begin(0x1A)) {
    Serial.println("Couldn't find MCP9808 heaterSens!");
  }
  else {
    Serial.println("MCP9808 heaterSens init");
  }
  HT_temp.setResolution(3); // sets the resolution mode of reading, the modes are defined in the table bellow:

}

void doHeaterLoop() { // heater control - controls solid state relay
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

  heaterPID.Compute();

}

void doTestChamberLoop() { // slower loop to add temp offset for test chamber
  static unsigned long lastTime, lastPrintTime;

  if (millis() - lastTime > 1000) {
    lastTime = millis();
    //  if (targetTempC + 4 > testChTempC ) { // what is this for????
    // PID testChPID(&deltaTempC, &outputOffsetC, &setPoint
    //  setPoint = 0;

    deltaTempC = testChTempC - targetTempC; // should be negative most times
    innerLoopSetpoint = outputOffsetC + targetTempC;
    // }
    // else innerLoopSetpoint = 0;
  }
  testChPID.Compute();
  if  (abs(targetTempC - testChTempC) < 0.6) {
    targetTempReached = true;
  } else {
    targetTempReached = false;
  }

#ifdef PIDprint

  if (millis() - lastPrintTime > 300) {
    lastPrintTime = millis();
    Serial.print("set ");
    Serial.print(targetTempC); Serial.print("\t");  Serial.print(tempTestChTempC);
    Serial.print(" TCHT ");     Serial.print((float)testChTempC, 2);  Serial.print("\t");
    Serial.print("HT ");     Serial.print((float)heaterTempC, 2);  Serial.print("\t");
    Serial.print("delta "); Serial.print(deltaTempC);  Serial.print("\t");
    Serial.print("output ");   Serial.print(outputOffsetC);   Serial.print("\t");
    Serial.print("innerSetpt "); Serial.print(innerLoopSetpoint);   Serial.print("\t");
    Serial.println(OutputPWM, 2);
  }

#endif

}


void readTempC() {
  /* read temp sensors and lowpass filter */
  static unsigned long lastTime;
  double tempHeaterTempC;

  if (millis() - lastTime >  250) { // this timing will affect smoothing filter value
    //                               // may require lower I term values
    lastTime = millis();
    /* turn these variables back on to work! */

    // temp sensor objects WT_temp1, WT_temp2, HT_temp

    tempHeaterTempC = HT_temp.readTempC();
    tempTestChTempC = (WT_temp1.readTempC() + WT_temp2.readTempC()) / 2.0;
//    targetTempC = map(pots[4], 0, 1023, 15, 48); // temp times 2 in map  - this needs to go someplace else
// to accomodate automatic control

    heaterTempC = smooth(tempHeaterTempC, lowPassFilterVal, heaterTempC);
    testChTempC = smooth(tempTestChTempC, lowPassFilterVal, testChTempC);

#ifdef TempSensorTestprint

    Serial.print("testChamber temp ");
    Serial.print(tempTestChTempC);
    Serial.print("\t heater temp ");
    Serial.println(tempHeaterTempC);

#endif

  }
}
