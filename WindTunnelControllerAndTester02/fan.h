/* Fan Control functions and PID for temperature control */

#define fanTachPin 2  // yellow wire
#define fanPWMpin  5  // orange wire (blue on fan) wire timer 3 (pin 2, 3, 5)
#define fanPotNumber 0
#define PWMCALIBRATION 2  // adjust PWM to make up for new wind tunnel setup
#define ISRSMOOTHSAMPLES 32

volatile unsigned long fanTach, ISRperiod, lastInterrTime; // for ISR
volatile int newISRdata;                                   // for ISR
int  fanPWM, smoothArrayCounter = 0;
unsigned long periodSmoothArray[ISRSMOOTHSAMPLES], arrayTotal;
unsigned int tach;
/********** TODO implement this PID scheme and see if you can stabilize fan *********/
double fanDoubleTach, fanDoublePWM, fanDoubleSetpoint = 272.0;
double Fan_Kp = 110, Fan_Ki = 3.0, Fan_Kd = 1.0;

//PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
// PID fanPID(&fanDoubleTach, &fanDoublePWM, &innerLoopSetpoint, H_Kp, H_Ki, H_Kd,  DIRECT);

void fanISR() {  // this reads the fan tach
  /* there is a glitch in this routine that reports 12 uSta every so often ?!?
    try hardware debounce - schmidt trigger?*/
  volatile unsigned long tempTime = micros();
  ISRperiod = tempTime - lastInterrTime;
  lastInterrTime = tempTime;
  newISRdata = 1;
}

void initFan() {
  pinMode(fanTachPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(fanTachPin), fanISR, CHANGE);
  TCCR3B = (TCCR3B & 0xF8) | 0x01 ;  // pwm pin must be 9 or 10! increase PWM freq
  Serial.println(" intFan ");

  //  fanPID.SetOutputLimits(130, 190);
  //  fanPID.SetSampleTime(300); // ms sample rate computes at this rate //used to be 2000
  //  fanPID.SetMode(AUTOMATIC); // turn the PID on
}


void doFan(int fanPWM) {
  static unsigned long lastTimePWM, lastTimeTach;
  static int localFanPWM;

  if (millis() - lastTimePWM > 20) { //time to PWM speed
    lastTimePWM = millis();
    analogWrite(fanPWMpin, fanPWM + PWMCALIBRATION); // note calibration fudge

    // Serial.println(ISRperiod);

    if ( newISRdata && (ISRperiod > 20) ) {  // debounces some short period tach glitches on fan
      arrayTotal = arrayTotal - periodSmoothArray[smoothArrayCounter];
      periodSmoothArray[smoothArrayCounter] = ISRperiod;
      arrayTotal = arrayTotal + ISRperiod;
      smoothArrayCounter = (smoothArrayCounter + 1) % ISRSMOOTHSAMPLES;
      int smoothTach = arrayTotal / ISRSMOOTHSAMPLES;
      tach = (1000000 / smoothTach );

      // Serial.print(ISRperiod); Serial.print("\t smooth"); Serial.print(smoothTach); Serial.print("\t tach"); Serial.println(tach);
      newISRdata = 0;
    }


    // Serial.print("\t tempTach " ),  Serial.print(tempTach) ; Serial.print("\t tach " ),  Serial.println(tach) ;
  }
}
