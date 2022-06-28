/*  Code functions that read analog values from the windP and wind C test jigs (sensors)
    and convert them to voltages and temperature for output to the test LCD
    Data from the Wind C and Wind P test jigs are also sent out serial port
*/
#define ADC_REFERNCEVOLTAGE 4.99 // meausure or use AREF pin?
int curvesInit = 0; // stores initFlag for function that runs curves
extern int pots[8];

struct Tester {
  float ADCcountAverage;
  float ADCvolts;
  int deltaCount;
  float tempVolts;
  float tempC;
  float revCoutputVolts;
  float pitot;
};

Tester tester;


/* pin defs for ADC inputs */
const int revPtempPin = A1;
const int revPoutPin = A2;
const int revShutdownPin = A0;

const int revCtempPin = A5;
const int revCrawPin = A6; //  use this one
const int revCoutPin = A7; //  don't use this one

const int pitotPin = A15;


void testSensors() {


  /* tests Rev C and P sensors - wiring is different for test jigs but
     software is mostly the same.
  */
  unsigned int bufADC = 0, bufPitot = 0;
  static int counter, maxv, minv, maxPitot, minPitot, initialized = 0, windPin, tempPin;
  static unsigned long totalWindADC, totalPitot, lastADCTime, testTime, printTime;

  if (millis() - lastADCTime < 12) {  // space out the reads a bit
    return;
  }


  lastADCTime = millis();

  if (!initialized) {

    CPflag = REVPFLAG; // temporary delete after getting this working
    /* assign pins for proper board */
    if (CPflag == REVCFLAG) {  // asssign pins for correct sensor
      windPin = revCrawPin;
      tempPin = revCtempPin;
    }
    else { // rev P
      windPin = revPoutPin;
      tempPin = revPtempPin;
    }

    /* reset all the variables
      these are all static variables to preserve data between funtion calls
    */
    counter = 0;
    totalWindADC = 0;
    maxv = 0;
    minv = 60000;
    maxPitot = 0;
    minPitot = 60000;
    initialized = 1;
    testTime = millis();
  }


  bufADC = analogRead(windPin);
  bufPitot = analogRead(pitotPin);
  if (bufADC > maxv ) maxv = bufADC;
  if (bufADC < minv ) minv = bufADC;

  if (bufPitot > maxPitot ) maxPitot = bufPitot;
  if (bufPitot < minPitot ) minPitot = bufPitot;

  totalWindADC += bufADC; // accumulate total for some smoothing

 

  //  Serial.print("read "); Serial.print(millis());
  //  Serial.print("  ");  Serial.print(totalWindADC);  Serial.print(" "); Serial.println(counter);
  //Serial.println(counter);
 counter++;
  if (counter > 9) {

    tester.ADCcountAverage = (float)totalWindADC / 10.0;
    tester.deltaCount = maxv - minv;
    tester.ADCvolts =  ((float) tester.ADCcountAverage * ADC_REFERNCEVOLTAGE) / 1024.0;  // voltage measured empirically on tester
    tester.pitot = totalPitot / 10.0;

    tester.tempVolts = (analogRead(tempPin) * ADC_REFERNCEVOLTAGE) / 1024.0;  // convert to volts
    // VOUT = TC  TA + V0°C  MCP9701/9701A datasheet
    // VOUT - V0C / TC = TA


    if (CPflag == REVPFLAG) {  // handle the different temp sensors on the various sensors
      tester.tempC = (tester.tempVolts - 0.400) / 0.0195;
    }
    else { // rev C
      tester.tempC = tester.tempVolts; // need to convert this to temperature
      /* read the "out" output to test the pot and circuit*/
      tester.revCoutputVolts = (analogRead(revCoutPin) * ADC_REFERNCEVOLTAGE) / 1024.0;
    }

#ifdef SensorTestPrint
if (millis() - printTime > 500){
  printTime = millis();
    Serial.print("exit "); Serial.println(tester.ADCcountAverage, 3); Serial.print(" totalWindADC "); Serial.println(totalWindADC);
    Serial.print("output"); Serial.print(tester.ADCvolts, 3); Serial.print(" V \t "); Serial.print(tester.tempC ); Serial.println("  C");

    Serial.print(millis() - testTime); Serial.println("ms");
}
#endif

    initialized = 0;
    testTime = millis();
  }
}


void readWindC() {
  static unsigned long lastTime;

#ifdef SensorTestPrint
  if (millis() - lastTime > 2000) {
    lastTime = millis();
    tester.ADCvolts = ((float)analogRead(revCrawPin) * ADC_REFERNCEVOLTAGE) / 1024.0;
    tester.tempVolts = (analogRead(revCtempPin) * ADC_REFERNCEVOLTAGE) / 1024.0;

    Serial.print("wind volts "); Serial.print(tester.ADCvolts, 3); Serial.print("\t");
    Serial.print("windC temp "); Serial.print(tester.tempVolts, 3); Serial.println();
  }
#endif

}




void runCcurves() {
  static float startTemp, endTemp, startPWM, endPWM;

  if (!curvesInit) {
    startTemp = analogRead(pots[2]); endTemp = analogRead(pots[3]);
    startPWM = analogRead(pots[6]);  endPWM = analogRead(pots[7]);
  }




}

void runPcurves() {

}
